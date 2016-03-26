#include "UsoDiscofunc.h"
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

int main (int argc, char *argv[]){
  /*----------------- Declaracion de Variables-------------------*/ 
  
  int resultado = 0;  /*Almacena el resultado total.             */
  char *cwd;   /*Almacena la direccion del directorio base*/
  char op1,op2;     // Variables auxiliares para la lectura de parametros
  int index;
if(argc==1){
    cwd=".";
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
            cwd=".";
            salida="/dev/stdout";

          }else if(argv[1][1]=='d'){
            cwd = (char *)malloc(strlen(argv[2])+1);
            memcpy(cwd,argv[2],strlen(argv[2])+1);
            cwd[strlen(cwd)] = '\0';
            salida="/dev/stdout";

          }else if(argv[1][1]=='o'){

            salida = (char *)malloc(strlen(argv[2])+1);
            memcpy(salida,argv[2],strlen(argv[2]));
            salida[strlen(salida)] = '\0';
            cwd=".";

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
                cwd = (char *)malloc(strlen(argv[2])+1);
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
                    cwd=".";
                  }
                }else if (op1=='d'){
                  if(op2=='n'){
                    salida="/dev/stdout";
                  }
                }else if(op1=='o'){
                  if(op2=='n'){
                    cwd=".";
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
                    cwd = (char *)malloc(strlen(argv[index+1])+1);
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
