#include <stdlib.h>
char *str_cpy(char * a)
{
    int len = 0;
    for(; a[len] != '\0'; len++);
    char *b = malloc((len+1)*sizeof(char));
    for(int i = 0; i < len+1; i++)
        b[i] = a[i];
    return b;
}