#include "header.h"


int getenv_var(char newenv[10]){
	char *env;
	env=(char*)malloc(sizeof(char)*200);
	//char newenv[10];

	if(strcmp(str[0]+1,"TIME")==0){
		time_t  today_time;
		time_str* today;
		time(&today_time);
		today = localtime(&today_time);
		printf("[TIME] %d:%d:%d \n", today->tm_hour, today->tm_min,today->tm_sec);
	}
	else{
		if(strcmp(str[0],"echo")==0){
			//strcpy(newenv,str[1]+1);
			strcpy(env,str[1]+1);
		}
		else{
			//strcpy(newenv,str[0]+1);
		}
		env = getenv(newenv);
		printf("%s=%s\n", newenv, env);
	}
	return -1;
}


