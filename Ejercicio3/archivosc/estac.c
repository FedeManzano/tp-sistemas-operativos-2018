

/**
	Alumno: 	Federico Manzano
	Ejercicio:	3
	TP:			3
*/


#include "lib/lista.c"
#include <semaphore.h>
#define MAX_BUF 1024


pthread_mutex_t cerrar = PTHREAD_MUTEX_INITIALIZER;

int fin = 1;
int fd;

pthread_t recubirCamaras;
pthread_t sen ;

t_lista_autos lista;
FILE *logu;


int precio = 100;

sem_t *escritura;

sem_t *cerrar_est;
sem_t *cerrar_camaras;
sem_t *abrir;


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
	int ran = (15 * rand() % 3) % 60 < 0 ? ((15 * rand() % 3) % 60) * -1 :  (15 * rand() % 3) % 60;
	a->ent.min = tm->tm_min + ran;
}

void calcular_monto(t_hora *en, t_autos *a){
	float total;
	int preHora = ( a->ent.hora  - en->hora);
	int preMin = (a->ent.min - en->min);
	if(preHora < 0)
		preHora *= -1;
	if(preMin < 0){
		preHora --;
		preMin = 60 + preMin;
		preMin = preMin / 15; 
		total = (preHora * precio + preMin * (float)(precio / 4));
	}else{
		preMin = preMin / 15; 
		total = (preHora * precio + preMin * (float)(precio / 4));
	}
	a->monto = total;
}

void procesar_entrada(char *msj){
	t_autos a;
	strcpy(a.condicion, "Entrada");
	strcpy(a.patente, msj);
	cargar_hora(&a);
	strcpy(a.patente, msj + 1);
	int c = insertar(&lista, &a);
	if(c != 3 && c != 2 && c != 0){
		printf("%-10s%-10s%d:%-10d%-8s\n","Entrada", a.patente, a.ent.hora, a.ent.min, "-");
		fprintf(logu, "%-12s%-10s%02d:%02d\n", "Entrada",a.patente, a.ent.hora, a.ent.min);
	}
}

void procesar_salida(char *msj){
	t_autos a;
	strcpy(a.patente, msj + 1);
	if(buscar_clave(&lista, &a) == ENCONTRADO){
		strcpy(a.condicion, "Salida");
		t_hora ent;
		ent.hora = a.ent.hora;
		ent.min = a.ent.min;
		cargar_hora_salida(&a);
		calcular_monto(&ent, &a);
		insertar(&lista, &a);
		printf("%-10s%-10s%d:%-10d%-8f\n","Salida", a.patente, a.ent.hora, a.ent.min, a.monto);
		fprintf(logu, "%-12s%-10s%02d:%02d\n", "Salida",a.patente, a.ent.hora, a.ent.min);
	}
}

void apertura_fifo(){
	system("clear");
 	printf("Encienda Las camaras ....\n");
	fd = open("fifo/camara.fifo",O_RDONLY);
	system("clear");
	printf("Estacionamiento Abierto\n");
	printf("Precio por hora $ %d\n", precio);
}


void mostrar_cabecera(){
	printf("\n\n%-10s%-10s%-10s%-10s\n","Evento", "Patente", "Hora", "Precio");
	printf("%-10s%-10s%-10s%-10s\n","------", "------", "----", "------");
}

void * camaras(void *args){
	char msj[1024];
 	apertura_fifo();

 	mostrar_cabecera();

	while(fin){
		read(fd,msj,MAX_BUF);
		if(*msj == 'E')
			procesar_entrada(msj);
		else
			procesar_salida(msj);
		sem_post(escritura);
	}
	pthread_mutex_unlock(&cerrar);
}



char *  crear_facuracion_arch(){
	time_t t;
	struct tm *tm;
	char nombreArch[100];
 	char fechayhora[100];
  	t=time(NULL);
  	tm=localtime(&t);
  	strcpy(nombreArch, "facturacion/Facturacion_");
  	sprintf(fechayhora, "%d%d%d", (tm->tm_year + 1900), tm->tm_mon + 1, tm->tm_mday);
  	strcat(nombreArch, fechayhora);
  	char *nom = (char *)malloc(sizeof(nombreArch));
  	strcpy(nom, nombreArch);
  	return nom;
}



