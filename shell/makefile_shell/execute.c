#include "header.h"

int execute(char* argv[]){
	pid_t pid;
	pid=fork();

	if (pid == -1) {
		perror("fork error");
		exit(0);
	}
	else if(pid==0){
		int ex_err = execve(findpath, argv ,NULL);
		free(findpath);
		if(ex_err==-1){
			printf("execve err\n");
		}
		exit(0);
	}
	else{
		wait(0);
	}
	return 0;
}

