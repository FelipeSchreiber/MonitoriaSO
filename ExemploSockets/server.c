/*PROGRAMA QUE CRIA UM SERVIDOR. ELE ARMAZENA AS REQUISICOES EM UMA FILA E PROCESSA UMA A UMA. 
O PROCESSAMENTO RETORNA A SOMA DOS DIGITOS ENVIADOS PELO CLIENTE. CASO UM CLIENTE SE DESCONECTE DURANTE
O PROCESSAMENTO, O PROXIMO DA FILA É ATENDIDO. 3 THREADS SÃO UTILIZADAS: UMA PARA VERIFICAR A CLI, 
OUTRA PARA AS REQUISIÇÕES E POR FIM UMA PARA PROCESSAR A FILA

COMPILE: gcc -o server server.c FilaLista.c -pthread
CHAMADA: ./server

 Ele segue o seguinte protocolo:
 cliente envia "1|numero_pid" para o servidor. Servidor envia 2|soma_dos_digitos. 
 Cliente envia 3|numero_pid para encerrar a conexao*/

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <semaphore.h>
#include <errno.h> 
#include "FilaLista.h"
#define PORT 8080
#define DELIM "|" //define o caractere delimitador das mensagens
#define MAX_CLIENTS 128
unsigned long server_fd, new_socket, valread,max_sd,sd; 
struct sockaddr_in address; 
int opt = 1; 
int addrlen = sizeof(address); 
char buffer[1024] = {0};//buffer de leitura do socket
char *clientMessageParsed[3]; //vetor onde será armazenada a mensagem do cliente separada por DELIM
char hello[1024] = "Hello from server";
unsigned long int clientBusy = 0;//variavel que guarda o socket que está sendo atendido.
sem_t sm_Queue;//prevent multiple threads access Queue at the same time
sem_t sm_noClientBusy;//prevent poping queue before another process send "3" code
sem_t sm_ClientList;//prevent multiple threads access clients list at the same time
sem_t sm_MAIN;//Mutex that prevents Main executing before other threads finish
unsigned long int client_sockets[MAX_CLIENTS] = {0};
fd_set readfds;//Conjunto para ficar escutando caso um dos sockets clientes ou do servidor tenha mensagem
Fila *Q;//fila que armazena as requisições
int KEEP_HANDLING = 1;
int KEEP_READING_QUEUE = 1;
int KEEP_LISTENING = 1;
int activity = 0;

//funcao que checa se um socket está na lista
int checkSocketInClientList(int sockId)
{
	int i;
	for(i = 0;i<MAX_CLIENTS;i++)
	{
		if(sockId == client_sockets[i])
		{
			//dprintf(1,"ACHOU %d\n",sockId);
			return 1;
		}
	}
	//dprintf(1,"NAO ACHOU %d\n",sockId);
	return 0;
}

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

//funcao que soma todos os digitos presentes no pid
unsigned long int soma(unsigned long int pid)
{
   unsigned long int remainder, sum, t = pid;
   while (t != 0)
   {
      remainder = t % 10;
      sum       = sum + remainder;
      t         = t / 10;
   }
   return sum;
}

//funcao que retorna a soma de todos os digitos presentes no pid do processo cliente
void requestMessageReceived(Message *msg)
{
	//envia 2 que é o código da mensagem, junto com a soma
	sem_wait(&sm_ClientList);
	dprintf(1,"Processando msg do cliente %ld\n",msg->id);
	if(checkSocketInClientList(msg->fd))
	{
		strncpy(hello, "2|\0", 1024);
		unsigned long int sum = soma(msg->id);
		char soma_str[20];
		sprintf(soma_str,"%ld",sum);
		strcat(hello,soma_str);
		//dprintf(1,"Enviando msg %s para cliente %ld\n",hello,msg->id);
		send(msg->fd , hello , 1024 , 0);
		clientBusy = msg->fd;
	}
	else
	{
		sem_post(&sm_noClientBusy);
	}
	sem_post(&sm_ClientList);
}

//pega lock clientlist. Essa funcao retorna 0 caso a lista nao esteja vazia, ou 1 caso positivo.
int checkEmptySocketList()
{
	int vazia = 1;
	sem_wait(&sm_ClientList);
	for(int i=0;i<MAX_CLIENTS;i++)
	{
		if (client_sockets[i] != 0)
		{
			vazia = 0;
			break;
		}
	}
	sem_post(&sm_ClientList);
	return vazia;
}

