#include "strcpy.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
    printf("O argumento passado foi: %s\n",argv[1]);
    printf("Recebido pela função: %s\n",str_cpy(argv[1]));
    exit(0);
}