/*PROGRAMA QUE CRIA N THREADS, FAZ R REQUISICOES CADA E DORME POR K SEGUNDOS ENTRE CADA REQUISICAO.

COMPILE: gcc -o client client.c
CHAMADA: ./client R K N. 

 Ele segue o seguinte protocolo:
 cliente envia "1|numero_pid" para o servidor. Servidor envia 2|soma_dos_digitos. 
 Cliente envia 3|numero_pid para encerrar a conexao*/

#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <time.h>
#include <sys/wait.h>
#define PORT 8080 
#define DELIM "|" //define o caractere delimitador das mensagens
char *clientMessageParsed[3];

//funcao para separar a mensagem recebida
void parse(char *buff)
{
	char *copy,*p;
	int cnt = 0;
	copy = strdup(buff);
	while(p = strsep(&copy,DELIM))
  	{	
  		clientMessageParsed[cnt] = strdup(p);
  		cnt++;
  	}
  	free(copy);
}

int Connect()
{
	int sock = 0; 
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 
	
	while((connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)) 
	{ 
		printf("\nConnection Failed \n");
	}
	return sock;
}

int request(char*buff,char*str_PID,int sock)
{
	int valread;
	strncpy(buff, "1|\0", 1024);
	strcat(buff,str_PID);
	dprintf(1,"Enviando mensagem %s\n",buff);
	send(sock , buff , 1024 , 0 ); 
	valread = read(sock , buff, 1024); 
	if(valread == 0 )
	{	
		dprintf(1,"A PROBLEM OCCURRED\n");
	}
	//printf("%s\n",buff );
	parse(buff);
	if (strcmp(clientMessageParsed[0],"2") == 0)
	{
		return 1;		
	}
	return 0;
}

void goodbye(char *buff,char*str_PID,int sock)
{
	strncpy(buff, "3|\0",1024);
	strcat(buff,str_PID);
	dprintf(1,"Enviando mensagem %s\n",buff);
	send(sock , buff , 1024 , 0);	
}

void makeRRequests(int r,char*buff,char*str_PID,int sock,int k)
{
	int ok;
	for(int j=0;j<r;j++)	
	{
		ok = request(buff,str_PID,sock);
		sleep(k);
		if(ok)
		{
			int sum = atoi(clientMessageParsed[1]);
			dprintf(1,"GOT RESULT: soma dos digitos de %s é: %d\n",str_PID,sum);			
			goodbye(buff,str_PID,sock);
		}
	}
	close(sock);
}

int main(int argc, char const *argv[]) 
{ 
	int r = atoi(argv[1]);//numero de requisicoes
  	int k = atoi(argv[2]);//tempo de soneca
  	int n = atoi(argv[3]);//numero de threads
  	int *pids = malloc(n*sizeof(int));
	for (int i = 0; i < n; i++)
  	{
    		if ((pids[i] = fork()) < 0)
    		{
    			exit(1);
    		}
    		else 
    		{
    			int process_id = getpid();
				char str_pid[1024];
				char buffer[1024] = {0};
				sprintf(str_pid,"%d|",process_id); 
				int sock = Connect();
				makeRRequests(r,buffer,str_pid,sock,k);
    		}
    	}
    	int status;
	int pid;
	while (n > 0)
	{
		pid = wait(&status);
		--n;
	}
	return 0; 
} 