//funcao que encerra o program quando o usuario digita "pare" no terminal
void processaPare()
{
	int is_empty = 0;
	is_empty = checkEmptySocketList();
	KEEP_HANDLING = 0;
	KEEP_LISTENING = 0;
	KEEP_READING_QUEUE = 0;
	//dprintf(1,"OK,%s\n",is_empty?"Socket List Vazio":"Socket List Não vazio");
	if(!is_empty)
	{
		sem_wait(&sm_ClientList);
		for(int i=0;i<MAX_CLIENTS;i++)
		{
			close(client_sockets[i]);
		}
		close(server_fd);
		sem_post(&sm_ClientList);
	}
}

//funcao que imprime na tela os pedidos pendentes quando o usuario digita "pedidos" no terminal
void processaPedidos()
{
	sem_wait(&sm_Queue);
	Lista *l = Q->ini;
	Message *msm;
	dprintf(1,"|Começo Lista PEDIDOS|\n=============================\n");
	while(l!=NULL)
	{
		msm = l->info;
		dprintf(1,"|processo: %ld|\n",msm->id);
		l = l->prox;
	}
	dprintf(1,"|Fim Lista PEDIDOS|\n=============================\n");
	sem_post(&sm_Queue);
}

//pega lock clientelist. Essa função fica lendo a linha de comando e espera que o usuario digite uma
//opcao: "pare" ou "pedidos". A primeira encerra o servidor e a segunda lista os pedidos pendentes
void *listenCLI()
{
	char input[1024]; 
	//STDIN 0
	//STDOUT 1
	//STDERR 2
	while(KEEP_LISTENING)
	{
		read(0,input,1024);
		input[strlen(input) - 1] = '\0';
		if (strcmp(input,"empty") != 0)
		{
			//dprintf(1,"%s (%s)--EVAL:%d\n","Input is:",input,(strcmp(input,"stop") == 0));
			if (strcmp(input,"pare") == 0)
			{
				processaPare();				
			}
			if (strcmp(input,"pedidos") == 0)
			{
				processaPedidos();
			}
			strncpy(input, "empty", sizeof(input));
		}
	}
}

//pega lock de cliente. Obtem a posicao disponivel na lista de sockets
int getFreeSpaceSocketList()
{
	int pos = -1;
	sem_wait(&sm_ClientList);
	for(int i=0;i<MAX_CLIENTS;i++)
	{
		if (client_sockets[i] == 0)
		{
			pos = i;
			break;
		}
	}
	sem_post(&sm_ClientList);
	return pos;
}
//pega lock de client_list
int saveSocketList(unsigned long int socketId)
{
	int found = getFreeSpaceSocketList();
	if(found != -1)
	{
		sem_wait(&sm_ClientList);
		dprintf(1,"Novo socket: %ld\n",socketId);
		client_sockets[found] = socketId;
		sem_post(&sm_ClientList);
	}
	else
	{
		close(socketId);	
	}
	return found;
}

//pega lock de clientes
void addClientSocketsToFD_SET()
{
	//add child sockets to set  
	sem_wait(&sm_ClientList);
	for (int i = 0 ; i < MAX_CLIENTS ; i++)   
	{   
        //socket descriptor  
	    sd = client_sockets[i]; 
                 
        //if valid socket descriptor then add to read list  
        if(sd > 0)   
        	FD_SET(sd , &readfds);   
                 
        //highest file descriptor number, need it for the select function  
        if(sd > max_sd)   
            max_sd = sd;   
    }
	sem_post(&sm_ClientList);  
}

