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

/*------------------------------ESTRUCTURAS--------------------------------*/

/*Nodos para las Colas*/
struct Node {
  char* data;     // Informacion
  struct Node* next;  // Apuntador al siguiente
};

struct Hilos {  
  pthread_t thread; // Para crear el thread
  char* direc_asig;   // Directorio de trabajo actual
  int pesoArch;   // Peso que ha contado el Hilo
  int libre;      // Indica si el hilo esta libre o no.
};

/*------------------------------------------------------------------------*/

/*----------------------------VARIABLES GLOBALES -------------------------*/
volatile int completo=0;  // Entero para saber si todos los hilos terminaron
              // de trabajar.
struct Hilos* arrayH;   // Arreglo de estructuras de tipo hilo
pthread_mutex_t mutexcola;  // Mutex usado para sincronizacion del uso
              // de la cola entre los hilos
int NUM_THREADS=1;  //Cantidad de Threads a crear
FILE *fap;        //apuntador a archivo
char *salida;     //Archivo de salida
/* Variables para la cola de Directorios*/
struct Node* front = NULL;  //Frente de la cola
struct Node* rear = NULL; //Parte posterior de la cola

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

/*Esta funcion ocncatena dos strings y retorna un apuntador a caracteres con la concatenacion*/
char* concat(char *s1, char *s2){
  /*VARIABLES:
    result  : apuntador a caracteres a retornar con la concatenaicon
    s1    : primer string a concatenar
    s2    : segundo string a concatenar
  */
  char *result;
    result = (char*)malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
/* Funcion que, dado un directorio, imprime los archivos regulares y
subdirectorios en su interior. Devuelve el peso de los archivos exa-
minados.                               */
int Buscar(char* cwd){
  /*  VARIABLES:
    DIR *dir : buscar en directorios
    struct stat st : obtener informacion de los archivos
    struct dirent *ent : buscar en directorios
    char filename[] : Almacenar nombre del archivo
    int resultadoT : Almacenar tamanio de los directorios
  */
  DIR *dir;
  struct stat st;
  struct dirent *ent;
    char filename[4096];
    int resultadoT = 0;

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
            fprintf (fap,"%d %s \n",(int)st.st_blocks, filename);
            resultadoT = resultadoT + (int)st.st_blocks;
          }

          /* Si el archivo es de tipo directorio, lo agregamos a la lista
          y sumamos su peso                                              */
          else if(S_ISDIR(st.st_mode)){
            pthread_mutex_lock (&mutexcola);
            Enqueue(filename);  
            pthread_mutex_unlock (&mutexcola);  

            fprintf (fap,"%d %s \n",(int)st.st_blocks, filename); 
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

/* Funcion que usa el hilo para realizar la busqueda    */
void *BuscarThread(void *threadid){
  /*  VARIABLES:
    long tid: id del thread
    int i : auxiliar entero
    char direct[] : Almacena directorio a explorar
  */
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

/* Funcion que se encarga de asignar trabajo a num hilos*/
void AsignarDir(int num){
  /*  VARIABLES:
    int i : Iterador
    char direct[] : Almacena direccion de directorio
  */
  int i;
  char direct[4096];
  
  /*  Mientras la busqueda no haya finalizado, entonces revisa cuales
    hilos estan libres para asignarles un directorio        */

  while(completo!=0 ||front!=NULL){
    for(i=0; i<num; i++){
      if(arrayH[i].libre == 0 && front != NULL){
        /* Evitar que el hilo maestro termine antes y
        que otros hilos hagan operaciones sobre la 
        Cola.                                       */

        completo++;
          pthread_mutex_lock (&mutexcola);
          snprintf(direct,sizeof direct, "%s", (char*)Front());
        Dequeue();  
        pthread_mutex_unlock (&mutexcola);

        /*  Asignar un nuevo directorio al hilo*/
        free(arrayH[i].direc_asig);
        arrayH[i].direc_asig = (char*)malloc(strlen(direct)+1);
          memset(arrayH[i].direc_asig,'\0',sizeof(arrayH[i].direc_asig));
          memcpy(arrayH[i].direc_asig,direct,strlen(direct)+1);
      
        /*Marcar como ocupado al thread*/
        arrayH[i].libre = 1;
      }
    }
  }
}

/*Funcion que se encarga de obtener el peso total del 
directorio d usando num hilos.                          */
int obteneResultado(int num, char* d){
  /*  VARIABLES:
    int i : Iterador
    int total : Almacenar peso total
    struct stat s : Para extraer el tamanio de un archivo
  */
  int i,total = 0;  
  struct stat s;
  stat(d,&s);

  total += (int)s.st_blocks;    // Agregamos el tamanio del directorio base
  for(i = 0 ; i<num;i++){     // Sumamos el almacenado de cada hilo.
    total += arrayH[i].pesoArch;
  }
  return total;         // Devolvemos el peso total
}

/*Funcion que se encarga de crear num hilos*/
void crearHilos(int num){
  /*  VARIABLES:
    int i : Iterador
    int rc : Para errores
  */
  int i, rc;
  for(i=0; i<NUM_THREADS; i++){
      rc = pthread_create(&arrayH[i].thread, NULL, BuscarThread, (char *)i);
    }
}

int main (int argc, char *argv[]){
  /*----------------- Declaracion de Variables-------------------*/ 
  
  int resultado = 0;  /*Almacena el resultado total.             */
  char cwd[1024];   /*Almacena la direccion del directorio base*/
  char op1,op2;     // Variables auxiliares para la lectura de parametros
  int index;
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
printf("-h  muestra por pantalla un mensaje de ayuda (sintaxis, descripción de parámetros, etc.) y termina\n");
printf("-n i  nivel de concurrencia solicitado. Por defecto crea un solo hilo trabajador\n");
printf("-d directorio   especifica un directorio desde donde calcula el espacio utilizado.");
printf(" Por defecto hace el cálculo desde el directorio actual.\n");
printf("-o salida   archivo que contendrá la salida con la lista de directorios y el espacio en bloques");
printf(" ocupado por los archivos regulares. El valor por defecto es la salida estándar.\n");
return 0;
      }else{
        fprintf(stderr, "Uso esperado : %s  [-h] |  [-n i] [-d directorio] [-o salida ]\n", argv[0]);
        return 1;
      }
    }else{
      if(argc==3){ //Falta anhadir valores por defecto
        if(argv[1][0]=='-'){
          if (argv[1][1]=='n'){

            NUM_THREADS=atoi(argv[2]);
            if (getcwd(cwd, sizeof(cwd)) != NULL){
                  fprintf(stdout, "Current working dir: %s\n", cwd);
            }else{
              perror("getcwd() error");
              return 1;
            }
            salida="/dev/stdout";

          }else if(argv[1][1]=='d'){
            memcpy(cwd,argv[2],strlen(argv[2])+1);
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
              return 1;
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
                NUM_THREADS=atoi(argv[2]);
              }else if(argv[1][1]=='d'){
                memcpy(cwd,argv[2],strlen(argv[2])+1);
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
                  NUM_THREADS=atoi(argv[4]);
                }else if(argv[3][1]=='d'){
                  memcpy(cwd,argv[4],strlen(argv[4])+1);
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
                      return 1;
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
            if(argc==6){
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
                    NUM_THREADS=atoi(argv[index+1]);
                  }else if(argv[index][1]=='d'){
                    memcpy(cwd,argv[index+1],strlen(argv[index+1])+1);
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
  fap = fopen(salida,"a+");
    /*---Inicializacion el mutex y asignacion de espacio a arrayH--*/
    pthread_mutex_init(&mutexcola, NULL);
  arrayH = malloc(NUM_THREADS*sizeof(struct Hilos));

    /*-----------------------Realizar Busqueda---------------------*/
  resultado += Buscar(cwd); /* Sumar el peso del contenido del directorio base*/
  crearHilos(NUM_THREADS);  /* Creacion de los Hilos*/
    AsignarDir(NUM_THREADS);  /* Asignacion de directorios a los hilos*/

    /*----------------------Terminar Busqueda----------------------*/
    resultado += obteneResultado(NUM_THREADS,cwd);  /* Sumar los pesos restantes*/
  fprintf(fap,"%d\n", resultado);            /* Imprimir resultado*/
}
