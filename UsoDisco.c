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

#define NUM_THREADS     8

/*------------------------------ESTRUCTURAS--------------------------------*/

/*Nodos para las Colas*/
struct Node {
  char* data;
  struct Node* next;
};

struct Hilos { 
	pthread_t thread;
	char* direc_asig; 	// Directorio de trabajo actual
	int pesoArch;		// Peso que ha contado el Hilo
	char* nuevosD;		// Lista de directorios nuevos encontrados.
	int libre;			// Indica si el hilo esta libre o no.
};

/*------------------------------------------------------------------------*/

/*----------------------------VARIABLES GLOBALES -------------------------*/
int* libre, resultados;
volatile int resultado = 0, completo=0, esMaestro = 0;
pthread_mutex_t* mutexdir;
pthread_mutex_t* mutexpeso;
pthread_mutex_t mutexcola;
struct Hilos* arrayH;

/* Variables para la cola de Directorios*/
struct Node* front = NULL;
struct Node* rear = NULL;

/*-------------------------------------------------------------------------*/



/* ------------------------------------------------------------------------
-----------------------FUNCIONES PARA COLA DE DIRECTORIOS------------------
---------------------------------------------------------------------------*/

// ENCOLAR
void Enqueue(char * x) {
  struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
  temp->data = (char *)malloc(strlen(x)+1);
  strcpy(temp->data,x); 
  temp->next = NULL;
  if(front == NULL && rear == NULL){
    front = rear = temp;
    return;
  }
  rear->next = temp;
  rear = temp;
}

// DESENCOLAR
void Dequeue() {
  struct Node* temp = front;
  if(front == NULL) {
    printf("Queue is Empty\n");
    return;
  }
  if(front == rear) {
    front = rear = NULL;
  }
  else {
    front = front->next;
  }
  free(temp);
}

// OBTENER EL PRIMERO
char* Front() {
  if(front == NULL) {
    printf("Queue is empty\n");
    return;
  }
  return front->data;
}

// IMPRIMIR LA COLA COMPLETA
void Print() {
  struct Node* temp = front;
  while(temp != NULL) {
    printf("%s ",temp->data);
    temp = temp->next;
  }
  printf("\n");
}
/*--------------------------------------------------------------
----------------------------------------------------------------
---------------------------------------------------------------*/


/* ---------------------------------Funcion util para concatenar -------------------------*/
char* concat(char *s1, char *s2){
	/*	Esta funcion ocncatena dos strings y retorna un apuntador a caracteres con la concatenacion
		
		VARIABLES:
			result 	: apuntador a caracteres a retornar con la concatenaicon
			s1 		: primer string a concatenar
			s2 		: segundo string a concatenar
	*/
	char *result;
    result = (char*)malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
/*------------------------------------------------------------------------------------------*/


int Buscar(char* cwd){
	DIR *dir;
	struct stat st;
	struct dirent *ent;
    char filename[4096];
    char* sepStr = "*||*";
    int resultadoT = 0;

		if ((dir = opendir (cwd)) != NULL) {
			printf("Los archivos en el directorio %s son: \n",cwd);
	  		while ((ent = readdir (dir)) != NULL) {
	  			/*Verificamos que no estemos tomando el directorio anterior*/
	  			if(ent->d_name[0]!='.' && ent->d_name[1]!='.'){
	  				/* En filename almacenamos la ruta completa del archivo a examinar*/ 					
		    		snprintf(filename,sizeof filename, "%s/%s", cwd, ent->d_name);

		    		/*Usamos la estructura stat para obtener atributos
		    		importantes, como el peso.*/
					stat(filename, &st);	

					/* Si el archivo es regular, sumamos su peso*/										
					if(S_ISREG(st.st_mode)){													
						printf ("	Regular : %s %d \n ", filename, (int)st.st_blocks);
						resultadoT = resultadoT + (int)st.st_blocks;
					}

					/* Si el archivo es de tipo directorio, lo agregamos a la lista*/
					else if(S_ISDIR(st.st_mode)){
						pthread_mutex_lock (&mutexcola);
						Enqueue(filename);	
						pthread_mutex_unlock (&mutexcola);	

						printf ("	Directorio : %s %d \n ", filename, (int)st.st_blocks);	
						resultadoT = resultadoT + (int)st.st_blocks;
					}
				}
	  		}
	  		/*Cerramos el directorio y verificamos si el hilo maestro
	  		ejecuto la funcion*/
	  		closedir (dir);	
	  		return resultadoT;				
		} 
		/*Mensaje de error si no se pudo abrir el archivo*/	
		else {			
	  		perror ("");
	  		printf("NO PUDE ABRIR: %s\n",cwd);
	  		return 1;
		}

}

void *BuscarThread(void *threadid){
	long tid;
	int i;
	char  direct[4096];
   	tid = (long)threadid;
   	while(1){

   		if (arrayH[tid].libre == 1){
  			snprintf(direct,sizeof direct, "%s",arrayH[tid].direc_asig);

   			i = Buscar(direct);

   			/* Suma de pero total al espacio asignado en la estructura*/ 
   			arrayH[tid].pesoArch += i; 	
   			completo--;	
   			arrayH[tid].libre = 0;
    		usleep(0);

    	}
	}
}




int main (int argc, char *argv[])
{
	/* Cosas para la busqueda en directorios*/

    /*Cosas para los threads*/
  	pthread_t threads[NUM_THREADS];
  	int rc , i;

  	/*Directorio inicial*/
  	char cwd[1024], direct[4096];

  	/*Inicializar los mutex*/
  	pthread_mutex_init(&mutexcola, NULL); 

 	/*Inicializamos el arreglo de estructuras que contendra la informacion de los hilos.*/
	arrayH = malloc(NUM_THREADS*sizeof(struct Hilos));

/* Revisar los directorios del master para agregarlos a la cola*/
	if (getcwd(cwd, sizeof(cwd)) != NULL){
        fprintf(stdout, "Current working dir: %s\n", cwd);
	}
    else{
        perror("getcwd() error");
    	return 0;
	}

	/*Buscar en el directorio*/
	resultado += Buscar(cwd);		// CAMBIAR PARA QUE NO SEA POR DEFECTO

  /* ---------------------------------Creacion de los Threads--------------------------*/
	for(i=0; i<NUM_THREADS; i++){
	    rc = pthread_create(&arrayH[i].thread, NULL, BuscarThread, (char *)i);
  	}

  	/* Esperar a que todos los hilos terminen*/
	while(completo!=0 ||front!=NULL){
		for(i=0; i<NUM_THREADS; i++){
			if(arrayH[i].libre == 0 && front != NULL){
		  		completo++;
		  		pthread_mutex_lock (&mutexcola);
		  		snprintf(direct,sizeof direct, "%s", (char*)Front());
				Dequeue();	
				pthread_mutex_unlock (&mutexcola);

				//free(arrayH[i].direc_asig);
				arrayH[i].direc_asig = (char*)malloc(strlen(direct)+1);
		  		memset(arrayH[i].direc_asig,'\0',sizeof(arrayH[i].direc_asig));
		  		memcpy(arrayH[i].direc_asig,direct,strlen(direct));
			
				/*Marcar como ocupado al thread*/
				arrayH[i].libre = 1;
			}
		}
	}

	/* Sumar todos los pesos para obtener el total */
	for(i = 0 ; i<NUM_THREADS;i++){
		resultado += arrayH[i].pesoArch;
	}

	/* Imprimir cosas al final */
	printf("Total: %d\n\n", resultado);
}
