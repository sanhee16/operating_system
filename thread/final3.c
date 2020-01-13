#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 1000

int letter[256];
int total[256];
int endfile=0;
void char_stat(char* line);

typedef struct bufobject{
	char* data;
	int linenum;
	int full;
	int check;
	pthread_mutex_t lock;
	pthread_cond_t cond;
}buf;

typedef struct sharedobject {
	FILE *rfile;
	char *line;
	pthread_mutex_t lock_cons;
	pthread_cond_t cond_cons;
	pthread_mutex_t lock_stat;
	pthread_cond_t cond_stat;

	buf buffer[BUFSIZE];
} so_t;

int Ncons,Nprod;
int ptr_prod=0;
int test_full=0;

void *producer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int next_ptr=0;
	while (1){
		pthread_mutex_lock(&so->buffer[next_ptr].lock);
		ptr_prod = next_ptr;
		next_ptr++;
		if(next_ptr==BUFSIZE)
			next_ptr=0;
		if(so->buffer[ptr_prod].full == 1){ // unlock here
			pthread_mutex_unlock(&so->buffer[ptr_prod].lock);
			continue;
		}
		read = getdelim(&line, &len, '\n', rfile);
		so->buffer[ptr_prod].linenum = i;
		if (read == -1) { // full -> put in my data into the buffer , unlock
			so->buffer[ptr_prod].full = 1;
			so->buffer[ptr_prod].data = NULL;
			endfile=1;
			pthread_mutex_unlock(&so->buffer[ptr_prod].lock);
			break;
		}
		so->buffer[ptr_prod].data=(char*)malloc(sizeof(char)*100);
		so->buffer[ptr_prod].data = strdup(line);
		i++;
		so->buffer[ptr_prod].full = 1;
		pthread_mutex_unlock(&so->buffer[ptr_prod].lock);
	}
	free(line);
	*ret = i;
	pthread_exit(ret);
}

int next_ptr=0;
int secondchance=0;
int firstchance=0;

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	int len;
	char *line;
	int ptr_cons=0;
	int exist = 0;

	while(1){
		pthread_mutex_lock(&so->lock_cons);
		ptr_cons=next_ptr;
		next_ptr++;
		if(next_ptr==BUFSIZE){
			next_ptr=0;
		}

		pthread_mutex_lock(&so->buffer[next_ptr].lock);
		pthread_mutex_unlock(&so->lock_cons);

		if(so->buffer[ptr_cons].check==3){
			pthread_mutex_unlock(&so->buffer[ptr_cons].lock);
			break;
		}

		if(endfile){
			so->buffer[ptr_cons].check++;
		}

		if(so->buffer[ptr_cons].full == 0){
			pthread_mutex_unlock(&so->buffer[ptr_cons].lock);
			continue;
		}

		line = so->buffer[ptr_cons].data;
		if(line==NULL){
			pthread_mutex_unlock(&so->buffer[ptr_cons].lock);
			continue;
		}
		exist=1;
		i++;
		so->buffer[ptr_cons].data = NULL;
		so->buffer[ptr_cons].full = 0;

		pthread_mutex_unlock(&so->buffer[ptr_cons].lock);

		if(exist){
			exist=0;
			pthread_mutex_lock(&so->lock_stat);
			char_stat(line);
			pthread_mutex_unlock(&so->lock_stat);
		}
	}
	*ret = i;
	pthread_exit(ret);
}


int main (int argc, char *argv[])
{ // consumer
	pthread_t prod[100];
	pthread_t cons[100];
	int rc;
	int *ret;
	int i;
	FILE *rfile;
	if (argc == 1) {
		exit (0);
	}
	so_t *share = malloc(sizeof(so_t));
	memset(share, 0, sizeof(so_t));
	rfile = fopen((char *) argv[1], "r");
	if (rfile == NULL) {
		perror("rfile");
		exit(0);
	}
	if (argv[2] != NULL) {
		Nprod = atoi(argv[2]);
		if (Nprod > 100) Nprod = 100;
		if (Nprod == 0) Nprod = 1;
	}
	//else Nprod = 1;

	if (argv[3] != NULL) {
		Ncons = atoi(argv[3]);
		if (Ncons > 100) Ncons = 100;
		if (Ncons == 0) Ncons = 1;
	}
	//else Ncons = 1;

	share->rfile = rfile;
	share->line = NULL;
	memset(share->buffer, 0, sizeof(buf)*BUFSIZE);
	memset(total,0,sizeof(total));

	pthread_mutex_init(&share->lock_cons, NULL);
	pthread_cond_init(&share->cond_cons, NULL);
	pthread_mutex_init(&share->lock_stat, NULL);
	pthread_cond_init(&share->cond_stat, NULL);

	for(int a=0;a<BUFSIZE;a++){
		pthread_mutex_init(&share->buffer[a].lock, NULL);
		pthread_cond_init(&share->buffer[a].cond, NULL);
	}

	for (i = 0 ; i < Nprod ; i++){
		pthread_create(&prod[i], NULL, producer, share);
	}
	for (i = 0 ; i < Ncons ; i++){
		pthread_create(&cons[i], NULL, consumer, share);
	}
	for (i = 0 ; i < Ncons ; i++) {
		rc = pthread_join(cons[i], (void **) &ret);
		printf("main: consumer_%d joined with %d\n", i, *ret);
	}
	for (i = 0 ; i < Nprod ; i++) {
		rc = pthread_join(prod[i], (void **) &ret);
		printf("main: producer_%d joined with %d\n", i, *ret);
	}


	for(int a=65;a<91;a++){
		for(int b=0;b<BUFSIZE;b++){
			total[a-65] = total[a-65] + letter[a] + letter[a+32];
		}
		printf("letter %c and %c : %d \n",a,a+32,letter[a]+letter[a+32]);
	}

	pthread_exit(NULL);
	exit(0);
}

void char_stat(char* line){

	size_t length = 0;
	//char* sep = "#<>() []{},;!+\\\"\n\t";
	char* sep = "\n";
	char* nextptr;

	char* sepstr = strtok_r(line,sep,&nextptr);


	while(sepstr){
		length=strlen(sepstr);
		for(int a=0;a<length;a++){
			if((*sepstr<256)&&(1<*sepstr)){
				letter[*sepstr]++;
			}
			sepstr++;
		}
		sepstr = strtok_r(NULL,sep,&nextptr);
	}


	return ;
}

