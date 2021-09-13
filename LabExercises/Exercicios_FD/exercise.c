#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
int main()
{
    int fd = open("foo.txt",O_RDWR);
    char buffer[36];
    int cur_pos = 0;
    read(fd,buffer,12);
    printf("Lido: %s\n",buffer);
    lseek(fd,-6, SEEK_CUR);
    strcpy(buffer,"master\n");
    write(fd,buffer,7);
    cur_pos = lseek(fd,0,SEEK_CUR);
    lseek(fd,-cur_pos,SEEK_CUR);
    read(fd,buffer,13);
    printf("Lido: %s\n",buffer);
    close(fd);
    return 0;
}