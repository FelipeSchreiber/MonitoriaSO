#define MAX 20
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void *client(void *param);
void *barber(void *param);

pthread_cond_t chairs_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t client_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t barber_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t chairs_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t barber_lock = PTHREAD_MUTEX_INITIALIZER;

int num_chairs;
int clientWait;
int cur_clients = 0;
int barber_available=0;

int main(int argc, char *argv[]) {
	pthread_t barberid;
	pthread_t clientids[MAX];
	
	printf("Main thread beginning\n");
   /* 1. Get command line arguments */
   int runTime,clients,i;
   if (argc != 5){
	   printf("Please enter 4 arguments: <Program run time> <Number of clients>\n");
	   printf("<Number of chairs> <Client wait time>\n");
	   exit(0);
   }
   runTime = atoi(argv[1]);
   clients = atoi(argv[2]);
   num_chairs = atoi(argv[3]);
   clientWait = atoi(argv[4]);
   /* 3. Create barber thread. */
   pthread_create(&barberid, NULL, barber, NULL);
   printf("Creating barber thread with id %lu\n",barberid);
   /* 4. Create client threads.  */
   for (i = 0; i < clients; i++){
	   pthread_create(&clientids[i], NULL, client, NULL);
	   printf("Creating client thread with id %lu\n",clientids[i]);
   }
   /* 5. Sleep. */
   printf("Main thread sleeping for %d seconds\n", runTime);
   sleep(runTime);
   /* 6. Exit.  */
   printf("Main thread exiting\n");
   exit(0);
}

void *barber(void *param) {
   int worktime;
  
   while(1) {
      /* wait for a client to become available */
      pthread_mutex_lock(&client_lock);
      while(cur_clients == 0)
      {
          pthread_cond_wait(&client_cond, &client_lock);   
      }
      cur_clients--;
      pthread_mutex_unlock(&client_lock);
      /* wait for mutex to access chair count (chair_mutex) */
      pthread_mutex_lock(&chairs_lock);
      /* increment number of chairs available */
      num_chairs += 1;
      printf("Barber: Taking a client. Number of chairs available = %d\n",num_chairs);
      barber_available = 1;
      /* signal to client that barber is ready to cut their hair (sem_barber) */
      pthread_cond_signal(&barber_cond);
      /* give up lock on chair count */
      pthread_mutex_unlock(&chairs_lock);
      /* generate random number, worktime, from 1-4 seconds for length of haircut.  */
      worktime = (rand() % 4) + 1;
      /* cut hair for worktime seconds (really just call sleep()) */
      printf("Barber: Cutting hair for %d seconds\n", worktime);
      sleep(worktime);
    } 
}

void *client(void *param) {
   int waittime;

   while(1) {
      /* wait for mutex to access chair count (chair_mutex) */
      pthread_mutex_lock(&chairs_lock);
      /* if there are no chairs */
	  if(num_chairs <= 0){
           /* free mutex lock on chair count */
		   printf("Client: Thread %u leaving with no haircut\n", (unsigned int)pthread_self());
		   pthread_mutex_unlock(&chairs_lock);
	  }
      /* else if there are chairs */
	  else{
           /* decrement number of chairs available */
		   num_chairs -= 1;
		   printf("Client: Thread %u Sitting to wait. Number of chairs left = %d\n",(unsigned int)pthread_self(),num_chairs);
         cur_clients+=1;
         /* signal that a customer is ready (sem_client) */
         pthread_cond_signal(&client_cond);
         /* free mutex lock on chair count */
	 pthread_mutex_unlock(&chairs_lock);
         /* wait for barber (sem_barber) */
         pthread_mutex_lock(&barber_lock);
         while(barber_available == 0)
         {
		      pthread_cond_wait(&barber_cond,&barber_lock);
         }
         pthread_mutex_unlock(&barber_lock);
         /* get haircut */
		   printf("Client: Thread %u getting a haircut\n",(unsigned int)pthread_self());
         barber_available = 0;
	  }
      /* generate random number, waittime, for length of wait until next haircut or next try.  Max value from command line. */
	  waittime = (rand() % clientWait) + 1;
      /* sleep for waittime seconds */
	  printf("Client: Thread %u waiting %d seconds before attempting next haircut\n",(unsigned int)pthread_self(),waittime);
	  sleep(waittime);
     }
}
