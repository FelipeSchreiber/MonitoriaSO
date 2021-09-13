#include <stdio.h>
#include <stdlib.h>
int *create_var_and_return()
{
    int a = 200;
    printf('Valor da variavel: %d\n',a);
    return &a;
}
int *create_var_and_return2()
{
    int *a = malloc(sizeof(int));
    *a = 200;
    printf("Valor da variavel: %d\n",*a);
    return a;
}
int main()
{
    int *addr = create_var_and_return();
    printf("Valor da variavel: %d\n",*addr);
    exit(0);
}