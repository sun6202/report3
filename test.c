#include <errno.h> // error management
#include <stdio.h> // standard I/O
#include <stdlib.h> // String casting, random, dynamic memory
#include <string.h> // String data type management
#include <unistd.h> // standard symbolic const and data type, processing command in shell
#include <sys/wait.h> // wait for process 
#include <sys/types.h> // use for thread type or process type
#include <fcntl.h> // file I/O & file remove, amend
#include <dirent.h> // management lib for directory
#include <signal.h> // management lib for specific process shutdown , or user interupt


static void child_habdler(int sig){
int status;
pid_t pid;
while((pid=waitpid(-1, &status, WNOHANG))>0){}
}
void park_handler(int sig){
printf("\n");
return;
}
int main(int argc, char**argv){
	int i=0;
	pid_t pid;
		signal(SIGCHLD,(void*)child_handler);
		signal(SIGINT,park_handler);
		signal(SIGQUIT,park_handler);
		signal(SIGTSTP,park_handler);
	switch(pid=fork()){
	case 0:{
		if(check!=1){
			signal(SIGINT, SIG_DEL);
			signal(SIGQUIT, SIG_DEL);
			signal(SIGTSTP, SIG_DEL);
		}
		else{
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGTSTP, SIG_IGN);
		}
			execvp(cmdvector[0], cmdvector);
			fatal("main()");}
	case -1:
		fatal("main()");
	default:
		if(check==1){
		break;}
		waitpid(pid,NULL,0)}
}

