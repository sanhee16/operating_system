#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>


//function
int printSiSH();
int stat_file();
int getenv_var(char newenv[10]);
int token();
int path_token();
int execute(char *argv[]);

//global variable
char* str[40];
char store_path[70][70];
char* basic[100];
int str_num, num_path;
char command[256];
char* findpath;
typedef struct tm time_str;

