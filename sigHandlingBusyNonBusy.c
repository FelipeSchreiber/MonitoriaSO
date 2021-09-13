#include <stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<sys/wait.h>
#include<unistd.h>

void sig_handler(int signum)
{
  //Return type of the handler function should be void
	printf("\nInside handler function - %d Caught - ENDING EXECUTION\n",signum);
	raise(SIGINT);
}

void sig_handler2(int signum)
{
    pid_t chld_pid, wpid;
    int status = 0;
	printf("\nInside handler SIGUSR2\n");
    chld_pid = fork();
    if(chld_pid == 0)
    {
        execlp("/bin/ping","ping","8.8.8.8","-c","5",NULL);
    }
    else 
    {
        while ((wpid = wait(&status)) > 0);
        printf("Child terminated\n");
    }  
}


void sig_handler3(int signum)
{
    pid_t chld_pid, wpid;
    int status = 0;
	printf("\nInside handler SIGUSR2\n");
    chld_pid = fork();
    if(chld_pid == 0)
    {
        execlp("/bin/ping","ping","8.8.8.8","-c","5",NULL);
    }
    else 
    {
        while ((wpid = wait(&status)) > 0);
        printf("Child terminated\n");
    }  
}

int main(int argc,char *argv[]){
  signal(SIGTERM,sig_handler); 
  signal(SIGUSR1,sig_handler2);
  signal(SIGUSR2,sig_handler3);
  printf("\nModo de uso:\n Passe o argumento 'b' para executar com busy wait, por default será executado sem busy wait\n");
  printf("\nThis process id is: %d\n",getpid());
  if(argc > 1 && argv[1][0] == 'b')
  {
      //execlp("/sbin/ping","ping","8.8.8.8","-c","50",NULL);	
	  while(1)
	  {
	    printf("Inside main function\n");
	    sleep(1); 
	  }
  }
  else
  {
	  while(1)
	  {
	    printf("Inside main function\n");	
	    pause();
	  }
  }
 return 0; 
}
