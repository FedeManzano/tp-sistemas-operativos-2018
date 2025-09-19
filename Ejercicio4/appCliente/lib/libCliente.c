#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

// Macros para validar el nombre de la materia que viene por parametro
#define ES_BLANCO(X) ((X) == ' ')
#define ES_MIN(X) ((X) >= 'a' && (X) <= 'z')
#define ES_MAY(X) ((X) >= 'A' && (X) <= 'Z')
#define ES_NUM(X) ((X) >= '0' && (X) <= '9')

int soc;

typedef struct {
	int socket;
}t_cliente;

typedef struct {
    int socket;
    char registro[1000];
    int op;
    float resultado;
}t_paquete;

typedef struct {
    int dni;
    char materia[100];
    char evaluacion;
    int nota;
}t_registro;


int validarEvaluacion(char *eva){
    return  !strcasecmp(eva, "Parcial1")        || 
            !strcasecmp(eva, "Parcial2")        || 
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

t_paquete deserializarPaquete(char *linea){
    t_paquete p;
    char *aux = strchr(linea, '\n');
    *aux = '\0';
    aux = strrchr(linea, '|');
    *aux = '\0';
    sscanf(aux + 1, "%f", &p.resultado);
    aux = strrchr(linea, '|');
    *aux = '\0';
    sscanf(aux + 1, "%d", &p.op);
    aux = strrchr(linea, '|');
    *aux = '\0';
    strcpy(p.registro, aux + 1);
    sscanf(linea, "%d", &p.socket);
    return p;
}


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

void  serializarPaquete(char *linea, t_paquete *p){
    sprintf(linea, "%d|%s|%d|%f\n", p->socket, p->registro, p->op, p->resultado);
}



void imprimir_menu(){
    printf("A-Consultar promedio general\n");
    printf("B-Consultar promedio de una materia\n");
    printf("C-Agregar nota\n");
    printf("D-Ver configuracion por defecto\n");
    printf("S-Salir\n\n");

}

void pausa(){
    int c;
    printf("Precione ( C / c ) para continuar ...\n");
    do {
        c=getchar();
    } while (c != 'C' && c != 'c');
}

char seleccionar_opcion(){
    char op;
    printf("Ingrese una opcion: ");
    do{
        
        fflush(stdin);
        scanf("%c", &op);
    }while(toupper(op) != 'A' && toupper(op) != 'B' && 
        toupper(op) != 'C' && toupper(op) != 'S' && toupper(op) != 'D' && toupper(op) != 'Z');
    return toupper(op);
}

void  manejadorSenalCorte(){
    t_paquete p;
    char linea[100];
    p.op = -1;
    int ele;
    serializarPaquete(linea, &p);
    write(soc, linea, strlen(linea));
    close(soc);
    exit(1);
}

void promGeneral(t_paquete *p){
    char *dni = ingresar_dni();
    strcpy(p->registro, dni);
    free(dni);
    strcat(p->registro, ",");
    strcat(p->registro, ",");
    strcat(p->registro, ",");
    strcat(p->registro, ".");
    p->op = 1;
    p->resultado = 0;
}

void promMateria(t_paquete *p){
    char *dni = ingresar_dni();
    char *materia = ingresar_materia();
    normalizarMateria(materia);
    strcpy(p->registro, dni);
    strcat(p->registro, ",");
    strcat(p->registro, materia);
    strcat(p->registro, ",,.");
    p->op = 2;
    p->resultado = 0;

    free(dni);
    free(materia);
}

void ingresarDatos(t_paquete *p, char *materia){
    char *dni = ingresar_dni(); 
    char *eva = ingresar_evaluacion();
    char *nota = ingresar_nota();

    normalizarMateria(materia);
    normalizarMateria(eva);

    strcpy(p->registro, dni);
    strcat(p->registro, ",");
    strcat(p->registro, materia);
    strcat(p->registro, ",");
    strcat(p->registro, eva);
    strcat(p->registro, ",");
    strcat(p->registro, nota);
    strcat(p->registro, ".");
    p->op = 3;
    p->resultado = 0;
    free(dni);
    free(nota);
    free(eva);
}

void verConfDefecto(){
    system("clear");
    FILE *confCli = fopen("conf/cliente.conf","rt");

    char cli[100];

    fgets(cli, sizeof(cli), confCli);
    printf("El ip del servidor por defecto es %s\n", cli);
    fgets(cli, sizeof(cli), confCli);
    printf("El puerto por defecto es %s\n", cli);
    fclose(confCli);
}

void mostrarMenu(int soc, char *materia){
	system("clear");
    char op;
	t_paquete p;
	char linea[1000];
	printf("Menu de opciones (Profesor %s)\n\n", materia);

	imprimir_menu();
    op = seleccionar_opcion();
	switch(op){
		case 'A':
            system("clear");
			promGeneral(&p);
			serializarPaquete(linea, &p);
			write(soc, linea, strlen(linea));
			break;
		case 'B':
            system("clear");
            promMateria(&p);
            serializarPaquete(linea, &p);
            write(soc, linea, strlen(linea));
			break;
		case 'C':
            system("clear");
            ingresarDatos(&p, materia);
            serializarPaquete(linea, &p);
            write(soc, linea, strlen(linea));
			break;
        case 'D':
            verConfDefecto();
            pausa();
            mostrarMenu(soc,materia);
            break;
        case 'Z':
            p.op = 4;
            serializarPaquete(linea, &p);
            write(soc, linea, strlen(linea));
            mostrarMenu(soc, materia);
            break;
		case 'S':
            manejadorSenalCorte();
			break;
	}
}

void *senales(void *args){
    while(1){
        signal(SIGINT, manejadorSenalCorte);
        signal(SIGTERM, manejadorSenalCorte);
        sleep(1);
    }
}



void iniciarCliente(char *ip, int puerto,char *materia){

    normalizarMateria(materia);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Error: No se pudo crear el socket.");
        return ;
    } 

    char recvBuff[5000];
    memset(recvBuff, '0',sizeof(recvBuff));

    struct sockaddr_in serv_addr; 
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto); 

    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    
    if ( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("Error: No se pudo conectar\n");
       return ;
    } 
    soc = sockfd;
    pthread_t sen ;
    pthread_create(&sen, NULL, senales, NULL);
    
    system("clear");
    int bytesRecibidos = 0;
    while ( (bytesRecibidos = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
    	int res;
        recvBuff[bytesRecibidos] = 0;
        t_paquete p ;
        p = deserializarPaquete(recvBuff);

        switch(p.op){
        	case 1:
        		mostrarMenu(sockfd, materia);
        	   break;
        	case 2:
                system("clear");
        		printf("El promedio general de %s es: %f\n", p.registro, p.resultado);
                pausa();
                mostrarMenu(sockfd, materia);
        		break;
        	case 3:
                system("clear");
        		printf("El promedio de la materia  %s de este alumno es: %4.2f\n\n",p.registro, p.resultado);
                pausa();
                mostrarMenu(sockfd, materia);
        	   break;
        	case 4:
                system("clear");
        		printf("La nota del alumno %s fue ingresada con exito",p.registro);
                pausa();
                mostrarMenu(sockfd, materia);
        	   break;
            case -1:
                printf("El servidor cerro su sesion\n");
                printf("Intente de nuevo mas tarde\n\n\n");
                close(sockfd);
                break;
        
    	} 
    	bytesRecibidos = 0;
    	memset(recvBuff, '0',strlen(recvBuff));
	}

    printf("\nGracias !!!\n\n");
}
