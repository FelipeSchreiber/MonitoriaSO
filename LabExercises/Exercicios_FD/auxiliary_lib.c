#include "auxiliary_lib.h"

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);    /** failure **/
}

//funcao para separar a mensagem recebida
char **parse(char *buff)
{
	char *copy,*p;
	int cnt = 0;
  char **clientMessageParsed = malloc(3*sizeof(char*));
	copy = strdup(buff);
	while(p = strsep(&copy,DELIM))
  	{	
  		clientMessageParsed[cnt] = strdup(p);
  		cnt++;
  	}
  	free(copy);
  return clientMessageParsed;
}

/*Funcao que converte o numero inteiro em string de acordo com o 
codigo da mensagem do tipo OTHER*/
char* other_translator(int code)
{
  char* meaning = malloc(sizeof(char)*20);
  switch (code)
  {
  case FDS:
    strcpy(meaning,"FDS"); 
    break;
  case PID:
    strcpy(meaning,"PID"); 
    break;
  case FILENAME:
    strcpy(meaning,"FILENAME"); 
    break;
  case SEND_FDS:
    strcpy(meaning,"SEND_FDS"); 
    break;
  case SEND_PID:
    strcpy(meaning,"SEND_PID"); 
    break;
  case SEND_FILENAME:
    strcpy(meaning,"SEND_FILENAME"); 
    break;
  default:
    strcpy(meaning,"");
    break;
  }
  return meaning;
}

/*Funcao que converte o numero inteiro em string de acordo com o codigo
da mensagem do tipo CONTROL*/
char* control_translator(int code)
{
  char* meaning = malloc(sizeof(char)*20);
  switch (code)
  {
    case OK_Leitura:
      strcpy(meaning,"OK_Leitura"); 
      break;
    case OK_Lida:
      strcpy(meaning,"OK_Lida"); 
      break;
    case OK_Escrita:
      strcpy(meaning,"OK_Escrita"); 
      break;
    case FIM:
      strcpy(meaning,"FIM"); 
      break;
    case OK_FIM:
      strcpy(meaning,"OK_FIM"); 
      break;
    default:
      strcpy(meaning,"");
      break;
  }
  return meaning;
}

/*funcao que recebe uma variavel onde será armezado o tipo de controle e 
o descritor de arquivo de onde deve ser efetuada a leitura. Caso a mensagem recebida 
tenha também um valor indicando (a quantidade de bytes que precisam ser lidos ou o 
descritor de arquivo onde as mensagens são escritas ou o pid do pai), a funcao 
retorna esse valor. Do contrario "-1" é retornado. No caso de o 3o argumento da mensagem 
recebida ser uma string, o mesmo será interpretado como o nome do arquivo que será aberto.*/
int readFromPipe(int *conversion,int read_end,message_type *msg_type,char *filename)
{
    char buffer[40];
    char * translated;
    read(read_end, buffer, 40);
    char **parsed_messages = parse(buffer);
    *msg_type = strtol(parsed_messages[0],NULL,10);
    (*conversion) = strtol(parsed_messages[1],NULL,10);
    if((*msg_type == CONTROL))
    {
        // translated = control_translator(*conversion);
        // dprintf(STDOUT_FILENO,"%d LIDO DO PIPE CONTROL %s -> %s\n",
        //                                 getpid(),translated,buffer);
        // free(translated);
    }
    if((*msg_type) ==  OTHER)
    {  
        if(*conversion == FILENAME)
          strcpy(filename,parsed_messages[2]);
        // translated = other_translator(*conversion);
        // dprintf(STDOUT_FILENO,"%d LIDO DO PIPE OTHER %s -> %s\n",
        //                               getpid(),translated,buffer);
        // free(translated); 
    }
    free(parsed_messages[0]);
    free(parsed_messages[1]);
    if((parsed_messages[2] != NULL) && !( (*msg_type == OTHER) && (*conversion == FILENAME)))
    {
        int payload = strtol(parsed_messages[2],NULL,10);
        free(parsed_messages[2]);
        free(parsed_messages);
        return payload;
    }
    free(parsed_messages);
    return -1;
}

