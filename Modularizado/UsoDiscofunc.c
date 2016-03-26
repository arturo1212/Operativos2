#include "UsoDiscofunc.h"

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
    int resultadoT = 0, auxSuma = 0;

    if ((dir = opendir (cwd)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
          /*Verificamos que no estemos tomando el directorio anterior*/
          if((strcmp(ent->d_name,".")!=0) && (strcmp(ent->d_name,"..")!=0)){
            /* En filename almacenamos la ruta completa del archivo a examinar*/          
            snprintf(filename,sizeof filename, "%s/%s", cwd, ent->d_name);

            /*Usamos la estructura stat para obtener atributos
            importantes, como el peso.*/
          stat(filename, &st);  

          /* Si el archivo es regular, sumamos su peso*/                    
          if(S_ISREG(st.st_mode)){                          
            
            resultadoT = resultadoT + (int)st.st_blocks;
            auxSuma = auxSuma + (int)st.st_blocks;
          }

          /* Si el archivo es de tipo directorio, lo agregamos a la lista
          y sumamos su peso                                              */
          else if(S_ISDIR(st.st_mode)){
            pthread_mutex_lock (&mutexcola);
            Enqueue(filename);  
            pthread_mutex_unlock (&mutexcola);  
            resultadoT = resultadoT + (int)st.st_blocks;
            auxSuma = auxSuma + (int)st.st_blocks;
          }
          }
        }
        fprintf (fap,"%d %s \n",(int)auxSuma, cwd);
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
