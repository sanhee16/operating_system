#include "header.h"


int stat_file(){
	struct stat fstat_buf;
	int ret, ret1;

	int i=0;
	while(1){
		if(basic[i]==NULL){
			break;
		}
		if(strcmp(str[0],"cd")==0){
			char cwd[100];
			getcwd(cwd,sizeof(cwd));
			chdir(str[1]);
			getcwd(cwd,sizeof(cwd));
			return 2;
		}
		char newpath[20];
		findpath = (char*)malloc(sizeof(int)*100);

		ret1 = stat(str[0], &fstat_buf);
		if(ret1==0){
			strcpy(findpath,str[0]);
			return 0;
		}
		strcpy(newpath,store_path[i]);
		strcat(newpath,"/");
		strcat(newpath,str[0]);
		ret = stat(newpath, &fstat_buf);
		i++;


		if(ret==0){
			strcpy(findpath,newpath);
			return 0;
		}
		if(i==num_path){
			printf("no file\n");
			return -1;
		}
	}
	return 0;
}

