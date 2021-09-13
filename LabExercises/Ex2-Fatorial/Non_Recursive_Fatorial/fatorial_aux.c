#include "fatorial_aux.h"
#include <execinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

void print_stack(void)
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
};

int compute_fatorial(int n)
{
    if(n == 0)
        return 1;
    if(n == 1)
        return 1;
    int result = 1;
    while (n>1)
    {
        print_stack();
        result *= n;
        n--;   
    }
    return result;
};
