#include "lista.c"

t_lista lista;
pthread_mutex_t archivo = PTHREAD_MUTEX_INITIALIZER;
int serverSocket;

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
    char evaluacion[100];
    int nota;
}t_registro;


int soc; // Global sincronizar
FILE *alumnos; // sincronizar

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

void txt_a_estructura_paquete(char *linea, t_registro *r){
    char *aux = strchr(linea, '.');
    *aux = '\0';
    aux = strrchr(linea, ',');
    *aux = '\0';
    sscanf(aux + 1, "%d", &r->nota);
    aux = strrchr(linea, ',');
    *aux = '\0';
    strcpy(r->evaluacion, aux + 1);
    aux = strrchr(linea, ',');
    *aux = '\0';
    strcpy(r->materia, aux + 1);
    sscanf(linea, "%d", &r->dni);
}


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
    sscanf(linea, "%d", &r->dni);
}

void  serializarPaquete(char *linea, t_paquete *p){
    sprintf(linea, "%d|%s|%d|%f\n", p->socket, p->registro, p->op, p->resultado);
}

void  manejadorSenalCorte(){
    t_paquete p;
    char linea[100];
    p.op = -1;
    int ele;
    serializarPaquete(linea, &p);
    while(!lista_vacia(&lista)){
        sacar(&lista, &ele);
        write(ele, linea, strlen(linea));
    }
    printf("El servidor se cerro\n" );
    close(serverSocket);
    exit(1);
}

void *senales(void *args){
    while(1){
        signal(SIGINT, manejadorSenalCorte);
        signal(SIGTERM, manejadorSenalCorte);
        sleep(1);
    }
}

void enviarConexion(int connfd){
    char linea[10000];
    t_paquete p;
    p.socket = connfd;
    p.op = 1;
    serializarPaquete(linea, &p);
    write(connfd, linea, strlen(linea));
}

 void sacar_promedio_general(t_paquete *p){
    alumnos = fopen("alumnos.txt", "rt");
    char linea[500];
    char lineaPaquete[500];
    t_registro comp;
    t_registro r;
    int suma = 0;
    int cantidad = 0;
    txt_a_estructura_paquete(p->registro, &comp);
    rewind(alumnos);
    fgets(linea, sizeof(linea), alumnos);
    while(!feof(alumnos)){
        txt_a_estructura(linea, &r);
        if(r.dni == comp.dni){
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

 void sacar_promedio_materia(t_paquete *p){
    alumnos = fopen("alumnos.txt", "rt");
    char linea[500];
    char lineaPaquete[500];
    t_registro comp;
    t_registro r;
    int suma = 0;
    int cantidad = 0;
    txt_a_estructura_paquete(p->registro, &comp);
    rewind(alumnos);

    fgets(linea, sizeof(linea), alumnos);
    while(!feof(alumnos)){
        txt_a_estructura(linea, &r);
        if(r.dni == comp.dni && strcasecmp(r.materia, comp.materia) == 0){
            suma += r.nota;
            cantidad ++;
        }
        fgets(linea, sizeof(linea), alumnos);
    }
    if(cantidad > 0)
        p->resultado =  (float)suma / cantidad;
    else
        p->resultado = 0;
    strcpy(p->registro, comp.materia);
    fclose(alumnos);
 }

 void ingresarRegistro(t_paquete *p){
    alumnos = fopen("alumnos.txt", "r+t");
    t_registro r;
    txt_a_estructura_paquete(p->registro, &r);
    fseek(alumnos, 0L, SEEK_END);
    fprintf(alumnos, "%d,%s,%s,%d\n", r.dni, r.materia, r.evaluacion, r.nota);
    fclose(alumnos);
 }


void mostrarBaseDatos(){
    system("clear");
    alumnos = fopen("alumnos.txt", "rt");
    t_registro r;
    char linea[100];
    fgets(linea, sizeof(linea), alumnos);
    printf("%-10s %-30s %-20s %-3s\n\n", "DNI", "Materia", "Evaluacion", "Nota");
    while(!feof(alumnos)){
        txt_a_estructura(linea, &r);
        printf("%-10d %-30s %-20s %02d\n", r.dni, r.materia, r.evaluacion, r.nota);
        fgets(linea, sizeof(linea), alumnos);
    }

    fclose(alumnos);

}

void * atenderClientes(void * args){
    t_cliente cli;
    cli.socket = soc;
    char rec[5000];
    memset(rec, '0',sizeof(rec));
    int bytesRecibidos = 0;
    char linea[1000];



    while ( (bytesRecibidos = read(cli.socket, rec, sizeof(rec)-1)) > 0){
        rec[bytesRecibidos] = 0;
        t_paquete p = deserializarPaquete(rec);
        pthread_mutex_lock(&archivo);
        switch(p.op){
            case 1:
                printf("Cliente %d calcula promedio general....\n", cli.socket);
                sacar_promedio_general(&p);
                p.op=2;
                serializarPaquete(linea, &p);
                write(cli.socket, linea, strlen(linea));
            break;
            case 2:
                printf("Cliente %d calcula promedio de la materia....\n", cli.socket);
                sacar_promedio_materia(&p);
                p.op=3;
                serializarPaquete(linea, &p);
                write(cli.socket, linea, strlen(linea));
            break;
            case 3:
                printf("Cliente %d agrega un registro....\n", cli.socket);
                ingresarRegistro(&p);
                p.op=4;
                serializarPaquete(linea, &p);
                write(cli.socket, linea, strlen(linea));
            break;
            case 4:
                mostrarBaseDatos();
            break;
            case -1:
                printf("Cliente %d desconectado....\n", cli.socket);
                sacar_clave(&lista, &cli.socket);
                close(cli.socket);
            break;
        }
        pthread_mutex_unlock(&archivo);
        memset(rec, '0',sizeof(rec));
    }
}



void iniciarServidor(int puerto){
    alumnos = fopen("alumnos.txt", "r+t");
    if(!alumnos){
        printf("Archivo inexistente");
        return;
    }

    crear_lista(&lista);
    struct sockaddr_in serv_addr; 
    char sendBuff[1025];

    memset(sendBuff, '0', sizeof(sendBuff)); 
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(puerto); 

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 
    time_t ticks; 
    pthread_t sen ;
    pthread_create(&sen, NULL, senales, NULL);
    serverSocket = listenfd;
    system("clear");
    printf("Esperando Conexiones ....\n");
    while (1){
        int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        soc = connfd;
        printf("Cliente %d Conectado....\n", connfd);
        enlistar(&lista, &connfd);
        enviarConexion(connfd);
        pthread_t cli ;
        pthread_create(&cli, NULL, atenderClientes, NULL);
    }
}
