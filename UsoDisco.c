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
volatile int auxiliar = 0, resultado = 0, esMaestro=0;

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
    char filename[1024];
    char* huevolteado = "*||*";
    int size, resultadoT;
    char* newdir;
    mode_t modo;

		if ((dir = opendir (cwd)) != NULL) {
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
						size = 4*(st.st_size/st.st_blksize);	// Calculamos el tamanio en bloques
						if(st.st_size%st.st_blksize>0){			// Si la diferencia es mayor a cero	
							size = size + 4;					// Toma otro bloque
							}									
							resultadoT = resultadoT + size;
							//printf ("Regular : %s %d   %d \n ", filename, size, (int)st.st_blksize);
						}

					/* Si el archivo es de tipo directorio, lo agregamos a la lista*/
					else if(S_ISDIR(st.st_mode)){
						newdir = concat(newdir,huevolteado);
						newdir = concat(newdir,filename);
						if(esMaestro==0){
							Enqueue(filename);
						}
						//printf ("Directorio : %s \n Esta es nueva dir: %s \n ", filename, newdir);
						
					}
				}
	  		}
	  		closedir (dir);
			if(esMaestro ==0){
				resultado = resultadoT;
			}
		} 	
		else {
	  		perror ("");
	  		printf("NO PUDE ABRIR: %s\n",cwd);
	  		return 1;
		}

}


/*----------------------------------FUNCION DEL THREAD-------------------------*/
void *BuscarThread(void *threadid){
	long tid;
	int i;
   	tid = (long)threadid;
   	while(1){
		if(libre[tid] = 1 && directorios[tid]!=NULL){
			Buscar(directorios[tid]);
			libre[tid] = 0;
			//Print();
    		usleep(1);
		}
		else if(directorios[tid] == NULL){
			libre[tid] = 0;
			auxiliar++;
		}
	}
}
/*------------------------------------------------------------------------------*/


int main (int argc, char *argv[])
{
	/* Cosas para la busqueda en directorios*/

    /*Cosas para los threads*/
  	pthread_t threads[NUM_THREADS];
  	int rc,i, listos = 0, todosL = 5;
  	long t;
  	char cwd[1024];
 	/*Asignar el tamanio del arreglo de mutex para el bloqueo*/
  	directorios = (char**)malloc(sizeof(char*)*NUM_THREADS);
  	/*Asignar el tamanio del arreglo de hilos libres*/
  	libre =(int*)malloc(sizeof(int)*NUM_THREADS);
 
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

	/*Como solo el maestro puede sumar los pesos y encolar
	directorios directamente,cambiamos la variable esMaestro
	a 1 para evitar que los hilos sumen*/
	esMaestro = 1;

  /*Inicializar cada posicion del arreglo de enteros en 5*/
  for(i=0;i<NUM_THREADS;i++){
    libre[i] = NUM_THREADS;
  }


  /* ---------------------------------Creacion de los Threads--------------------------*/
  	while(auxiliar<NUM_THREADS){
	  	for(t=0; t<NUM_THREADS; t++){
			if (libre[t] == 5){
			    printf("Principal: Creando Thread %ld\n", t);
			    libre[t] = 1;
			    rc = pthread_create(&threads[t], NULL, BuscarThread, (char *)t);
			    if (rc){
			       printf("ERROR; return code from pthread_create() is %d\n", rc);
			       exit(0);
			    }
	  		}
	  	}
	}
	/*---------------------------------------------------------------------------------*/


	/*---------------------------Asignar directorios a los Threads libres-------------*/
	while(front!=NULL){
		/* Recorrer cada todos los threads en busca de alguno disponible para trabajar*/
	  	for(t=0; t<NUM_THREADS; t++){
	  		/* Si el thread esta disponible y hay elementos en la cola*/
		  	if (libre[t] == 0 && front!=NULL){
		  		printf("EL FRONT ES: %s\n",Front());
		  		directorios[t] = (char*)malloc(strlen((char*)Front()));
		  		memset(directorios[t],'\0',sizeof(directorios[t]));
		  		memcpy(directorios[t],(char*)Front(),strlen((char*)Front())+1);
		  		printf("EL DIRECTORIO ES: %s\n",directorios[t]);
		  		Dequeue();
		  		libre[t] = 1;
	    	} 
		}
	}
  	/*----------------------------------------------------------------------------------*/


  	/* ----------------------Esperar a que todos los hilos terminen-------------------*/
  	// RECORDAR QUE ESTE WHILE DEBE IR JUNTO CON EL OTRO PARA EVITAR CASOS BORDE
 	while(todosL>0){
 		todosL = 5;
	   	for(i=0;i<NUM_THREADS;i++){	/*Si todos no son libres entonces queda trabajo*/
	    	if (libre[i] == 0 ){
	    		todosL--;
	    	}
	   	}
 	}
 	/*---------------------------------------------------------------------------------*/



 	if(front==NULL){
 		for(i=0;i<NUM_THREADS;i++){
	    	printf("directorio T %d: %s\n",i,directorios[i]);
	   	}
	   	for(i=0;i<NUM_THREADS;i++){
	    	//printf("libertad T %d: %d\n",i,libre[i]);
	   	}
 		printf("EL Front es NULL!\n Resultado: %d\n",resultado);
 	}
}
