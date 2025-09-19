/**
	Simulacion de programa cliente servidor
	utilizando memoria compartida
	TP3 Sistemas Operativos
	Integrantes: Federico Manzano
	App: Cliente
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
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


// Macros para validar el nombre de la materia que viene por parametro
#define ES_BLANCO(X) ((X) == ' ')
#define ES_MIN(X) ((X) >= 'a' && (X) <= 'z')
#define ES_MAY(X) ((X) >= 'A' && (X) <= 'Z')
#define ES_NUM(X) ((X) >= '0' && (X) <= '9')

// Condiciones de las peticiones
#define CALCULAR_PROMEDIO_GENERAL    1
#define CALCULAR_PROMEDIO_MATERIA    2
#define INSERTAR_NOTA                3
#define CONEXION                    10
#define DESCONEXION                 -10



typedef struct {
	int socket;
	char registro[1000];
	int op;
    float resultado;
}t_paquete;


t_paquete *memoria;
int fin = 1;

int fd; // File descriptor


// Sincronizar peticiones
sem_t * hay_lectura;
sem_t * escribir; // Es liberado desde el servidor pero se utiliza desde el cliente
sem_t * respuesta;

// Avido de salida
sem_t *aviso_salida;
sem_t *recibir_salida;

// Sincronizar salida con hilos
pthread_t caida_servidor;
pthread_t salida_individual; // Hilo caida del servidor

char * normalizarMateria(char *materia){
    char *desde = materia;
    char *hasta = materia;

    while(*desde){
        while(*desde && ES_BLANCO(*desde))
            desde ++;
        if(ES_MIN(*desde))
            *desde = toupper(*desde);
        *hasta = *desde;
        desde ++;
        hasta ++;
        while(*desde  && !ES_BLANCO(*desde)){
            if(!ES_MIN(*desde))
                *desde = tolower(*desde);
            *hasta = *desde;
            desde ++;
            hasta ++;
        }
        if(*desde && ES_BLANCO(*desde)){
            *hasta = *desde;
            desde++ ;
            hasta ++;
        }
    }
    *hasta = '\0';

    return materia;
}

int validarEvaluacion(char *eva){
	return 	!strcasecmp(eva, "Parcial1") 		||
			!strcasecmp(eva, "Parcial2") 		||
			!strcasecmp(eva, "Recuperatorio");
}


int validarMateria(const char * materia){
	char *m = (char *)materia;

	while(*m){
		if(!ES_MIN(*m) && !ES_BLANCO(*m) && !ES_MAY(*m) && !ES_NUM(*m))
			return 0;
		m ++;
	}
	return 1;
}

int validarDni(const char * dni){
	char *d = (char *)dni;
	while(*d){
		if(!ES_NUM(*d))
			return 0;
		d ++;
	}
	return 1;
}



void imprimir_menu(){
    printf("A-Consultar promedio general\n");
    printf("B-Consultar promedio de una materia\n");
    printf("C-Agregar nota\n");
    printf("S-Salir\n\n");

}

char seleccionar_opcion(){
    char op;
    printf("Ingrese una opcion: ");
    do{

        fflush(stdin);
        scanf("%c", &op);
        op = toupper(op);
    }while(op != 'A' && op != 'B' && op != 'C' && op != 'S');
    return op;
}


char * ingresar_dni(){
	char *dni = (char *)malloc(sizeof(char) * 10);
	printf("Ingrese el numero de dni: ");
	int val = 0;
	do{
		fflush(stdin);
		scanf("%s", dni);
		val = validarDni(dni);
		if(!val)
			printf("\nEl dni es incorrecto ingrese nuevamente: ");
	}while(!val);
	return dni;
}

char * ingresar_evaluacion(){
	char *evaluacion = (char *)malloc(sizeof(char) * 50);
	printf("Ingrese la evaluacion: ");
	int val = 0;
	do{

		fflush(stdin);
		scanf("%*c%[^\n]", evaluacion);
		val = validarEvaluacion(evaluacion);
		if(!val)
			printf("\nEvaluacion erronea ingrese nuevamente: ");
	}while(!val);
	return evaluacion;
}

char * ingresar_materia(){
	char *materia = (char *)malloc(sizeof(char) * 50);
	printf("Ingrese la materia: ");
	int val = 0;
	do{

		fflush(stdin);
		scanf("%*c%[^\n]", materia);
		val = validarMateria(materia);
		if(!val)
			printf("\nMateria erronea ingrese nuevamente: ");
	}while(!val);

	return materia;
}

char * ingresar_nota(){
	char *nota = (char *)malloc(sizeof(char) * 4);
	printf("Ingrese la nota: ");
	int val = 0;
	do{
		fflush(stdin);
		scanf("%s", nota);
		val = validarDni(nota);
		val = atoi(nota) <= 10 && atoi(nota) >= 0;
		if(!val)
			printf("\nLa nota es incorrecta ingrese nuevamente: ");
	}while(!val);
	return nota;
}

void pausa(){
    int c;
    printf("Precione (C / c) para continuar ...\n");
    do {
        c=getchar();
    } while (c != 'c' && c != 'C');
}

void pedir_promedio_general(){
	t_paquete p;
	system("clear");
	p.op = CALCULAR_PROMEDIO_GENERAL;
	char *dni = ingresar_dni();
	strcpy(p.registro, dni);

	free(dni);
	sem_wait(escribir);
	*memoria = p;
	sem_post(hay_lectura);
	sem_wait(respuesta);


	printf("%4.2f\n", memoria->resultado);
	sem_post(escribir);
	pausa();
}

void pedir_promedio_materia(){
	t_paquete p;
	system("clear");
	p.op = CALCULAR_PROMEDIO_MATERIA;
	char *dni = ingresar_dni();
	char *materia = ingresar_materia();

	normalizarMateria(materia);

	strcpy(p.registro, dni);
	strcat(p.registro, ",");
	strcat(p.registro, materia);

	free(dni);
	free(materia);

	sem_wait(escribir);
	*memoria = p;
	sem_post(hay_lectura);
	sem_wait(respuesta);
	printf("El promedio es %4.2f\n", memoria->resultado);
	sem_post(escribir);
	pausa();
}


/**
	Se inserta un registro dentro de la base de datos

*/
void agregar_nota(char *materia){
	system("clear");
	t_paquete p;
	char *dni = ingresar_dni();
	char *eva = ingresar_evaluacion();
	char *nota = ingresar_nota();

	normalizarMateria(materia);
	normalizarMateria(eva);


	strcpy(p.registro, dni);
	strcat(p.registro, ",");
	strcat(p.registro, materia);
	strcat(p.registro, ",");
	strcat(p.registro, eva);
	strcat(p.registro, ",");
	strcat(p.registro, nota);

	free(nota);
	free(eva);

	p.op = INSERTAR_NOTA;
	sem_wait(escribir);
	*memoria = p;
	sem_post(hay_lectura);
	sem_wait(respuesta);

	/** Si el resultado es 0 la nota no pudo ingresarse
	Caso contrario la nota fue insertada en la base de datos*/
	if(memoria->resultado == 1)
		printf("La materia ha sido ingresada con exito\n\n");
	else
		printf("La materia no pudo ingresarse\n\n");
	sem_post(escribir);
	pausa();
}


