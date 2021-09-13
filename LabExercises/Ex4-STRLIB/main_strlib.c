#include <stdio.h>
#include <stdlib.h>
#include "strlib.h"
int main()
{
    void (*funcionalidades[3])(int,...);
    funcionalidades[0] = &str_len_wrapper;
    funcionalidades[1] = &str_cpy_wrapper;
    funcionalidades[2] = &split_wrapper;

    char buffer[100];
    char *buffer2;
    char **string_parsed;
    int len;
    char opt[2];
    printf("Digite uma palavra:\n");
    scanf("%s",buffer);
    printf("Escolha uma opção:\n \
    1- Tamanho da string\n  \
    2- Copiar string\n \
    3- Split\n \
    ");
    scanf("%s",opt);
    int int_opt = atoi(opt)-1;
    printf("Buffer: %p\n",&buffer2);
    switch (int_opt)
    {
    case 0:
        (*funcionalidades[int_opt])(2,buffer,&len);
        printf("Resultado da operação escolhida: %d\n",len);    
        break;
    case 1:
        (*funcionalidades[int_opt])(2,buffer,&buffer2);
        printf("Resultado da operação escolhida: %s\n",buffer2);
        break;
    case 2:
        printf("Digite o token para fazer split\n");
        scanf("%s",opt);
        char **parsed;
        int *total = malloc(sizeof(int));
        split_wrapper(2,buffer,opt[0],&parsed,total);
        //printf("Addr : %p\n",parsed);
        printf("Resultado da operação escolhida:\n");
        for(int i = 0;i<*total+1;i++)
        {
            printf("--->%s<---\n",parsed[i]);
        }
    default:
        break;
    }
    exit(0);
}