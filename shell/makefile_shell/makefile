OBJECTS = myshell.o execute.o path_token.o token.o stat_file.o getenv_var.o

test : $(OBJECTS)
	gcc -o test $(OBJECTS)
	
myshell.o : myshell.c
	gcc -c myshell.c

execute.o :  execute.c
	gcc -c execute.c

path_token.o : path_token.c
	gcc -c path_token.c

token.o : token.c
	gcc -c token.c

stat_file.o : stat_file.c
	gcc -c stat_file.c

getenv_var.o : getenv_var.c
	gcc -c getenv_var.c

rm :
	rm *.o test