void generar_facturacion(){
	t_autos a;
	int i = 0;
	char *nombre = crear_facuracion_arch();
	FILE *fac = fopen(nombre, "wt");

	while(!lista_vacia(&lista)){
		sacar_random(&lista, &a, 0);
		if(a.estado == 1)
			fprintf(fac, "%-10s $%4.2f\n", a.patente, a.monto);
		else
			fprintf(fac, "%-10s %s\n", a.patente, "Aun en el estacionamiento");
		i ++;
	}


	free(nombre);
	fclose(fac);
}

void crear_log(){
	time_t t;
	struct tm *tm;
	char nombreArch[100];
 	char fechayhora[100];
  	t=time(NULL);
  	tm=localtime(&t);
  	strcpy(nombreArch, "log/Movimientos_");
  	sprintf(fechayhora, "%d%d%d", (tm->tm_year + 1900), tm->tm_mon + 1, tm->tm_mday);
  	strcat(nombreArch, fechayhora);
  	logu = fopen(nombreArch, "wt");
}


void liberar_recursos(){
	printf("\n\nEspere un momento ....\n");
	sem_post(cerrar_camaras); // Cuando se cierran las camaras libera
	printf("\nLa peticion de cierre fue enviado al proceso de las camaras\n");
	printf("Ahora puede apagar las camaras .....\n");
	printf("Cuando las camaras esten apagadas se realizara la facturacion\n\n\n");
	sem_wait(cerrar_est);// Cuando se cierran las camaras libera

	pthread_mutex_lock(&cerrar);
	fin = 0; // Termina los hilos
	pthread_mutex_lock(&cerrar);

	// Borrar fifo
	close(fd);
	unlink("fifo/camara.fifo");

	// Borrar semaforo
	sem_close(escritura);
	sem_unlink("escritura");
	sem_close(abrir);
	sem_unlink("abrir");
	sem_close(cerrar_est);
	sem_unlink("estacionamiento");


	fclose(logu);

	// Genera la facturacion
	generar_facturacion();
	vaciar_lista(&lista);
	
	system("clear");

	char *r = crear_facuracion_arch();
	printf("Factulacion en ./%s\n", r);
	printf("Fin\n");
	free(r);
	sem_post(cerrar_camaras);
}


void condicion_de_fin(){
	char op;
	printf("En cualquier momento puede presionar t / T para terminar \n\n\n\n");
	do{
		fflush(stdin);
		scanf("%c", &op);
	} while(op != 't' && op != 'T');
	liberar_recursos();
}


void *senal(void *args){
	while(1){
		signal(SIGINT, liberar_recursos);
        signal(SIGTERM, liberar_recursos);
        sleep(1);
	}
}

void ayuda(){
	printf(" ******* AYUDA ******\n");
	printf("Simulacion de un estacionamiento de autos utilizando Fifo para la comunicacion\n");
	printf("\nEjecucion:\n");
	printf("Ejecutar el proceso de las camaras: \n");
	printf("[Ruta]./camaras\n");
	printf("Luego ejecutar el procesos estacionamiento:\n");
	printf("[Ruta]./stac\n\n");
	printf("\nCerrar:\n");
	printf("\nEnviar una peticion desde las camaras con Ctrl + C");
	printf("\nLuego Ctrl + C desde las camaras\n\n");
	printf("DESCRIPCION:\n");
	printf("\nEl programa simula las patentes y la hora de ingreso \n");
	printf("y egreso de los autos al estacionamiento.\n\n");
	
}


int validar_parametros(char * argv[]){
	if(argv[1] != NULL){
		if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "-?")){
			ayuda();
			return 1;
		}
		precio = atoi(argv[1]);
		if(precio == 0){
			printf("El precio ingresado es erroneo \n");
			return 5;
		}
	}
	return 0;
}

int main(char *args, char * argv[]){
	if(validar_parametros(argv))
		return 0;
	
	// Semaforos IPC
	escritura = sem_open("escritura",O_CREAT, 0600, 1);
	cerrar_est = sem_open("estacionamiento",O_CREAT, 0600, 0);
	cerrar_camaras = sem_open("camaras",O_CREAT, 0600, 0);
	abrir = sem_open("abrir",O_CREAT, 0600, 0);

	printf("Esperando que se enciendan las camaras\n" );
	sem_wait(abrir); // Cuando se encvienden las camaras desbloquea
	pthread_create(&sen, NULL, senal, NULL);

	// Inicializar recursos compartidos
	crear_log();
	crear_lista_autos(&lista);

	// Iniciar hilo lector 
	pthread_create(&recubirCamaras, NULL, camaras, NULL);
	pthread_join(recubirCamaras, NULL);
	return 0;
}
