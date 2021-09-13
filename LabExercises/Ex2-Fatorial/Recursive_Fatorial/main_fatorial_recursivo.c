#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <unistd.h>
#define SIZE 100
//Passar a flag -rdynamic ao compilar

void
print_stack(void)
{
    int j, nptrs;
    void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
       would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

   for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

   free(strings);
}

int compute_fatorial(int n)
{
    print_stack();
    if(n == 0)
        return 1;
    if(n == 1)
        return 1;
    else
    {
        int result = compute_fatorial(n - 1);
        printf("Endereço da variável: %p\n",&result);
        printf("Endereço da funcao: %p\n",&compute_fatorial);
        return result*n;
    }
}

int main()
{
    int a = compute_fatorial(5);
    print_stack();
    printf("O resultado é: %d\n",a);
    exit(0);
}