#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>



#define LISTA_LLENA 0
#define LISTA_VACIA 0
#define ENCONTRADO 1
#define NO_ENCONTRADO 0
#define ACTUALIZADO 2
#define NO_ACTUALIZADO 3
#define DUPLICADO 2
#define SIN_ELEMENTOS -3
#define OK 1

typedef struct {
	int hora;
	int min;
}t_hora;

typedef struct {
	char condicion[15];
	char patente[8];
	t_hora ent;
  	float monto;
	int estado;
}t_autos;

typedef struct s_nodo {
	struct s_nodo *sig;
	t_autos info;
}t_nodo_autos;


typedef t_nodo_autos *t_lista_autos;


void crear_lista_autos(t_lista_autos *l){
	*l = NULL;
}

int lista_vacia(t_lista_autos *l){
	return *l == NULL;
}

int insertar(t_lista_autos *l, t_autos *info){
	int comp = 0;
	while(*l && (comp = strcmp((*l)->info.patente, info->patente)) != 0)
		l = &(*l)->sig;
	if(*l && comp == 0){
		if(strcmp(info->condicion, "Salida") == 0){
			(*l)->info.monto = info->monto;
			(*l)->info.estado = 1;
			return ACTUALIZADO;
		}else
			return NO_ACTUALIZADO;
	}
	while(*l)
		l = &(*l)->sig;
	t_nodo_autos *nuevo = (t_nodo_autos *)malloc(sizeof(t_nodo_autos));
	if(!nuevo)
		return LISTA_LLENA;
	nuevo->info = *info;
	nuevo->info.estado = 0;
	nuevo->info.monto = 0;
	nuevo->sig = *l;
	*l = nuevo;
	return OK;
}

int registrar_entrada(t_lista_autos *l, t_autos *info){
	while(*l && strcmp((*l)->info.patente, info->patente) != 0)
		l = &(*l)->sig;
	if(*l && strcmp((*l)->info.patente, info->patente) == 0)
		return DUPLICADO;
	while(*l)
		l = &(*l)->sig;
	t_nodo_autos *nuevo = (t_nodo_autos *)malloc(sizeof(t_nodo_autos));
	if(!nuevo)
		return LISTA_LLENA;
	nuevo->info = *info;
	nuevo->sig = *l;
	*l = nuevo;
	return OK;
}

int sacar_random(t_lista_autos *l, t_autos *info,int ran){
	t_nodo_autos *p;
	if(!*l)
		return LISTA_VACIA;
	while(*l && ran > 0){
		l = &(*l)->sig;
		ran --;
	}
	if(*l){
		p = *l;
		*info = p->info;
		*l = p->sig;
		free(p);
		return OK;
	}
	return LISTA_VACIA;
}

int catidad_elementos(t_lista_autos *l){
	int cant = 0;
	if(!*l)
		return SIN_ELEMENTOS;
	while(*l){
		l = &(*l)->sig;
		cant ++;
	}
	return cant;
}

void vaciar_lista(t_lista_autos *l){
	t_nodo_autos *p;
	while(*l)
	{
		p = *l;
		*l = p->sig;
		free(p);
	}
}

int buscar_clave(t_lista_autos *l, t_autos *info){
	while(*l && strcmp((*l)->info.patente, info->patente) != 0)
		l = &(*l)->sig;
	if(*l && strcmp((*l)->info.patente, info->patente) == 0)
		if((*l)->info.estado == 0){
			*info = (*l)->info;
			return OK;
		}
	return NO_ENCONTRADO;
}
