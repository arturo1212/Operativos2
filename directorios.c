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

int main() {
	DIR *dir;
	struct stat st;
	struct dirent *ent;
    char cwd[1024], filename[1024];
    int size, resultado = 0;
    mode_t modo;

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        fprintf(stdout, "Current working dir: %s\n", cwd);
    else{
        perror("getcwd() error");
    	return 0;
	}
	if ((dir = opendir (cwd)) != NULL) {
		resultado = 0;
  		while ((ent = readdir (dir)) != NULL) {
  			
    			snprintf(filename, sizeof filename, "%s/%s", cwd, ent->d_name);
				stat(filename, &st);
				size = st.st_size;
				if(S_ISREG(st.st_mode)){
					printf ("Regular : %s %d \n ", filename, size);
					resultado = resultado + size;
				}
				else if(S_ISDIR(st.st_mode)){
					printf ("Directorio : %s %d \n ", filename, size);
				}
				else{
					printf ("Raro : %s %d \n ", filename, size);
				}	
  		}
  		printf("\nEl tamanio total es: %d bytes \n\n",resultado);
  	closedir (dir);
	} 	
	else {
  		perror ("");
  		return 1;
	}
}
