#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	printf("my pid is %d\n", getpid());

	for(int i=0; i<10; i++){
		pid = fork();
		if (pid == 0) {
			// child
			printf("child process with pid %d(fork %d)\n", getpid(),pid);
			exit(0);
			} 
		else if(pid < 0){
			perror("fork error");
			exit(0);
		}
		else{
			// parent
			//	printf("my pid is %d(fork %d)\n", getpid(),pid);
			wait(0); // syscall
		}
	}
	return 0;
}
