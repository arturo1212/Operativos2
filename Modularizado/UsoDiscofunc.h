#ifndef UsoDiscofunc_h
#define UsoDiscofunc_h

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

/*----------------------------VARIABLES GLOBALES -------------------------*/
extern volatile int completo;  // Entero para saber si todos los hilos terminaron
              // de trabajar.
extern struct Hilos* arrayH;   // Arreglo de estructuras de tipo hilo
extern pthread_mutex_t mutexcola;  // Mutex usado para sincronizacion del uso
              // de la cola entre los hilos
extern int NUM_THREADS;  //Cantidad de Threads a crear
extern FILE *fap;        //apuntador a archivo
extern char *salida;     //Archivo de salida
/* Variables para la cola de Directorios*/
extern struct Node* front;  //Frente de la cola
extern struct Node* rear; //Parte posterior de la cola

/*-------------------------------------------------------------------------*/

// ENCOLAR
void Enqueue(char * x);

// DESENCOLAR
void Dequeue();

// OBTENER EL PRIMERO
char* Front();

// IMPRIMIR LA COLA COMPLETA
void Print();

/*Esta funcion ocncatena dos strings y retorna un apuntador a caracteres con la concatenacion*/
char* concat(char *s1, char *s2);

/* Funcion que, dado un directorio, imprime los archivos regulares y
subdirectorios en su interior. Devuelve el peso de los archivos exa-
minados.                               */
int Buscar(char* cwd);

/* Funcion que usa el hilo para realizar la busqueda    */
void *BuscarThread(void *threadid);

/* Funcion que se encarga de asignar trabajo a num hilos*/
void AsignarDir(int num);

/*Funcion que se encarga de obtener el peso total del 
directorio d usando num hilos.                          */
int obteneResultado(int num, char* d);

/*Funcion que se encarga de crear num hilos*/
void crearHilos(int num);

#endif