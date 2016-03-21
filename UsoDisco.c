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

char** directorios;
int* libre;
volatile int auxiliar = 0, resultado = 0, yavio=0, todosL=0;

/*Queue - Linked List implementation*/
struct Node {
  char* data;
  struct Node* next;
};
// Two glboal variables to store address of front and rear nodes. 
struct Node* front = NULL;
struct Node* rear = NULL;

// To Enqueue an integer
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

// To Dequeue an integer.
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

char* Front() {
  if(front == NULL) {
    printf("Queue is empty\n");
    return;
  }
  return front->data;
}

void Print() {
  struct Node* temp = front;
  while(temp != NULL) {
    printf("%s ",temp->data);
    temp = temp->next;
  }
  printf("\n");
}


char* concat(char *s1, char *s2){
	/*	Esta funcion ocncatena dos strings y retorna n apuntador a caracteres con la concatenacion
		
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

int Buscar(char* cwd){
	DIR *dir;
	struct stat st;
	struct dirent *ent;
    char filename[1024];
    int size;
    mode_t modo;
		if ((dir = opendir (cwd)) != NULL) {
	  		while ((ent = readdir (dir)) != NULL) {
	  				if(ent->d_name[0]!='.' && ent->d_name[1]!='.'){ 			
		    			snprintf(filename, sizeof filename, "%s/%s", cwd, ent->d_name);
						stat(filename, &st);
						size = st.st_size;
						if(S_ISREG(st.st_mode)){
							resultado = resultado + size;
						}
						else if(S_ISDIR(st.st_mode)){
							if(strcmp(filename,cwd)!=0 && yavio==0){
								Enqueue(filename);
								printf("ENCOLAANDOOOOOOOO!!!!!!!!!!! %s\n",filename);
							}
						}
					}
	  		}
	  	closedir (dir);
		} 	
		else {
	  		perror ("");
	  		return 1;
		}
}

void *BuscarThread(void *threadid){
	long tid;
	int i;
   	tid = (long)threadid;
   	while(1){
		if(libre[tid] = 1 && directorios[tid]!=NULL){
			Buscar(directorios[tid]);
			libre[tid] = 0;
			Print();
			todosL--;
    		sleep(2);
		}
		else if(directorios[tid] == NULL){
			libre[tid] = 0;
			auxiliar++;
		}
	}
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
	/* Cosas para la busqueda en directorios*/

    /*Cosas para los threads*/
  	pthread_t threads[NUM_THREADS];
  	int rc,i, listos = 0;
  	long t;
  	char cwd[1024],palabra[1024];
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
	Buscar(cwd);
	printf("La Cola es: \n\n");
	Print();
	printf("La Cola es: \n\n");
  /*Inicializar cada posicion del arreglo de enteros en 5*/
  for(i=0;i<NUM_THREADS;i++){
    libre[i] = NUM_THREADS;
  }

  yavio = 1;
  /* Creacion de los Threads*/
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
	while(front!=NULL || todosL>0){
	  	for(t=0; t<NUM_THREADS; t++){
		  	if (libre[t] == 0 && front!=NULL){
		  		todosL++;
		  		memcpy(palabra,Front(),strlen(Front())+1);
		  		directorios[t] = (char*)malloc(strlen(palabra)+1);
		  		memcpy(directorios[t],palabra,strlen(palabra));
		  		Dequeue();
		  		libre[t] = 1;
	    	} 
		}
	}
	/*
	Print();
	 for(i=0;i<NUM_THREADS;i++){
    	printf("STRING: %s\n",directorios[i]);    
  }
  */
 	if(front==NULL){
 		for(i=0;i<NUM_THREADS;i++){
	    	printf("directorio T %d: %s\n",i,directorios[i]);
	   	}
	   	for(i=0;i<NUM_THREADS;i++){
	    	printf("libertad T %d: %d\n",i,libre[i]);
	   	}
 		printf("EL Front es NULL!\n Resultado: %d\n",resultado);
 	}
}
