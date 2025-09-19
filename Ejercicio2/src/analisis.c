#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "../lib/lista.c"
#include "../lib/listaDueno.c"
#include <pthread.h>

pthread_mutex_t listaMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fin = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ex = PTHREAD_MUTEX_INITIALIZER;


t_lista listado;
char extensionCad[5];
char directorio[] = "./directorio";
pthread_t he;

int condFin = 0;

void titulo(){
  printf("%-10s%-10s%-10s%-10s%-10s%-11s%-10s\n",
    "Extension", "Menor(B)", "Mayor(B)", "Cantidad", "Dueno", "D.Cantidad", "% Por");
}

void imprimir(void *e){
    t_lista *l = (t_lista *)e;
    printf("\n");
    int cantidad = cantidad_total(l);
    while(*l){
    printf("%-10s%-10ld%-10ld%-10d%-10d%-11d%4.2f %-2c \n",
        (*l)->info.ext,
        (*l)->info.tam_menor,
        (*l)->info.tam_mayor,
        (*l)->info.cantidad,
        (*l)->info.dueno.nombre,
        (*l)->info.dueno.cantidad,
        ((float)((*l)->info.cantidad * 100)) / cantidad,
        '%'
      );
      l = &(*l)->sig;
    }
}


int comp_clave(const void *e1, const void *e2){
  t_info *ele1 = (t_info *)e1;
  t_info *ele2 = (t_info *)e2;
  return strcmp(ele1->ext, ele2->ext);
}


int comp_tam_mayor(const void *e1, const void *e2){
  t_info *ele1 = (t_info *)e1;
  t_info *ele2 = (t_info *)e2;
  return ele1->tam_mayor - ele2->tam_mayor;
}


int comp_dueno(const void *e1, const void *e2){
  t_info_dueno *ele1 = (t_info_dueno *)e1;
  t_info_dueno *ele2 = (t_info_dueno *)e2;
  return ele1->nombre - ele2->nombre;
}

void act(void *e1, const void *e2){
  t_info *ele1 = (t_info *)e1;
  t_info *ele2 = (t_info *)e2;

  if(ele1->tam_mayor == 0 && ele1->tam_menor == 0){
    ele1->tam_menor = ele2->tam;
    ele1->tam_mayor = ele2->tam;
  }else if(ele1->tam_mayor < ele2->tam){
    ele1->tam_mayor = ele2->tam;
  }else if(ele1->tam_menor > ele2->tam){
    ele1->tam_menor = ele2->tam;
  }
  ele1->cantidad ++;
}

void act_dueno(void *e1, const void *e2){
  t_info *ele1 = (t_info *)e1;
  t_info *ele2 = (t_info *)e2;
  ele1->dueno.nombre = ele2->dueno.nombre;
  ele1->dueno.cantidad = ele2->dueno.cantidad;
}

char *obtener_directorio(struct dirent *ent, char *ruta){
	int tmp=strlen(ruta);
  	char * nombrecompleto=malloc(tmp+strlen(ent->d_name)+2);
  	if (ruta[tmp-1]=='/')
   		sprintf(nombrecompleto,"%s%s", ruta, ent->d_name);
  	else
    	sprintf(nombrecompleto,"%s/%s", ruta, ent->d_name);
    return nombrecompleto;
}

char * extension(char *ruta){
  char *ext = strrchr(ruta, '.');
  return ext;
}

void calcular_long(char *archivo, t_extension *e){
  FILE *arch = fopen(archivo, "rt");
  fseek(arch, 0L, SEEK_END);
  e->tam = ftell(arch);
  fclose(arch);
}


void * manejador(void *args){
  char *ext = (char *)malloc(strlen(extensionCad) + 1);
  strcpy(ext, extensionCad);
  t_lista_dueno listadoDueno;
  crear_lista_dueno(&listadoDueno);
  pthread_mutex_unlock(&ex);


  t_extension e;
  struct dirent *ent;
  struct stat info;
  t_dueno d;

  DIR *dir = opendir (directorio);
  while((ent = readdir (dir)) != NULL){
    char *exte = extension(ent->d_name);
    if(exte){
      if(strcmp(ext, exte) == 0){
        pthread_mutex_lock(&listaMutex);
        strcpy(e.ext, exte);
        calcular_long(obtener_directorio(ent, directorio), &e);
        insertar(&listado, &e, comp_clave, act);
        stat(obtener_directorio(ent, directorio), &info);
        d.nombre = info.st_uid;
        insertar_dueno(&listadoDueno, &d, comp_dueno);
        pthread_mutex_unlock(&listaMutex);
      }
    }
  }
  strcpy(e.ext, ext);
  sacar_mayor(&listadoDueno, &d);
  e.dueno = d;

  pthread_mutex_lock(&listaMutex);
  insertar(&listado, &e, comp_clave, act_dueno);
  pthread_mutex_unlock(&listaMutex);

  vaciar_lista_dueno(&listadoDueno);
  free(ext);

  pthread_mutex_lock(&fin);
  condFin --;
  pthread_mutex_unlock(&fin);
}


int main(char *args, char * argv[]){

  if(argv[1] == NULL){
    printf("Error en los parametros consulte la ayuda\n");
    return -10;
  }

  if(argv[2] != NULL){
    printf("Error en los parametros consulte la ayuda\n");
    return -10;
  }

  if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "-?")){
    printf(" *** Ejecucion ***\n");
    printf("[Ruta del programa]analisis [directorio a analizar]\n");
    printf("Compilacion\n");
    printf("[Ruta del programa]make ./analisis\n");
    return 8;
  }

  crear_lista(&listado);
  DIR *dir;
  t_extension e;
  struct dirent *ent;

  strcpy(directorio, argv[1]);
  dir = opendir (argv[1]);

  if (dir == NULL){
    printf("No puede abrir el directorio\n\n");
    return -3;
  }

  while ((ent = readdir (dir)) != NULL){
    if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 ){
      char *ext = extension(ent->d_name);
      if(ext && *ext){
        strcpy(e.ext, ext);
        pthread_mutex_lock(&listaMutex);
        if(!buscar_extension(&listado, &e, comp_clave)){
          pthread_mutex_lock(&ex);
          strcpy(extensionCad, ext);
          strcpy(e.ext, ext);
          insertar(&listado, &e, comp_clave, act);
          pthread_mutex_lock(&fin);
          condFin ++;
          pthread_mutex_unlock(&fin);
          pthread_create(&he, NULL, manejador, NULL);
        }
        pthread_mutex_unlock(&listaMutex);
      }
    }
  }


  while(condFin) sleep(1);
  closedir (dir);
  ordenar(&listado, comp_tam_mayor);
  mostrar(&listado, titulo, imprimir);
  vaciar_lista(&listado);
  pthread_mutex_destroy(&listaMutex);
  pthread_mutex_destroy(&fin);
  pthread_mutex_destroy(&ex);
  return 0;
}
