#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h> //STDOUT_FILENO read, write, pipe, _exit
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DELIM "|" //define o caractere delimitador das mensagens
/*define headers que antecipam o número*/
typedef enum {OTHER,CONTROL}message_type;
typedef enum {OK_Leitura,OK_Lida,OK_Escrita,FIM,OK_FIM,IDLE}control_messages;
typedef enum {FDS,PID,FILENAME,SEND_FDS,SEND_PID,SEND_FILENAME}other_messages;

void report_and_exit(const char* msg);
char **parse(char *buff);
char* control_translator(int code);
char* other_translator(int code);
int readFromPipe(int *conversion,int read_end,message_type *msg_type,char *filename);
void writeInPipe(int msg_no,int virtualFileAddr,char *msg_header);
void send_message(int fd,char *message,int write_end);
void tarefa1(char * path);

int state_machine_control(control_messages msg_rcvd,int payload,char**messages_to_send,
                  int *cur_message,int *other_proc_finished,
                  int write_pipe,int read_pipe,int *fd,pid_t *pid,int total_msg);

int state_machine_other(other_messages msg_rcvd,int payload,
                  int write_pipe,int read_pipe,int *fd,pid_t *pid,char*filename);

void connect(char **messages_to_send,int total_msg,
              int write_end,int read_end,char*filename);