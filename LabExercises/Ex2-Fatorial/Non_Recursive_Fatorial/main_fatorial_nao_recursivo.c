#include <stdlib.h>
#include <stdio.h>
#include "fatorial_aux.h"
//Passar a flag -rdynamic ao compilar

int main()
{
    int a = compute_fatorial(5);
    print_stack();
    printf("O resultado é: %d\n",a);
    exit(0);
}