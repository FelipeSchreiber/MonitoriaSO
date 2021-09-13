#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int str_len(char * a)
{
    int len = 0;
    for(; a[len] != '\0'; len++);
    return len + 1;
};

void str_len_wrapper(int num,...)
{
    va_list VA;
    va_start(VA,num);
    char *a = va_arg(VA,char*);
    int *len = va_arg(VA,int*);
    va_end(VA);
    *len = str_len(a);
};

int first_occurence_str(char * a,char token)
{
    int pos = 0;
    for(; a[pos] !=  token; pos++)
        if(a[pos]=='\0')
            return pos;
    return pos;
};

char *str_cpy(char * a)
{
    int len = str_len(a);
    char *b = malloc(len*sizeof(char));
    for(int i = 0; i < len; i++)
        b[i] = a[i];
    return b;
};

void str_cpy_wrapper(int num,...)
{
    va_list VA;
    va_start(VA,num);
    char *a = va_arg(VA,char*);
    char **b = va_arg(VA,char**);
    va_end(VA);
    //printf("Endereco passado: %p\n",b);
    *b = str_cpy(a);
};

char *str_sep(char*a,char token)
{
    int len = first_occurence_str(a,token);
    char *b = malloc((len+1)*sizeof(char));
    for(int i = 0; i<len; i++)
    {
        b[i] = a[i];
    }
    b[len] = '\0';
    return b;
}

char **split(char * a,char token,int*total)
{
    int count_tokens = 0;
    char *copy = str_cpy(a);
    //printf("Valor da copia: %s, len: %d\n",copy,str_len(copy));
    char *p;
    int len_a = str_len(a);
    for(int i = 0;(i<len_a) & (a[i] != '\0');i++)
    {
        if(a[i] == token)
        {
            count_tokens++;
        }
        //printf("%c\n",a[i]);
    }
    *total = count_tokens;
    //printf("Count: %d\n",count_tokens);
    char **string_parsed = malloc((count_tokens+1)*sizeof(char*));
    int cnt = 0;
    int next_valid_pos = 0;
    int next_occurence = 0;
    while(count_tokens>0)
  	{	
        count_tokens--;
        p = str_sep(&copy[next_valid_pos],token);
        //printf("P: %s\n",p);
  		string_parsed[cnt] = p;
  		cnt++;
        next_occurence = first_occurence_str(&copy[next_valid_pos],token);  
        next_valid_pos += 1 + next_occurence;
        //printf("Next valid pos %d and char %c\n",next_valid_pos,copy[next_valid_pos]);
        if(count_tokens == 0)
        {
            string_parsed[cnt] = str_cpy(&copy[next_valid_pos]);
        } 
  	}
    //printf("Fim loop %s\n",string_parsed[*total]);
  	free(copy);
    return string_parsed;
};

void split_wrapper(int num,...)
{
    va_list VA;
    va_start(VA,num);
    char *a = va_arg(VA,char*);
    char token = (char)va_arg(VA,int);
    char ***b = va_arg(VA,char***);
    int *total = va_arg(VA,int*);
    va_end(VA);
    *b = split(a,token,total);
    //printf("Addr : %p\n",*b); 
};