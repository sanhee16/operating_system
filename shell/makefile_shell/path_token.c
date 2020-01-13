#include "header.h"

int path_token(){
	char *saveptr, *str_var;
	char save_path[200];
	int j=-0;
	char* basic_path;

	basic_path=getenv("PATH");
	strcpy(save_path,basic_path);

	for (j=0,str_var=save_path; ;str_var=NULL,j++) {
		basic[j] = strtok_r(str_var, ":", &saveptr);
		if (basic[j] == NULL)
			break;
		strcpy(store_path[j],basic[j]);
		num_path++;
	}
	num_path--;
	return 0;
}