/**
	Establecer la conexion con el servidor

*/
void conexion(){
	t_paquete p;
	p.op = CONEXION; // Comando CONEXION = 10


	p.socket = getpid();

	// Este semafor asegura que un solo proceso escriba en la memoria compartida
	sem_wait(escribir);
	*memoria = p;

	// Se le informa al servidor que hay algo en la memoria compartida
	sem_post(hay_lectura);

	// Se le da permiso a los demas procesos de poder escribir
	sem_post(escribir);
}


/**
	Funcion que se mantiene bloqueada por un semaforo
	para dar respuesta a la caida del servidor con
	clientes conectados
*/
void * servidor(void *args){
	// El servidor cuando se cae habilita el semaforo "aviso_salida = 1"
	sem_wait(aviso_salida);

	// Se cierra el File Descriptor
	close(fd);
    	system("clear");
	puts("Sevidor caido");

	// Se le informa al servidor este cliente cerro correctamente
	// Habilitando el semaforo "recicibir-salida"
	sem_post(recibir_salida);
	exit(1);
}

void aviso(){
	t_paquete p;
	p.op = DESCONEXION;
	sem_wait(escribir);
	*memoria = p;
	close(fd);
	system("clear");
	printf("%s\n", "Desconectado");
	sem_post(hay_lectura);
	sem_post(escribir);
	exit(1);
}


