#ifndef _lista_H_
#define _lista_H_

#include "tipos.h"


typedef int (* t_cmp)(const void *, const void *);
typedef void (* t_act)(void *, const void *);
typedef void (* t_titulo)();
typedef void (* t_imp)(void * );


typedef struct s_nodo_dueno {
	struct s_nodo_dueno *sig;
	t_info_dueno info;
}t_nodo_dueno;

typedef t_nodo_dueno * t_lista_dueno;


typedef struct s_nodo {
	struct s_nodo *sig;
	t_info info;
}t_nodo;

typedef t_nodo * t_lista;


void crear_lista_dueno(t_lista_dueno *l);
int insertar_dueno(t_lista_dueno *l, t_info_dueno *info, t_cmp clave);
int sacar_mayor(t_lista_dueno *l, t_info_dueno *info);
void vaciar_lista_dueno(t_lista_dueno *l);

void crear_lista(t_lista * l);
int lista_vacia(t_lista *l);
void inicializar(t_nodo *nodo);
int insertar(t_lista *l, t_info *info, t_cmp clave, t_act act);
int actualizar(t_lista *l, t_info *info, t_cmp clave, t_act act);
int sacar(t_lista *l, t_info *info);
void vaciar_lista(t_lista *l);
int cantidad_total(t_lista *l);
void mostrar(t_lista *l, t_titulo ti, t_imp imp);
int buscar_extension(t_lista *l, t_info *info, t_cmp clave);
t_nodo **buscar_mayor_menor(t_lista *l, t_cmp tam_mayor_menor);
void ordenar(t_lista *l, t_cmp tam_mayor_menor);

#endif
