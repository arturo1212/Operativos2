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

/*------------------------------------------------------------------------*/

/*----------------------------VARIABLES GLOBALES -------------------------*/
char** directorios;
int* libre, resultados;
volatile int resultado = 0, esMaestro=0;
pthread_mutex_t mutexsum,mutexsum1;

/* Variables para la cola de Directorios*/
struct Node* front = NULL;
struct Node* rear = NULL;
/*Variables para la cola de cajas*/
struct Node* frente = NULL;
struct Node* atras = NULL;

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
						Enqueue(filename);					
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
   		if (front!= NULL){

   			pthread_mutex_lock (&mutexsum);
   			esMaestro++;
   			snprintf(direct,sizeof direct, "%s",(char*)Front());
   			Dequeue();
			pthread_mutex_unlock (&mutexsum);

   			i = Buscar(direct);
   			pthread_mutex_lock (&mutexsum);  
   			resultado = resultado + i; 	
   			esMaestro--;		
   			pthread_mutex_unlock (&mutexsum);
    		usleep(0);

    	}
	}
}




int main (int argc, char *argv[])
{
	/* Cosas para la busqueda en directorios*/

    /*Cosas para los threads*/
  	pthread_t threads[NUM_THREADS];
  	int rc,i, listos = 0, todosL = 5;
  	long t;
  	char cwd[1024];
 	pthread_mutex_init(&mutexsum, NULL); 	
/* Revisar los directorios del master para agregarlos a la cola*/
	if (getcwd(cwd, sizeof(cwd)) != NULL){
        fprintf(stdout, "Current working dir: %s\n", cwd);
	}
    else{
        perror("getcwd() error");
    	return 0;
	}

	/*Buscar en el directorio*/
	Buscar(cwd);		// CAMBIAR PARA QUE NO SEA POR DEFECTO


  /* ---------------------------------Creacion de los Threads--------------------------*/
	for(t=0; t<NUM_THREADS; t++){
		//printf("Principal: Creando Thread %ld\n", t);
	    rc = pthread_create(&threads[t], NULL, BuscarThread, (char *)t);
  	}
	while(front!=NULL || esMaestro!=0){
		//Print();
	}
	printf("Total: %d\n", resultado);

}
