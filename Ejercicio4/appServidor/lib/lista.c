#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

typedef int t_info;
typedef struct s_nodo{
	struct s_nodo *sig;
	t_info info;
}t_nodo;


typedef t_nodo * t_lista;


void crear_lista(t_lista *l){
	*l = NULL;
}

int enlistar(t_lista *l, t_info *info){
	t_nodo *nuevo;
	nuevo = (t_nodo *)malloc(sizeof(t_nodo));
	if(!nuevo)
		return 0;
	nuevo->info = *info;
	nuevo->sig = *l;
	*l = nuevo;
	return 1;
}

int sacar(t_lista *l, t_info *info){
	t_nodo *e = *l;
	if(!*l)
		return 0;
	*info = e->info;
	*l = e->sig;
	free(e);
	return 1;
}

int lista_vacia(t_lista *lista){
	return *lista == NULL;
}

int sacar_clave(t_lista *l, t_info *info){
	t_nodo *e;
	while(*l && *info != (*l)->info)
		l = &(*l)->sig;
	if(!*l)
		return 0;
	e = *l;
	*info = e->info;
	*l = e->sig;
	free(e);
	return 1;
}
