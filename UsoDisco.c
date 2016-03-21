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
  	int rc,i, listos = 0,index,cant_threads;
  	long t;
  	char cwd[1024],palabra[1024],*salida,op1,op2;
 	/*Asignar el tamanio del arreglo de mutex para el bloqueo*/
  	directorios = (char**)malloc(sizeof(char*)*NUM_THREADS);
  	/*Asignar el tamanio del arreglo de hilos libres*/
  	libre =(int*)malloc(sizeof(int)*NUM_THREADS);
 	if(argc==1){
		if (getcwd(cwd, sizeof(cwd)) != NULL){
	        fprintf(stdout, "Current working dir: %s\n", cwd);
		}else{
			perror("getcwd() error");
			return 0;
		}
		salida="/dev/stdout";
	}else{
		if(argc==2){
			if((strcmp(argv[1],"-h"))==0){
				printf("UsoDisco  [-h] |  [-n i] [-d directorio] [-o salida ]\n");
				printf("------------------------------\n");
printf("-h 	muestra por pantalla un mensaje de ayuda (sintaxis, descripción de parámetros, etc.) y termina\n");
printf("-n i 	nivel de concurrencia solicitado. Por defecto crea un solo hilo trabajador\n");
printf("-d directorio 	especifica un directorio desde donde calcula el espacio utilizado.");
printf(" Por defecto hace el cálculo desde el directorio actual.\n");
printf("-o salida 	archivo que contendrá la salida con la lista de directorios y el espacio en bloques");
printf(" ocupado por los archivos regulares. El valor por defecto es la salida estándar.\n");
			}else{
				fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
				return 1;
			}
		}else{
			if(argc==3){ //Falta anhadir valores por defecto
				if(argv[1][0]=='-'){
					if (argv[1][1]=='n'){

						i=atoi(argv[2]);
						if (getcwd(cwd, sizeof(cwd)) != NULL){
					        fprintf(stdout, "Current working dir: %s\n", cwd);
						}else{
							perror("getcwd() error");
							return 0;
						}
						salida="/dev/stdout";

					}else if(argv[1][1]=='d'){
						memcpy(cwd,argv[2],strlen(argv[2]));
						cwd[strlen(cwd)] = '\0';
						salida="/dev/stdout";

					}else if(argv[1][1]=='o'){

						salida = (char *)malloc(strlen(argv[2])+1);
						memcpy(salida,argv[2],strlen(argv[2]));
						salida[strlen(salida)] = '\0';
						if (getcwd(cwd, sizeof(cwd)) != NULL){
					        fprintf(stdout, "Current working dir: %s\n", cwd);
						}else{
							perror("getcwd() error");
							return 0;
						}

					}else{
						fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
						return 1;
					}
				}else{
					fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
					return 1;
				}
			}else{
				if(argc==4){
					//Mensaje de error
					fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
					return 1;
				}else{
					if(argc==5){ //Falta anhadir valores por defecto
						if(argv[1][0]=='-'){
							op1=argv[1][1];

							if (argv[1][1]=='n'){
								cant_threads=atoi(argv[2]);
							}else if(argv[1][1]=='d'){
								memcpy(cwd,argv[2],strlen(argv[2]));
								cwd[strlen(cwd)] = '\0';
							}else if(argv[1][1]=='o'){
								salida = (char *)malloc(strlen(argv[2])+1);
								memcpy(salida,argv[2],strlen(argv[2]));
								salida[strlen(salida)] = '\0';
							}else{
								fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
								return 1;
							}
							if(argv[3][0]=='-'){
								op2=argv[3][1];
								if(argv[3][1]=='n'){
									cant_threads=atoi(argv[4]);
								}else if(argv[3][1]=='d'){
									memcpy(cwd,argv[4],strlen(argv[4]));
									cwd[strlen(cwd)] = '\0';
								}else if(argv[3][1]=='o'){
									salida = (char *)malloc(strlen(argv[4])+1);
									memcpy(salida,argv[4],strlen(argv[4]));
									salida[strlen(salida)] = '\0';
								}else{
									fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
									return 1;
								}
								if(op1=='n'){
									if(op2=='d'){
										salida="/dev/stdout";
									}else if(op2=='o'){
										if (getcwd(cwd, sizeof(cwd)) != NULL){
											fprintf(stdout, "Current working dir: %s\n", cwd);
										}else{
											perror("getcwd() error");
											return 0;
										}
									}
								}else if (op1=='d'){
									if(op2=='n'){
										salida="/dev/stdout";
									}
								}else if(op1=='o'){
									if(op2=='n'){
										if (getcwd(cwd, sizeof(cwd)) != NULL){
											fprintf(stdout, "Current working dir: %s\n", cwd);
										}else{
											perror("getcwd() error");
											return 0;
										}
									}
								}
							}else{
								fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
								return 1;
							}
						}else{
							fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
							return 1;
						}
					}else{
						if(argc=6){
							//Mensaje de error
							fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
							return 1;
						}else if(argc>7){
							//Mensaje de error
							fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
							return 1;
						}else{
							for(index = 1;index<7;index+=2){
								if(argv[index][0]=='-'){
									if (argv[index][1]=='n'){
										cant_threads=atoi(argv[index+1]);
									}else if(argv[index][1]=='d'){
										memcpy(cwd,argv[index+1],strlen(argv[index+1]));
										cwd[strlen(cwd)] = '\0';
									}else if(argv[index][1]=='o'){
										salida = (char *)malloc(strlen(argv[index+1])+1);
										memcpy(salida,argv[index+1],strlen(argv[index+1]));
										salida[strlen(salida)] = '\0';
									}else{
										fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
										return 1;
									}
								}else{
									fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
									return 1;
								}
							}
						}	
					}
				}
			}
		}
	}
	/* Revisar los directorios del master para agregarlos a la cola*/
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
