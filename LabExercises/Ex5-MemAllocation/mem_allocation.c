#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

long allocate_mem2()
{
    long end,start;
    int *matriz = malloc(1000000*sizeof(int));
    start = getMicrotime();
    for(int j = 0; j<1000000; j++)
    {
        matriz[j] = 1;
    }
    end = getMicrotime();
    free(matriz);
    return (end - start);
}

long allocate_mem()
{
    long end,start;
    int **matriz = malloc(10000*sizeof(int*));
    for(int i = 0; i<10000; i++)
    {
        matriz[i] = malloc(100*sizeof(int));
    }
    start = getMicrotime();
    for(int i = 0; i<10000; i++)
    {
        for(int j = 0; j<100; j++)
        {
            matriz[i][j] = 1;
        }
    }
    end = getMicrotime();
    for(int i = 0; i<10000; i++)
    {
        free(matriz[i]);
    }
    free(matriz);
    return (end-start);
}

long run_1000x(long(*func)(void))
{
    long cumulative = 0;
    for(int i = 0; i<1000; i++)
    {
        cumulative += func();
    }
    return cumulative/1000;
}

int main()
{
    long avg_time;
    avg_time = run_1000x(allocate_mem2);
    printf("Total time: %ld\n",avg_time);
}