#include <signal.h>
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
#include "queueADT.h"

#define USER 10

void signal_handler(int signo);
void running_fifo();
void do_child(int signo);

typedef struct pcb_object{
	pid_t pid;
	int exec_time;
}p_ob;

typedef struct user_object{
	int cpu_burst;
	int pid;
}u_ob;

u_ob u_info[USER];
p_ob user[USER];

Queue running;

int count = 0;
int order = 0;
int next=1;
int set[USER];
int waiting_time[USER];

int main()
{
	InitQueue(&running);



	set[0]=1;
	set[1]=5;
	set[2]=1;
	set[3]=7;
	set[4]=6;
	set[5]=5;
	set[6]=7;
	set[7]=2;
	set[8]=3;
	set[9]=10;


	srand(time(NULL));

	struct sigaction old_sa;
	struct sigaction new_sa;
	memset(&new_sa, 0, sizeof(new_sa));
	new_sa.sa_handler = &signal_handler;
	sigaction(SIGALRM, &new_sa, &old_sa);

	//	srand((unsigned)time(NULL));

	for(int a=0;a<USER;a++){
		pid_t ppid;
		ppid = fork();

		if (ppid == -1) {
			perror("fork error");
			exit(0);
		}
		else if(ppid==0){ 
			u_info[a].cpu_burst=set[a];
			u_info[a].pid = getpid();
			printf("user : set %d (main %d)\n", u_info[a].cpu_burst,set[a]);
			printf("maken child proc %d \n", getpid());

			new_sa.sa_handler = &do_child;
			sigaction(SIGUSR1, &new_sa, &old_sa);
			while(1);
		}
		else{ 
			user[a].pid = ppid;
			user[a].exec_time = set[a];

			Enqueue(&running, a);
		}
	}

	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = 0;
	new_itimer.it_interval.tv_usec = 1;
	new_itimer.it_value.tv_sec = 0;
	new_itimer.it_value.tv_usec = 1;
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while (1);
	return 0;
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
	printf("proc[%d] remain cpu : %d \n",set_order,u_info[set_order].cpu_burst);

	if(u_info[set_order].cpu_burst==0){
		exit(0);
	}
}

void signal_handler(int signo){
	count++;
	running_fifo();
	return ;
}


void running_fifo(){
	if(next==1){

		order = Dequeue(&running);
		if(order==-1){
			float total=0;
			printf("\n--------waiting time---------\n");
			for(int a=0;a<USER;a++){
				printf("prod[%d] %d \n",a,waiting_time[a]);
				total = waiting_time[a] + total;

			}
			printf("\n");

			printf("\naverage waiting time : %f \n",total/10);

			exit(0);
		}
		next=0;
	}
	printf("\n---------tick %d--------\n",count);
	int num_running = counting(&running);
	int run_arr[num_running];

	printf("ready queue[%d] : ",num_running);
	sorting(&running, run_arr);
	for(int k=0; k<num_running; k++){
		printf("%d ",run_arr[k]);
	}
	printf("\n");


	printf("running queue[1] : %d\n", order);

	user[order].exec_time--;

	kill(user[order].pid,SIGUSR1);	

	if(user[order].exec_time==0){
		next=1;
		waiting_time[order] = count-set[order];
		return ;
	}

	return ;
}
