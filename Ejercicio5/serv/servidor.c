/**
    Simulacion de programa cliente servidor
    utilizando memoria compartida
    TP3 Sistemas Operativos
    Integrantes: Federico Manzano
    App: Servidor
*/


#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

// Condiciones de las peticiones
#define CALCULAR_PROMEDIO_GENERAL    1
#define CALCULAR_PROMEDIO_MATERIA    2
#define INSERTAR_NOTA                3
#define CONEXION                    10
#define DESCONEXION                 -10

typedef struct {
    char dni[12];
    char materia[100];
    char evaluacion[100];
    int nota;
}t_registro;


typedef struct {
	int socket;
	char registro[1000];
	int op;
    float resultado;
}t_paquete;


int fd; // File descriptor

t_paquete *memoria; // Variable de memoria compartida

int fin = 1; // Condicion de fin
int conectados = 0; //cantidad de usuarios conectados

// Sincronizar peticiones
sem_t *hay_lectura;
sem_t *respuesta;
sem_t *escribir;

// Sincronizar salida
sem_t *aviso_salida;
sem_t *recibir_salida;

pthread_t salida; // Hilo caida del servidor



void txt_a_estructura(char *linea, t_registro *r){
    char *aux = strchr(linea, '\n');
    if(!aux) return;
    *aux = '\0';
    aux = strrchr(linea, ',');
    if(!aux) return;
    *aux = '\0';
    sscanf(aux + 1, "%d", &r->nota);
    aux = strrchr(linea, ',');
    if(!aux) return;
    *aux = '\0';
    strcpy(r->evaluacion, aux + 1);
    aux = strrchr(linea, ',');
    if(!aux) return;
    *aux = '\0';
    strcpy(r->materia, aux + 1);
    strcpy(r->dni, linea);
}


void separar_registro(t_paquete *p, t_registro *r){
	char *aux = strchr(p->registro, ',');
	*aux = '\0';
	strcpy(r->dni, p->registro);
	strcpy(r->materia, aux + 1);
}

void ingresarRegistro(t_paquete *p){
    FILE *alumnos = fopen("alumnos.txt", "r+t");
    t_registro r;
    fseek(alumnos, 0L, SEEK_END);
    fprintf(alumnos, "%s\n", p->registro);
    fclose(alumnos);
 }

void sacar_promedio_materia(t_paquete *p){
    FILE *alumnos = fopen("alumnos.txt", "rt");
    char linea[500];
    t_registro r;
    t_registro aux;
    int suma = 0;
    int cantidad = 0;
    rewind(alumnos);
    separar_registro(p, &aux);


    fgets(linea, sizeof(linea), alumnos);
    while(!feof(alumnos)){
        txt_a_estructura(linea, &r);

        if(strcmp(r.dni, aux.dni) == 0 && strcasecmp(r.materia, aux.materia) == 0){
            suma += r.nota;
            cantidad ++;
        }
        fgets(linea, sizeof(linea), alumnos);
    }
    if(cantidad > 0)
        p->resultado =  (float)suma / cantidad;
    else
        p->resultado = 0;
    strcpy(p->registro, aux.materia);
    fclose(alumnos);
 }

 void sacar_promedio_general(t_paquete *p){
    FILE *alumnos = fopen("alumnos.txt", "rt");
    char linea[500];
    t_registro r;
    int suma = 0;
    int cantidad = 0;
    rewind(alumnos);

    fgets(linea, sizeof(linea), alumnos);
    while(!feof(alumnos)){
        txt_a_estructura(linea, &r);
        if(strcmp(r.dni, p->registro)){
            suma += r.nota;
            cantidad ++;
        }
        fgets(linea, sizeof(linea), alumnos);
    }
    if(cantidad > 0)
        p->resultado =  (float)suma / cantidad;
    else
        p->resultado = 0;
    fclose(alumnos);
}

void aviso(){
    for (int i = 0; i < conectados; ++i)
    {
        sem_post(aviso_salida);
        sem_wait(recibir_salida);
    }

    sem_close(hay_lectura);
    sem_unlink("hay_lectura");
    sem_close(respuesta);
    sem_unlink("respuesta");
    sem_close(aviso_salida);
    sem_unlink("aviso_salida");
    sem_close(recibir_salida);
    sem_unlink("recibir_salida");
    sem_close(escribir);
    sem_unlink("escribir");

    close(fd);
    munmap(memoria, sizeof(t_paquete));
    shm_unlink("compartida");

    exit(1);
}

void * caida_servidor(void *args){
    while(1){
        signal(SIGINT, aviso);
        signal(SIGTERM, aviso);
        sleep(1);
    }
}

int main(char *args, char *argv[]){
 	// Coordina las lecturas provenientes de los clientes

    if(argv[1] != NULL && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "-?"))){
        puts(" *** Ejecucion ***");
        puts("[Ruta del programa]servidor");
        puts("Compilacion");
        puts("[Ruta del Programa]make servidor");
        puts("[Ruta del Programa]make all // Compila el cliente y el servidor");
        return 10;
    }



  	hay_lectura = sem_open("hay_lectura",O_CREAT, 0600, 0);
  	respuesta = sem_open("respuesta",O_CREAT, 0600, 0);
    escribir = sem_open("escribir",O_CREAT, 0600, 1);


      // Sincronizar salida
    aviso_salida = sem_open("aviso_salida",O_CREAT, 0600, 0);
    recibir_salida = sem_open("recibir_salida",O_CREAT, 0600, 0);


  	// Apertura de la memoria compartida
  	 fd = shm_open("compartida", O_RDWR | O_CREAT, 0666);

  	 if ( fd < 0 ) {
          // Error en la creacion de la memoria compartida
          puts("Error al crear la memoria compartida");
          return -10;
      }

      pthread_create(&salida, NULL, caida_servidor, NULL);

      // Truncar la memoria t_paquete
  	ftruncate(fd, sizeof(t_paquete));
  	memoria = (t_paquete *)
  		mmap(NULL, sizeof(t_paquete), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  	// Comprobar si se ha podido mapear el objeto en la memoria
      if ( memoria == MAP_FAILED ) {
      	puts("Error al mapear el objeto en la memoria comparida");
      	return -10;
      }
      system("clear");
      puts("esperando conexiones ...");
  	  while(fin){
  		sem_wait(hay_lectura);
  		switch(memoria->op){
  			case CALCULAR_PROMEDIO_GENERAL:
          puts("calculo del promedio general ...");
  				sacar_promedio_general(memoria);
  				sem_post(respuesta);
  			break;
  			case CALCULAR_PROMEDIO_MATERIA:
          puts("calculo del promedio por materia ...");
  				sacar_promedio_materia(memoria);
  				sem_post(respuesta);
  			break;
        case INSERTAR_NOTA:
          puts("Nota ingresada ...");
          ingresarRegistro(memoria);
          memoria->resultado = 1;
          sem_post(respuesta);
        break;
  			case CONEXION:
          puts("Cliente conectado ...");
          conectados ++;
          sem_post(respuesta);
  			break;
        case DESCONEXION:
          puts("Cliente desconectado ...");
          conectados --;
        break;
  		}
  	}
  	return 0;
}
