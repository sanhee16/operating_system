#include <stdio.h>
#include <stdlib.h>
//#include "file1.c"


int main(int argc, char * argv[])
{
	int res;
	int val = 3; //default
	if (argc == 2)
		val = atoi(argv[1]);
	
	res = foo(val);
	
	printf("%d-factorial: %d\n", val, res); 
	return 0;
}
