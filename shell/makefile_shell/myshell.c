#include "header.h"

int main(int argc, char *argv[]){
	num_path=0;
	memset(store_path,0,sizeof(store_path));
	path_token();
	int space=0;
	while(1){
		memset(str,0,sizeof(str));
		str_num,space=0;
		printSiSH();
		fgets(command,sizeof(command),stdin);
		command[strlen(command)-1]='\0';
		if((strcmp(command, "quit")==0))
			break;

		space = token(command);
		if(space==0){
			if(str[0][0]=='$'){
				getenv_var(str[0]+1);
			}
			else if(strcmp(str[0],"echo")==0){
				stat_file();

				for(int a=1; ;a++){
					if(str[a]==NULL) break;
					
					if(str[a][0]=='$'){			
						char* env;
						env = getenv(str[a]+1);
						printf("%s",env);
					}
					else{
						printf("%s",str[a]);
					}
					printf(" ");
				}
				printf("\n");
			}
			else{
				int find = stat_file();
				if(find==0){
					execute(str);
				}
				else if(find==-1){
					printf("cannot find program\n");
				}
			}
		}
	}
	return 0;
}

int printSiSH(){
	char prompt[80];
	getcwd(prompt, 80);
	fprintf(stdout,"SiSH %s ",prompt);
	return 0;
}
