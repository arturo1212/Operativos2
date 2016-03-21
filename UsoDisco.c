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

char* directorios;
int* libre;
volatile int auxiliar = 0, resultado;

void *CrearThread(void *threadid)
{	
   long tid;
   tid = (long)threadid;
   printf("Se ha creado el Thread #%ld!\n", tid);
   auxiliar++;
   libre[tid] = 4;
}

void *hola(void *threadid){
    long tid;
    tid = (long)threadid;
	printf("Ahora imprimo %ld\n",tid);
	auxiliar++;
	libre[tid] = 0;						//Disponible

}

int main (int argc, char *argv[])
{

  pthread_t threads[NUM_THREADS];
  int rc,i;
  long t;

  /*Asignar el tamanio del arreglo de mutex para el bloqueo*/
  directorios = malloc(sizeof(char*)*NUM_THREADS);
  /*Asignar el tamanio del arreglo de hilos libres*/
  libre = malloc(sizeof(int)*NUM_THREADS);
  
  /*Inicializar cada posicion del arreglo de enteros en 5*/
  for(i=0;i<NUM_THREADS;i++){
    libre[i] = NUM_THREADS;
  }

  /* Creacion de los Threads*/
  	while(auxiliar<NUM_THREADS){
	  	for(t=0; t<NUM_THREADS; t++){
			if (libre[t] == 5){
			    printf("Principal: Creando Thread %ld\n", t);
			    libre[t] = 1;
			    rc = pthread_create(&threads[t], NULL, CrearThread, (char *)t);
			    if (rc){
			       printf("ERROR; return code from pthread_create() is %d\n", rc);
			       exit(0);
			    }
	  		}
	  	}
	}

	printf("Auxiliar primero:%d\n",auxiliar);	
	auxiliar = 0;
	while(auxiliar<NUM_THREADS){
	  for(t=0; t<NUM_THREADS; t++){
	  	if (libre[t] == 4){
	  		libre[t] = 1;
		    rc = pthread_create(&threads[t], NULL, hola, (char *)t);
		    if (rc){
		       printf("ERROR; return code from pthread_create() is %d\n", rc);
		       exit(0);
		    }
		  }
		}

	}
	printf("Auxiliar:%d\n",auxiliar);
}