//pega o lock clientlist
void checkIOServer()
{
	if (FD_ISSET(server_fd, &readfds)) 
	{ 
			if ((new_socket = accept(server_fd, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			//add new socket to array of sockets 
			if (saveSocketList(new_socket) == -1)
			{
				dprintf(1,"NEW_SOCKET %ld REFUSED\n",new_socket);
			}			
	} 
}

//pega o lock e coloca a mensagem na fila. Caso a mensagem seja do tipo "3" (despedida) 
void addMessageQueue(int socketFd)
{
	parse(buffer);
	Message*msg = malloc(sizeof(Message));
	msg->type = (*clientMessageParsed[0] - '0');
	msg->id = atoi((clientMessageParsed[1]));
	msg->fd = socketFd;

	if(msg->type == 3)
	{
		sem_post(&sm_noClientBusy);
		clientBusy = 0;	
		dprintf(1,"Goodbye socket %ld\n",msg->fd);
		//close(msg->fd);	
	}
	else
	{
		//dprintf(1,"addMessageQueue Waiting Queue lock\n");
		sem_wait(&sm_Queue);
		//dprintf(1,"addMessageQueue SUCCESS\n");
		fila_insere(Q,msg);
		//dprintf(1,"FILA INSERE OK\n");
		sem_post(&sm_Queue);
	}
}

//Pega o lock de clients_list e fila. Essa funcao checa se houve alguma operação de IO nos sockets de
// cliente: ou um cliente se desconectou ou está querendo enviar mensagem.
void checkIOclient()
{
	sem_wait(&sm_ClientList);  
	for (int i = 0; i < MAX_CLIENTS; i++)   
        {   
            
            sd = client_sockets[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    //getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);   
                    //printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    //Close the socket and mark as 0 in list for reuse 
                    dprintf(1,"SOCKET %ld disconnected\n",client_sockets[i]);
					//Cliente se desconectou enquanto estava sendo atendido. Nesse caso o lock é liberado e
					// a conexao interrompida.
					if(clientBusy == sd)
                    {
                    	sem_post(&sm_noClientBusy);
                    }
                    close( sd );   
                    client_sockets[i] = 0;
                    
                }  
                else 
                {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';
                    addMessageQueue(sd); 
                }
       
            }
      	}
    	sem_post(&sm_ClientList);                      
}
// Essa funcao lê a próxima mensagem da lista. Seguindo o protocolo estabelecido, a mensagem do tipo "1"
// é esperada. As mensagens do tipo "2" são enviadas diretamente ao cliente e não são postas na fila. 
// As mensagens do tipo "3" não são colocadas na fila.
// Caso a mensagem seja válida, ela é processada. O lock noClientBusy é obtido impedindo que outras requisições sejam
// atendidas antes de ser recebido a mensagem do tipo "3" correspondente ao cliente sendo atendido.
// Em caso da mensagem ser inválida, a conexao é encerrada e lock liberado.
void *processQueue()
{
	int vazia = 0;
	Message*msg;
	while(KEEP_READING_QUEUE)
	{
		sem_wait(&sm_noClientBusy);
		sem_wait(&sm_Queue);
		vazia = fila_vazia(Q);
		sem_post(&sm_Queue);
		if(!vazia)
		{
			sem_wait(&sm_Queue);
			msg = fila_retira(Q);
			dprintf(1,"Retirada da fila a msg do pid %ld e fd %ld\n",msg->id,msg->fd);
			sem_post(&sm_Queue);	
			switch(msg->type)
			{
				case 1:
					requestMessageReceived(msg);
					free(msg);
				break;
							
				default:
					dprintf(1,"Received an invalid message\n");
					sem_post(&sm_noClientBusy);
					close(msg->fd);
				break;
			}
		}
		else
		{
			sem_post(&sm_noClientBusy);
		}		
	}
}

//Pega os locks de client list e fila. Essa funcao é a responsavel por coordenar as conexoes.
void *handleConnection(void *arg)
{
	// Creating socket file descriptor IPV4 IPv6
	//A funcao socket cria um endpoint para a comunicacao. Para isso ela recebe 3 parametros:
	//AF_INET - Estabelece que o dominio da conexao é IPv4.
	//SOCK_STREAM - define o tipo da conexao, ou seja, que a conexao será confiavel, de mão dupla e orientada a conexao (TCP)
	//0 - especifica o protocolo utilizado para estabelecer a conexao. Normalmente só existe um único
	//protocolo por tipo, por isso o default é colocar 0 nessa opcao. Em caso de haver mais de uma pode ser colocado outro numero.
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, MAX_CLIENTS) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	struct timeval tv;
    	tv.tv_sec = 1;
	while(KEEP_HANDLING) 
	{ 
		//clear the socket set 
		FD_ZERO(&readfds); 
		FD_SET(server_fd, &readfds); 
		max_sd = server_fd; 
		addClientSocketsToFD_SET();
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv); 
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
		checkIOServer();
		checkIOclient();
	}
	sem_post(&sm_MAIN);
}

int main(int argc, char const *argv[]) 
{ 
	Q = fila_cria();
	int i=0;
	sem_init(&sm_MAIN,0,0);
	sem_init(&sm_noClientBusy,0,1);
	sem_init(&sm_Queue,0,1);
	sem_init(&sm_ClientList,0,1);
	pthread_t serverThreads[3];
	pthread_t thread_id;
	pthread_create(&thread_id,NULL,processQueue,NULL);
	serverThreads[2] = thread_id;
	pthread_create(&thread_id,NULL,handleConnection,NULL);
	serverThreads[1] = thread_id;
	printf("Server id %ld\n",thread_id);
	pthread_create(&thread_id,NULL,listenCLI,NULL);
	serverThreads[0] = thread_id;
	sem_wait(&sm_MAIN);
	return 0; 
} 