/**
	Se mantiene a la espera hasta que
	el usuario quiera terminar el proceso
*/
void * salida(void *args){
    while(1){
        signal(SIGINT, aviso);
        signal(SIGTERM, aviso);
        sleep(1);
    }
}


int main(char *args, char *argv[]){

	// Si no ingresa ningun parametro error de ejecucion
	if(argv[1] == NULL){
		printf("La cantidad de parametros es erronea consulte la ayuda");
		return -10;
	}

	// Si es la ayuda mostrar la ayuda y salir
	if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "-?") ){
	      puts(" *** Ejecucion ***");
	      puts("[Ruta del programa]servidor");
	      puts("Compilacion");
	      puts("[Ruta del Programa]make cliente");
	      puts("[Ruta del Programa]make all // Compila el cliente y el servidor");
	      return 10;
	}


	// Configurar la materia de la cual es responsable el profesor
	char materia[100];
	strcpy(materia, argv[1]);


	// No se permiten nombres de materia con caracteres especiales
	if(!validarMateria(materia)){
		puts("No se permiten nombres de materias con carateres especiales");
		return -10;
	}

	// Formato al nombre de la materia
	normalizarMateria(materia);

	// Sinbronizar peticiones
	hay_lectura = sem_open("hay_lectura",O_CREAT, 0600, 0);
	escribir = sem_open("escribir",O_CREAT, 0600, 1);
	respuesta = sem_open("respuesta",O_CREAT, 0600, 0);

	// Sincronizar salida
	aviso_salida = sem_open("aviso_salida",O_CREAT, 0600, 0);
    recibir_salida = sem_open("recibir_salida",O_CREAT, 0600, 0);


    // Apertura de la memoria compartida
    fd = shm_open("compartida", O_RDWR | O_CREAT, 0666);
	memoria = (t_paquete *)mmap(NULL, sizeof(t_paquete),
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


	// Gestionan la salida del sistema
	pthread_create(&caida_servidor, NULL, servidor, NULL);
	pthread_create(&salida_individual, NULL, salida, NULL);

	// Sincronizar la conexion
	conexion();

	/* Abre el semaforo para establecer la
	conexion con el servidor */
	sem_wait(respuesta);

	while(fin){
		system("clear");
		printf("Menu de opciones (Profesor %s)\n\n", materia);
		imprimir_menu();
		char op = seleccionar_opcion();
		switch(op){
			case 'A':
				/**
					Envia al servidor una peticion de pedido de un promedio
					general de un alumno, queda bloquedo el cliente hasta que
					el servidor devuelva el promedio solicitado
				*/
				pedir_promedio_general();
			break;


			case 'B':

				/**
					Envia al servidor una peticion de pedido de un promedio
					de una materia de un alumno, queda bloquedo el cliente hasta que
					el servidor devuelva el promedio solicitado
				*/
				pedir_promedio_materia();
			break;


			case 'C':
				/**
					Envia al servidor una peticion de pedido de insercion de una
					nota nueva en la base de datos
				*/
				agregar_nota(materia);
			break;
			case 'S':
				/** Cierre libera el filedescriptos utilizado por el proceso*/
				aviso();
			break;
		}
	}
	pthread_join(caida_servidor, NULL);
}
