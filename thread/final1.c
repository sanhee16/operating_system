#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct sharedobject {

	FILE *rfile;
	char *line;
	int linenum;
	int full;

	pthread_mutex_t lock;
	pthread_cond_t cond;

} so_t;

void *producer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;

	while (1) {
		pthread_mutex_lock(&so->lock);
		read = getdelim(&line, &len, '\n', rfile);
		while(so->full == 1){
			pthread_cond_signal(&so->cond);
			pthread_cond_wait(&so->cond, &so->lock);
		}
		so->linenum = i;
		if (read == -1) {
			so->full = 1;
			so->line = NULL;
			pthread_cond_signal(&so->cond);
			pthread_mutex_unlock(&so->lock);
			break;
		}
		so->line = (char*)malloc(sizeof(char)*100);
		so->line = strdup(line);
		//printf("[prod:%d] %s \n",i,so->line);
		i++;
		so->full = 1;
		pthread_cond_signal(&so->cond);
		pthread_mutex_unlock(&so->lock);
	}
	free(line);
	//printf("Prod_%x: %d lines\n", (unsigned int)pthread_self(), i);
	*ret = i;
	pthread_exit(ret);
}

void *consumer(void *arg) {
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	int len;
	char *line;

	while (1) {
		pthread_mutex_lock(&so->lock);
		while(so->full == 0){
			pthread_cond_signal(&so->cond);
			pthread_cond_wait(&so->cond, &so->lock);
		}
		line = so->line;
		if(line == NULL) {
			pthread_cond_signal(&so->cond);
			pthread_mutex_unlock(&so->lock);
			break;
		}
		//printf("Cons_%x: [%02d:%02d] %s",(unsigned int)pthread_self(), i, so->linenum, so->line);
		i++;
		so->line=NULL;
		so->full = 0;

		pthread_cond_signal(&so->cond);
		pthread_mutex_unlock(&so->lock);
	}

	//printf("Cons: %d lines\n", i);
	*ret = i;
	pthread_exit(ret);
}


int main (int argc, char *argv[]){
	pthread_t prod[100];
	pthread_t cons[100];
	int Nprod, Ncons;
	int rc;   long t;
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

	if (argv[3] != NULL) {
		Ncons = atoi(argv[3]);
		if (Ncons > 100) Ncons = 100;
		if (Ncons == 0) Ncons = 1;
	}

	share->rfile = rfile;
	share->line = NULL;

	pthread_mutex_init(&share->lock, NULL);
	pthread_cond_init(&share->cond, NULL);


	for (i = 0 ; i < Nprod ; i++)
		pthread_create(&prod[i], NULL, producer, share);
	for (i = 0 ; i < Ncons ; i++)
		pthread_create(&cons[i], NULL, consumer, share);

	for (i = 0 ; i < Ncons ; i++) {
		rc = pthread_join(cons[i], (void **) &ret);
		printf("main: consumer_%d joined with %d\n", i, *ret);
	}
	for (i = 0 ; i < Nprod ; i++) {
		rc = pthread_join(prod[i], (void **) &ret);
		printf("main: producer_%d joined with %d\n", i, *ret);
	}

	pthread_exit(NULL);
	exit(0);

}

