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

#include "queue.h"
#define USER 10
#define TQ 2
#define ADDR_NUM 10

void signal_handler(int signo);
int count = 0;
void running_tq();
void do_child(int signo);
void check_waiting();
void check_running();
void parent_job(int signo);
void check_page();

typedef struct {
	long mtype;
	int order;
	int pid;
	int io_time;
}msgbuf;

typedef struct {
	long mtype;
	int addr[ADDR_NUM];
	int order;
	int pid;
	int diff[ADDR_NUM];
	int w_data[ADDR_NUM/2];
}pmbuf;

typedef struct frame_object{
	int addr;
	int pid;
	int order;
	int data[0x400];
}frame_ob;

typedef struct page_object{
	int addr;
	int pid;
	int order;
}page_ob;

typedef struct pcb_object{
	pid_t pid;
	int io_burst;
	int remain_tq;
	int cpu_burst;
	int* TTBR;
	Queue clock;

}p_ob;

typedef struct user_object{
	int cpu_burst;
	int io_burst;
	int pid;
	int mem_acc[ADDR_NUM];
}u_ob;

typedef struct tlb_object{
	int pid;
	int pa;
	int va;

}tlb_ob;


Queue free_frame;
Queue free_page;
Queue used_page;
Queue free_page_entry;
Queue used_page_entry;


Queue swap_proc;
Queue used_frame;
Queue HDD;


pmbuf msg_pm;
frame_ob* frame;

u_ob u_info[USER];
p_ob user[USER];
msgbuf msg;
Queue running;
Queue waiting;

int order = 0;
int next=1;
int msgq;
int msgq_pm;

int remem_io[USER];
int io_request=0;
int kernel=0;
int cpu_set[USER];
int io_set[USER];

int *PM;
int key = 32161620;
int key_pm = 32161621;

int *frame_ptr;
int *page2_ptr;
int *page1_ptr;
frame_ob *frame_info;
frame_ob *store_HDD;
page_ob* page2_info;

int main(){


	InitQueue(&swap_proc);
	InitQueue(&used_frame);
	InitQueue(&HDD);
	for(int i=0;i<USER;i++){
		InitQueue(&user[i].clock);
	}
	InitQueue(&running);
	InitQueue(&waiting);
	InitQueue(&free_frame);
	InitQueue(&free_page);
	InitQueue(&free_page_entry);
	InitQueue(&used_page);
	InitQueue(&used_page_entry);



	PM = (int *)malloc(sizeof(int)*0x40000000);
	frame_ptr = PM + 0x104000;
	page2_ptr = PM + 0x4000;
	page1_ptr = PM;

	frame_info = (frame_ob*)malloc(sizeof(frame_ob)*0xF0000);
	page2_info = (page_ob*)malloc(sizeof(page_ob)*0x400);

	for(int a=0;a<0xFFBF6;a++){
		Enqueue(&free_frame, a);
	}
	for(int a=0;a<0x400;a++){
		Enqueue(&free_page, a);
	}

	store_HDD = (frame_ob*)malloc(sizeof(frame_ob)*0xF0000);

	//srand(getpid()*a);
	//srand(time(NULL));
	msgq = msgget( key, IPC_CREAT | 0666);
	msgq_pm = msgget( key_pm, IPC_CREAT | 0666);	

	for(int a=0;a<USER;a++){
		srand(getpid()*a);
		io_set[a] = (rand()+2)%20;
		cpu_set[a] = (rand()+2)%20;
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
			user[a].TTBR = page1_ptr + 0x400*a;
			for(int z=0;z<0x400; z++){
				*(user[a].TTBR + z) = 0;
			}
			Enqueue(&running, a);
			int check = counting(&running);
		}
	}
	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = 0;
	new_itimer.it_interval.tv_usec = 15000;
	new_itimer.it_value.tv_sec = 0;
	new_itimer.it_value.tv_usec = 15000;

	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while(1){
		int ret_pm = msgrcv(msgq_pm, &msg_pm, sizeof(pmbuf), 1, IPC_NOWAIT);
		if(ret_pm!=-1){
			check_page();
		}

		int ret = msgrcv(msgq, &msg, sizeof(msgbuf), 1, IPC_NOWAIT);
		if(ret!=-1){
			user[msg.order].io_burst=msg.io_time;
			Enqueue(&waiting, msg.order);
		}
	}
}

