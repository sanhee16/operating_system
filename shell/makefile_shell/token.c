#include "header.h"

int token(char* command){
	char *next, *token_str;
	str_num=0;

	while(1){
		str[str_num] = strtok_r(command, " ", &next);
		if(str[0]==NULL){
			return 1;
		}
		if(str[str_num]==NULL){
			break;
		}
		command=NULL;
		str_num++;
	}
	return 0;
}