/*Funcao que recebe um inteiro e uma string indicando como interpretar esse numero, 
tal como o descritor de arquivo onde deve ser efetuada a escrita*/
void writeInPipe(int msg_no,int virtualFileAddr,char * msg_header)
{
  char stringConvertedInt[40];
  sprintf(stringConvertedInt, "%s|%d",msg_header,msg_no);
  //printf("%d escreveu no pipe: %s\n",getpid(),stringConvertedInt);
  //sleep(3);
  write(virtualFileAddr, &stringConvertedInt, sizeof(stringConvertedInt));
}

/*Funca que escreve no arquivo uma mensagem determinada por "message" e envia um código 
pelo pipe indicando que a leitura está disponível*/
void send_message(int fd,char *message,int write_end)
{
  int msg_len = 0;
  char buffer[20];
  msg_len = strlen(message);
  write(fd,message,msg_len);
  sprintf(buffer,"%d|%d",CONTROL,OK_Leitura);
  writeInPipe(msg_len,write_end,buffer);
}

/*
funcao que com base na mensagem recebida executa uma acao e
retorna a proxima mensagem que precisa ser enviada
Ela recebe:
msg_rcvd -> código da mensagem indicando como deve ser interpretado o "payload"
payload -> um inteiro que pode ser o descritor de arquivo, o pid do pai ou numero
de bytes a serem lidos.
messages_to_send -> um vetor de strings a serem escritas uma de cada vez no arquivo
cur_message -> indica qual é a proxima mensagem a ser enviada do vetor "messages_to_send"
other_proc_finished -> indica se o outro processo com o qual se comunica já terminou de
enviar as mensagens
write_pipe -> indica a ponta onde o processo deve escrever
read_pipe -> indica a ponta onde o processo deve ler
fd -> indica qual arquivo está sendo manipulado
pid -> indica o pid do outro processo com o qual está comunicando
total_msg -> indica a quantidade de mensagens que precisam ser enviadas
 */
int state_machine_control(control_messages msg_rcvd,int payload,char**messages_to_send,
                  int *cur_message,int *other_proc_finished,
                  int write_pipe,int read_pipe,int *fd,pid_t *pid,int total_msg)
{
  switch (msg_rcvd)
  {
    case OK_Leitura:
    {
        char buf[20];
        int last_pos;
        last_pos=read(*fd,buf,payload);
        if(last_pos == -1)
          printf("%s\n",strerror(errno));
        buf[payload] = '\0';
	      dprintf(STDOUT_FILENO,"%d leu do arquivo %d|%d bytes: %s\n",getpid(),payload,last_pos,buf);/* echo to the standard output */
        sleep(3);
        return OK_Lida;
    }
    break;
    case OK_Escrita:
    {
    /*processo disse que podia escrever, mas nao tem mais nada para escrever*/
      if(*cur_message == total_msg)
      {
        return FIM;
      }
      send_message(*fd,messages_to_send[*cur_message],write_pipe);
      (*cur_message)++;
      return IDLE;//A funcao send_message já envia OK_Leitura
    }
    break;
    case OK_Lida:
    {
      return OK_Escrita;
    }
    break;
    case FIM:
    {
      if(*cur_message < total_msg)//checa se ainda tem mensagem para enviar
      {
        send_message(*fd,messages_to_send[*cur_message],write_pipe);
        (*cur_message)++;
        *other_proc_finished = 1;
        return IDLE;
      }
      return OK_FIM;
    }
    break;
    case OK_FIM:
    {
      dprintf(STDOUT_FILENO,"%d Closing files and pipes....\n",getpid());
      close(*fd);
      close(write_pipe);
      close(read_pipe);
      return OK_FIM;
    }
    break;
  }
}

void tarefa1(char * path)
{
  pid_t cpid = fork();
  if (cpid < 0) report_and_exit("fork");              /* check for failure */
  if (0 == cpid) 
  {    /*** child ***/       
    execlp("/bin/ls","ls","-la",path,NULL);
    _exit(0);
  }
  else
  {
    waitpid(cpid, NULL, 0);
  }
}

