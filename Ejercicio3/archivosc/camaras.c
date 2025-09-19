#include "lib/lista.c"
#include <semaphore.h>

int fin 	= 1;
int fuera 	= 0;

// Inicializar los semaforos mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t salir = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t terminar = PTHREAD_MUTEX_INITIALIZER;


t_lista_autos lista;

pthread_t entrada ;
pthread_t salida ;
pthread_t sen ;

int fd;
sem_t *escritura;
sem_t *cerrar_est;
sem_t *cerrar_camaras;
sem_t *abrir;

int valida = 1;

char letras[] = {
					'0','A','B','C','D','E','F',
					'G','H','I','J','K','L',
					'M','N','O','P','Q','R',
					'S','T','U','V','W','X','Y','Z'
};


// Generacion de patentes aleatoria
char * damePatente(){
	int numero = rand() % 999;
	char let[10];
	let[0] = 'E';
	let[1] = letras[rand() % 24 + 1];
	let[2] = letras[rand() % 24 + 1];
	let[3] = letras[rand() % 24 + 1];
	let[4] = '\0';
	char nu[4];
	sprintf(nu, "%03d", numero);
	strcat(let,nu);
	char *e = (char *)malloc(sizeof(char) * 8);
	strcpy(e, let);
	return e;
}

void cargar_hora(t_autos *a){
	time_t t;
	struct tm *tm;
	t = time(NULL);
	tm=localtime(&t);
	a->ent.hora = tm->tm_hour;
	a->ent.min = tm->tm_min;
}

void cargar_hora_salida(t_autos *a){
	time_t t;
	struct tm *tm;
	t = time(NULL);
	tm=localtime(&t);
	a->ent.hora = tm->tm_hour + (rand() % 4 + 1);
	a->ent.min = tm->tm_min;
}

void cargar_entrada(t_autos *a, char *patente){
	strcpy(a->patente, patente);
	strcpy(a->condicion, "Entrada");
}

void * entrada_cam(void *args){
	t_autos a;
	printf("Camara de Entrada Activada\n");
	while(fin){
		sem_wait(escritura);
		char *patente = damePatente();
		cargar_hora(&a);
		if(!fuera)
			printf("%s %02d:%02d\n", patente, a.ent.hora, a.ent.min);
		cargar_entrada(&a, patente);
		pthread_mutex_lock(&mutex); // RC
		if(registrar_entrada(&lista, &a) != DUPLICADO)
			write(fd,patente,sizeof(patente));
		free(patente);
		pthread_mutex_unlock(&mutex); // FIN RC
		sleep(rand() % 15);
	}
	pthread_mutex_unlock(&salir);
}

void * salida_cam(void *args){
	t_autos a;
	printf("Camara de Salida Activada\n\n\n");
	sleep(rand() % 30);
	while(fin){
		sem_wait(escritura);
		pthread_mutex_lock(&mutex); // RC
		int ra = rand()  % catidad_elementos(&lista);
		if(sacar_random(&lista, &a, ra) != 0){
			cargar_hora_salida(&a);
			a.patente[0] = 'S';
			if(!fuera)
				printf("%s %02d:%02d\n", a.patente, a.ent.hora, a.ent.min);
			write(fd,a.patente,sizeof(a.patente));
		}
		
		pthread_mutex_unlock(&mutex); // FIN RC
		sleep(rand() % 19);
	}
	
	pthread_mutex_unlock(&salir);
}

void mje(){
	printf("No puede cerra el programa desde las camaras debe cerrar el estacionamiento\n");
}


void lib(){
	printf("Necesita la peticion de cierre por parte del estacionamiento\n");
	sem_wait(cerrar_camaras);
	fin = 0;

	system("clear");

	printf("Terminando .. de procesar espere ...\n");
	printf("No cierre el terminal ...\n");
	fuera = 1;

	pthread_mutex_lock(&salir);
	pthread_mutex_lock(&salir);
	pthread_mutex_lock(&salir); // Terminan todos los hilos

	pthread_mutex_destroy(&mutex);
	
	close(fd);
	unlink("fifo/camara.fifo");
	
	vaciar_lista(&lista);
	printf("Ahora espere a que se cierre el estacionamiento\n");
	sem_post(cerrar_est);
	sem_wait(cerrar_camaras);
	sem_close(cerrar_camaras);
	sem_unlink("camaras");
	exit(0);
}

void *senal(void *args){
	while(fin){
		signal(SIGINT, lib);
        signal(SIGTERM, lib);
        sleep(1);
	}
}

int main(char *args, char argv[]){
	abrir = sem_open("abrir",O_CREAT, 0600, 0);

	mkfifo("fifo/camara.fifo",0666);
	sem_post(abrir); // permite la apertura del estacionamiento
	printf("Ya puede abrir el estacionamiento ...\n");
	fd = open("fifo/camara.fifo",O_WRONLY);

	escritura = sem_open("escritura",O_CREAT, 0600, 1);
	cerrar_est = sem_open("estacionamiento",O_CREAT, 0600, 0);
	cerrar_camaras = sem_open("camaras",O_CREAT, 0600, 0);

	// Creo la lista 
	crear_lista_autos(&lista);

	// Creacion de hilos 
	pthread_create(&sen, NULL, senal, NULL);
	pthread_create(&entrada, NULL, entrada_cam, NULL);
	pthread_create(&salida, NULL, salida_cam, NULL);

	// Cierre de hilos
	pthread_join(salida, NULL);
	pthread_join(entrada, NULL);
	pthread_join(sen, NULL);

	// Perimiten que se cierre todo antes de salir 
	pthread_mutex_lock(&terminar);
	pthread_mutex_destroy(&terminar);
	return 0;
}
