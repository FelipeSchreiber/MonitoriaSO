#include <sys/wait.h> /* wait */
#include <errno.h>
#include "auxiliary_lib.h"
#include <fcntl.h>//O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO
#include <stdio.h>
#include <stdlib.h>   /* exit functions */
#define ReadEnd  0
#define WriteEnd 1
/*Esse programa tem por objetivo introduzir descritores de arquivo junto com pipes.
 O processo efetua o fork() num dado momento e é preciso que utilizem pipes para coordenarem
 a escrita e leitura em um arquivo texto. Para isso vamos adotar o seguinte protocolo:
 Existem 3 mensagens de controle:
 OK_Leitura -> informa que uma nova mensagem chegou no arquivo
 OK_Lida -> informa que a mensagem já foi lida 
 OK_Escrita -> pode escrever
no arquivo
FIM -> informa que não tem mais nada para escrever no arquivo, e portanto pode fechá-lo.

Por simplicidade, vamos considerar que apos receber a mensagem de lida o processo cede a vez
para que o outro possa escrever. Quando enviar o código OK_Leitura, enviar em seguida
*/

int main() 
{
  int pipeFDs_1[2]; // two file descriptors: Pai => Filho
  int pipeFDs_2[2]; //Filho => Pai
  if (pipe(pipeFDs_1) < 0) report_and_exit("pipeFD");
  if (pipe(pipeFDs_2) < 0) report_and_exit("pipeFD");
  pid_t cpid = fork();                                /* fork a child process */
  if (cpid < 0) report_and_exit("fork");              /* check for failure */

  if (0 == cpid) 
  {    /*** child ***/ 
    close(pipeFDs_1[WriteEnd]);//o filho só lê desse pipe
    close(pipeFDs_2[ReadEnd]);//o filho só escreve nesse pipe
    char *messages_son[]={"Hello UFRJ\n","NOOOOOO\n"};
    int total_msg = sizeof(messages_son)/sizeof(messages_son[0]);
    char filename[36];
    strcpy(filename,"");
    /* lseek(file_descriptor, -12L, SEEK_END); 
    NÃO É NECESSARIO MUDAR O CURSOR DE POSICAO DADO QUE O OPEN() CRIA UM NOVO CURSOR
    DIFERENTE DAQUELE VISTO NO ARQUIVO PAI
    */
    connect(messages_son,total_msg,
            pipeFDs_2[WriteEnd],pipeFDs_1[ReadEnd],filename);
    printf("Filho terminou\n");
    _exit(0);                                         /* exit and notify parent at once  */
  }
  else 
  {              /*** parent ***/
    char *messages_father[]={"Hello world\n","I'm your father\n","Goodbye\n"};
    int total_msg = sizeof(messages_father)/sizeof(messages_father[0]);
    close(pipeFDs_1[ReadEnd]);//o pai só escreve nesse pipe
    close(pipeFDs_2[WriteEnd]);//o pai só lê desse pipe
    char filename[36];
    strcpy(filename,"foo.txt\0");
    connect(messages_father,total_msg,
            pipeFDs_1[WriteEnd],pipeFDs_2[ReadEnd],filename);
    printf("Pai terminou\n");
    waitpid(cpid,NULL,0);                                       /* wait for child to exit */
    exit(0);                                          /* exit normally */
  }
  return 0;
}