#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define NUM_THREADS     5
#define MAX_PATH 256

pthread_mutex_t *mutex;
pthread_mutex_t *resultados;

void *PrintHello(void *threadid)
{
   long tid;
   tid = (long)threadid;
   pthread_mutex_lock(&mutex[tid]);
   printf("Se ha creado el Thread #%ld!\n", tid);
pthread_mutex_unlock(&mutex[tid]);
}
void *hola(void *threadid){

   long tid;
   tid = (long)threadid;
   	pthread_mutex_lock(&mutex[tid]);
	printf("Ahora imprimo %ld\n",tid);
	sleep(2);
	pthread_mutex_unlock(&mutex[tid]);
}

int main (int argc, char *argv[])
{

  pthread_t threads[NUM_THREADS];
  int rc;
  long t;

  /*Asignar el tamanio del arreglo de mutex para el bloqueo*/
  mutex = malloc(sizeof(pthread_mutex_t)*NUM_THREADS);

  for(t=0; t<NUM_THREADS; t++){
    printf("Principal: Creando Thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello, (char *)t);
    if (rc){
       printf("ERROR; return code from pthread_create() is %d\n", rc);
       exit(0);
    }
  }

  for(t=0; t<NUM_THREADS; t++){
    rc = pthread_create(&threads[t], NULL, hola, (char *)t);
    if (rc){
       printf("ERROR; return code from pthread_create() is %d\n", rc);
       exit(0);
    }
  }
  pthread_exit(NULL);

}