int do_count=0;
void do_child(int signo){
	int set_order=-1;
	for(int a=0; a<USER; a++){
		if(u_info[a].pid == getpid()){
			set_order=a;
			break;
		}
	}
	do_count++;
	u_info[set_order].cpu_burst--;
	srand(getpid());
	int loop_count=0;
	for(int z=0;z<ADDR_NUM;z++){
		loop_count++;

		if(loop_count<=ADDR_NUM/2){
			u_info[set_order].mem_acc[z] = (rand()*do_count+2)%0xFFFFFFFF;
			msg_pm.diff[z]=0;
			msg_pm.w_data[z]=loop_count+do_count;
		}
		else{
			u_info[set_order].mem_acc[z] = u_info[set_order].mem_acc[z-ADDR_NUM/2];
			msg_pm.diff[z]=1;
		}
		//	printf("diff %d ",msg_pm.diff[z]);
	}
	msg_pm.mtype=1;
	msg_pm.order=set_order;
	msg_pm.pid=getpid();
	for(int a=0;a<ADDR_NUM;a++){
		msg_pm.addr[a]=u_info[set_order].mem_acc[a];
	}

	int ret2 = msgsnd(msgq_pm, &msg_pm, sizeof(pmbuf), NULL);
	if(ret2 == -1){
		perror("msgsnd error ");
	}
	else if(ret2!=-1){
		//	printf("send msg \n");
	}
	if(u_info[set_order].cpu_burst==0){
		u_info[set_order].cpu_burst=cpu_set[set_order];
		msg.mtype = 1;
		msg.order = set_order;
		msg.pid = getpid();
		msg.io_time = u_info[set_order].io_burst;

		int ret = msgsnd(msgq, &msg, sizeof(msgbuf), NULL);

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
int frame_count=0;
int this_count=0;
void check_page(){
	int page2_num;

	int offset;
	int frame_num;
	int pm_order;

	int pm_pid;
	int page1_entry, page2_entry;
	int page2_index, page1_index;
	int state_bit;
	int diff;
	int p_data;
	int PA[ADDR_NUM];
	int write;

	pm_order = msg_pm.order;
	pm_pid = msg_pm.pid;	
	int addr;
	printf("\n");
	for(int a=0;a<ADDR_NUM;a++){
		addr = msg_pm.addr[a];
		diff = msg_pm.diff[a];
		if(diff==0){
			write = msg_pm.w_data[a];
		}

		page1_index=(addr&0xFFC00000)>>22;
		page2_index=(addr&0x003FF000)>>12;
		offset = addr&0xFFF;

		page1_entry = *(user[pm_order].TTBR + page1_index);
		state_bit=page1_entry&0x1;

		if(state_bit==0){

			int count_page2 = counting(&used_page);
			if(count_page2>20){
				int delete_page = Dequeue(&used_page);
				Enqueue(&free_page, delete_page);
				printf("page2 change %d -> out \n",delete_page);

				for(int index=0;index<0x400;index++){
					int delete_frame=0;
					int check_valid = *(page2_ptr+0x400*delete_page + index)&0x1;
					if(check_valid == 1){
						delete_frame = *(page2_ptr+0x400*delete_page + index)>>12;

						for(int z=0; z<0x400; z++){
							*(frame_ptr+0x400*delete_frame+z)=0;
						}
						*(page2_ptr+0x400*delete_page + index)=0;
					}
				}
				int check_page1_index = (page2_info[delete_page].addr&0xFFC00000) >>22;
				*(user[page2_info[delete_page].order].TTBR+check_page1_index)=0;
			}
			int new_page2 = Dequeue(&free_page); 

			state_bit=1;

			page2_info[new_page2].addr = addr;
			page2_info[new_page2].pid = pm_pid;
			page2_info[new_page2].order = pm_order;

			*(user[pm_order].TTBR + page1_index) =(new_page2<<22) | 0x1;
			page1_entry = *(user[pm_order].TTBR + page1_index);
			page2_num = (page1_entry & 0xFFC00000)>>22;
		}
		if(state_bit==1){
			page2_num = (page1_entry & 0xFFC00000)>>22;
			page2_entry = *(page2_ptr + 0x400*page2_num + page2_index);
			state_bit = page2_entry & 0x1;
			frame_num = page2_entry>>12;

			find(&used_page,page2_num);
			Enqueue(&used_page,page2_num);

			if(state_bit==0){
				printf("page fault \n");
				int count_frame = counting(&user[pm_order].clock);
				if(count_frame>100){
					for(int del=0;del<10;del++){
						int delete_frame=0;
						int count_used = 0;
						count_used = counting(&user[pm_order].clock);
						int used[count_used];
						sorting(&user[pm_order].clock, used);

						int check_ref=0;

						for(int loop=0;loop<count_used;loop++){
							if(((used[check_ref]&0x2)>>1)==1){
								find(&user[pm_order].clock,used[check_ref]);
								used[check_ref] = used[check_ref]&0xFFFFF001;
								Enqueue(&user[pm_order].clock,used[check_ref]);	
								check_ref++;
							}
							else{
								int del_page = find(&user[pm_order].clock, used[check_ref]);
								delete_frame = (del_page & 0xFFFFF000)>>12;
								printf("delete frame %d \n",delete_frame);
								break;
							}
							if(check_ref==count_used){
								int del_page = Dequeue(&user[pm_order].clock);
								delete_frame = (del_page & 0xFFFFF000)>>12;
								printf("delete frame %d \n",delete_frame);
								break;
							}
						}

						for(int z=0; z<0x400; z++){
							*(frame_ptr+0x400*frame_num+z)=0;
						}

						int find_addr = frame_info[delete_frame].addr;
						int find_page1_num = (find_addr&0xFFC00000)>>22;
						int find_page2_num = (find_addr&0x003FF000)>>12;
						int find_offset = find_addr&0xFFF;
						int find_page1_entry = *(user[frame_info[delete_frame].order].TTBR + find_page1_num*4);
						int find_page2_index = (find_page1_entry & 0xFFC00000)>>22;
						*(page2_ptr+0x400*find_page2_num + find_page2_index) =0;
						Enqueue(&free_frame, delete_frame);
					}
				}

				int new_frame = Dequeue(&free_frame);
				frame_num = new_frame;
				page2_entry = *(page2_ptr+0x400*page2_num + page2_index);
				for(int z=0;z<0x400;z++){
					*(frame_ptr+0x400*frame_num+z)=0x1000*frame_num + z;
					frame_info[frame_num].data[z]=0x1000*frame_num + z;
				}

				*(page2_ptr+0x400*page2_num + page2_index) = (frame_num<<12) | 0x1;
				state_bit=1;
				page2_entry = *(page2_ptr+0x400*page2_num + page2_index);

				frame_count++;
				frame_info[frame_num].addr = addr;
				frame_info[frame_num].pid = pm_pid;
				frame_info[frame_num].order = pm_order;
			}
			if(state_bit==1){
				page2_entry = *(page2_ptr+0x400*page2_num + page2_index);

				if(page2_entry&0x2 == 0){
					find(&user[pm_order].clock, page2_entry);

					*(page2_ptr+0x400*page2_num + page2_index) = (*(page2_ptr+0x400*page2_num + page2_index))|0x2;
					page2_entry = *(page2_ptr+0x400*page2_num + page2_index);
					Enqueue(&user[pm_order].clock, page2_entry);
				}	
				else{
					find(&user[pm_order].clock, page2_entry);
					Enqueue(&user[pm_order].clock, page2_entry);
				}
				int count_frame = counting(&user[pm_order].clock);

				frame_num = page2_entry >> 12;
				PA[a] = (frame_ptr+0x400*frame_num+offset)-PM;
				printf("VA 0x%08x -> PA 0x%08x : ",addr,PA[a]);
				if(diff==0){
					*(frame_ptr+0x400*frame_num+offset/4) = write;
					printf("write data %d \n",write);

				}
				else if(diff==1){
					p_data = *(frame_ptr+0x400*frame_num+offset/4);
					printf("read  data %d \n",p_data);
				}	
			}
		}
	}
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
			return ;
		}
		next=0;
	}

	int num_running = counting(&running);
	int run_arr[num_running];

	printf("ready queue[%d] : ",num_running);
	sorting(&running, run_arr);
	for(int k=0; k<num_running; k++){
		printf("%d ",run_arr[k]);
	}
	printf("\n");


	printf("running queue[1] : %d\n", order);
	printf("running pid[%d] | remain cpu_burst %d | origin cpu time %d \n",user[order].pid,user[order].cpu_burst,cpu_set[order]);

	user[order].remain_tq--;
	user[order].cpu_burst--;

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

