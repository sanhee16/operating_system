#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "queueADT.h"
#include "msg.h"
#define USER 10
#define TQ 2

void signal_handler(int signo);
int count = 0;
void running_tq();
void do_child(int signo);
void check_waiting();
void check_running();
void parent_job(int signo);

typedef struct pcb_object{
	pid_t pid;
	int io_burst;
	int remain_tq;
	int cpu_burst;
}p_ob;

typedef struct user_object{
	int cpu_burst;
	int io_burst;
	int pid;
}u_ob;

u_ob u_info[USER];
p_ob user[USER];
msgbuf msg;
Queue running;
Queue waiting;

int order = 0;
int next=1;
int msgq;
int remem_io[USER];
int io_request=0;
int kernel=0;
int cpu_set[USER];
int io_set[USER];

int key = 32161620;

int main()
{
	InitQueue(&running);
	InitQueue(&waiting);


	srand(time(NULL));
	msgq = msgget( key, IPC_CREAT | 0666);
	    printf("msgq id: %d\n", msgq);

	for(int a=0;a<USER;a++){
		io_set[a] = (rand()%20) + 1;
		cpu_set[a] = (rand()%20) + 1;
	}
	struct sigaction old_sa;
	struct sigaction new_sa;
	memset(&new_sa, 0, sizeof(new_sa));
	new_sa.sa_handler = &signal_handler;
	sigaction(SIGALRM, &new_sa, &old_sa);

	for(int a=0;a<USER;a++){
		pid_t ppid;
		ppid = fork();

		if(ppid == -1){
			perror("fork error");
			exit(0);
		}
		else if(ppid==0){ //for child
			InitQueue(&running);
			InitQueue(&waiting);

			u_info[a].cpu_burst = cpu_set[a];
			u_info[a].io_burst = io_set[a];
			u_info[a].pid = getpid();

			printf("proc %d \n",a);
			printf("io : %d, cpu : %d \n",u_info[a].io_burst,u_info[a].cpu_burst);

			new_sa.sa_handler = &do_child;
			sigaction(SIGUSR1, &new_sa, &old_sa);

			while(1);
		}
		else{ 
			kernel=getpid();
			user[a].pid = ppid;
			user[a].io_burst = 0;
			user[a].cpu_burst = cpu_set[a];
			user[a].remain_tq = TQ;

			Enqueue(&running, a);
			int check = counting(&running);
		}
	}
	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = 0;
	new_itimer.it_interval.tv_usec = 8000;
	new_itimer.it_value.tv_sec = 0;
	new_itimer.it_value.tv_usec = 8000;

	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while(1){
		int ret = msgrcv(msgq, &msg, sizeof(msgbuf), 0, IPC_NOWAIT);
		if(ret==-1){

		}
		else{
			user[msg.order].io_burst=msg.io_time;
			//printf("msgrcv[%d] (%d)\n",msg.order,msg.io_time);
			Enqueue(&waiting, msg.order);
		}
	}	
}

void do_child(int signo){
	int set_order=-1;
	for(int a=0; a<USER; a++){
		if(u_info[a].pid == getpid()){
			set_order=a;
			break;
		}
	}

	u_info[set_order].cpu_burst--;
	//	printf("proc[%d] remainvp : %d\n",set_order, u_info[set_order].cpu_burst);

	if(u_info[set_order].cpu_burst==0){
		//		printf("snd msg \n");
		u_info[set_order].cpu_burst=cpu_set[set_order];
		msg.mtype = 1;
		msg.order = set_order;
		msg.pid = getpid();
		msg.io_time = u_info[set_order].io_burst;

		int ret = msgsnd(msgq, &msg, sizeof(msgbuf), 0);

		if(ret==-1){
			perror("msgsnd error ");
		}
	}
}

void signal_handler(int signo){
	count++;
	running_tq();
	if(count==10000){
		for(int a =0; a<USER ; a++){
			kill(user[a].pid, SIGINT);
		}
		exit(0);
	}
	return ;
}

void running_tq(){
	check_running();

	check_waiting();
	return ;
}

void check_waiting(){
	int num_waiting = counting(&waiting);
	int wait_arr[num_waiting];

	printf("waiting queue[%d] : ",num_waiting);

	int success = sorting(&waiting, wait_arr);
	for(int k=0; k<num_waiting; k++){
		printf("%d ",wait_arr[k]);
	}
	printf("\n");
	if(success == 0){
		printf("waiting  time[%d] : ",num_waiting);
		for(int k=0; k<num_waiting; k++){
			int a = wait_arr[k];
			printf("%d ",user[a].io_burst);

			if(user[a].io_burst==0){
				user[a].io_burst=io_set[a];
				find(&waiting, a);
				Enqueue(&running, a);
			}
			else{
				user[a].io_burst--;
			}
		}
	}
	printf("\n");
	return ;
}

void check_running(){
	printf("\n---------tick %d--------\n",count);

	if(next==1){
		order = Dequeue(&running);
		if(order==-1){
			//printf("empty");
			exit(0);
		}
		next=0;
	}

	int num_running = counting(&running);
	int run_arr[num_running];

	printf("running queue[%d] : ",num_running);
	sorting(&running, run_arr);
	for(int k=0; k<num_running; k++){
		printf("%d ",run_arr[k]);
	}
	printf("\n");


	printf("running queue[1] : %d\n", order);
	printf("running pid[%d] | remain cpu_burst %d | origin cpu time %d \n",user[num_running].pid,user[num_running].cpu_burst,cpu_set[num_running]);

	user[order].remain_tq--;
	user[order].cpu_burst--;

	//printf("proc[%d] remaintq : %d\n", order, user[order].remain_tq);
	kill(user[order].pid,SIGUSR1);
	if(user[order].cpu_burst==0){
		next=1;
		user[order].remain_tq=TQ;
		user[order].cpu_burst = cpu_set[order];
		return ;
	}
	if(user[order].remain_tq > 0){
		next=0;
	}
	else if(user[order].remain_tq==0){
		next=1;
		user[order].remain_tq=TQ;
		Enqueue(&running, order);
	}
	return ;
}