int state_machine_other(other_messages msg_rcvd,int payload,
                  int write_pipe,int read_pipe,int *fd,pid_t *pid,char*filename)
{
  switch (msg_rcvd)
  {
      case PID:
      {
        *pid = payload;
        //printf("Pid received %d\n",*pid);
        char buffer[40];
        if(*fd != -1)
        {
          char fd_path[64];  // actual maximal length: 37 for 64bit systems
          snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd",*pid);
          tarefa1(fd_path);
          snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%d",*pid,*fd);
          printf("Arquivo mapeado no processo pai: %s\n",fd_path);
          sprintf(buffer,"%d",OTHER);
          writeInPipe(SEND_FILENAME,write_pipe,buffer);
        }
        else
        {
          sprintf(buffer,"%d",OTHER);
          writeInPipe(SEND_FDS,write_pipe,buffer);
        }
        return IDLE;
      }
      break;
      case FDS:
      {
        *fd = payload;
        //printf("FD received %d\n",*fd);
        char buffer[40];
        if(*pid != -1)
        {
          char fd_path[64];  // actual maximal length: 37 for 64bit systems
          snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd",*pid);
          tarefa1(fd_path);
          snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%d",*pid,*fd);
          printf("Arquivo mapeado no processo pai: %s\n",fd_path);
          sprintf(buffer,"%d",OTHER);
          writeInPipe(SEND_FILENAME,write_pipe,buffer);
        }
        else
        {
          sprintf(buffer,"%d",OTHER);
          writeInPipe(SEND_PID,write_pipe,buffer);
        }
        return IDLE;
      }
      break;     
      case FILENAME:
      {
        (*fd) = open(filename,O_RDWR);
        //printf("nome arquivo %s\n",filename);
        char msg_type[5];
        snprintf(msg_type, sizeof(msg_type),"%d",CONTROL);
        writeInPipe(OK_Escrita,write_pipe,msg_type);
        return IDLE;
      }
      break;
      case SEND_PID:
      { 
        char buffer[40];
        pid_t father_pid = *pid;
        sprintf(buffer,"%d|%d",OTHER,PID);
        writeInPipe(father_pid,write_pipe,buffer);
        return IDLE;
      }
      break;
      case SEND_FDS:
      {
        char buffer[40];
        sprintf(buffer,"%d|%d",OTHER,FDS);
        writeInPipe(*fd,write_pipe,buffer);       
        return IDLE;
      }
      break;
      case SEND_FILENAME:
      {
        char buffer[40];
        sprintf(buffer,"%d|%d|%s",OTHER,FILENAME,filename);
        //printf("Sending %s\n",buffer);
        write(write_pipe,buffer,sizeof(buffer));  
        return IDLE;
      }       
      break;
  }
}

/*Funcao que recebe dois descritores de arquivo, que correspondem as pontas de
leitura e escrita dos pipes, as mensagens que precisam ser enviadas e estabelece a
conexao. Caso essa funcao seja chamada pelo processo pai, ele será o responsável por iniciar a 
comunicação*/
void connect(char **messages_to_send,int total_msg,
              int write_end,int read_end,char*filename)
{
  int file_descriptor,cur_message, other_proc_finished, payload;
  pid_t pid = -1;
  cur_message = other_proc_finished = 0;
  int msg_rcvd,msg_send;
  message_type msg_type = OTHER;
  file_descriptor = msg_rcvd = msg_send = -1;
  if(strcmp(filename,"") != 0)
  {
    char buffer[40];
    pid = getpid();
    file_descriptor = open(filename, O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
    if(file_descriptor < 0)
    {
      dprintf(STDOUT_FILENO,"AN ERROR OCCURED: %s\n",strerror(errno));
    }
    dprintf(STDOUT_FILENO,"Father pid %d \n",pid);
    sprintf(buffer,"%d|%d",OTHER,PID);
    writeInPipe(pid,write_end,buffer);
  }
  while(msg_rcvd != OK_FIM)
  {  
      payload = readFromPipe(&msg_rcvd,read_end,&msg_type,filename);     
      switch (msg_type)
      {
        case OTHER:
          msg_send = state_machine_other(msg_rcvd,payload,
                    write_end,read_end,&file_descriptor,
                    &pid,filename);
        break;
        case CONTROL:
          msg_send = state_machine_control(msg_rcvd,payload,messages_to_send,
                    &cur_message,&other_proc_finished,
                    write_end,read_end,&file_descriptor,&pid,total_msg);
        break;
      }
      if(msg_send != IDLE)
      {
        char char_msg_type[5];
        snprintf(char_msg_type, sizeof(char_msg_type),"%d",CONTROL);
        writeInPipe(msg_send,write_end,char_msg_type);
      }
  }                                     
}
