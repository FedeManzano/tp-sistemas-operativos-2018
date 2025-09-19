
#include "include/lista.h"

void crear_lista(t_lista * l){
	*l = NULL;
}

int lista_vacia(t_lista *l){
	return *l == NULL;
}

void inicializar(t_nodo *nodo){
	t_nodo *n = (t_nodo *)nodo;
	n->info.cantidad 	= 0;
	n->info.tam_mayor 	= 0;
	n->info.tam_menor 	= 0;
	n->info.dueno.cantidad = 0;
}

int insertar(t_lista *l, t_info *info, t_cmp clave, t_act act){
	while(*l && clave(&(*l)->info, info) != 0 )
		l = &(*l)->sig;
	if(*l && !clave(&(*l)->info, info)){
		act(&(*l)->info, info);
		return 2;
	}

	t_nodo *nuevo = (t_nodo *)malloc(sizeof(t_nodo));
	if(!nuevo)
		return 0;
	nuevo->info = *info;
	inicializar(nuevo);
	nuevo->sig = *l;
	*l = nuevo;
	return 1;
}

int actualizar(t_lista *l, t_info *info, t_cmp clave, t_act act){
	while(*l && clave(&(*l)->info, info) != 0 )
		l = &(*l)->sig;
	if(*l && !clave(&(*l)->info, info)){
		act(&(*l)->info, info);
		return 2;
	}
	return 0;
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

void vaciar_lista(t_lista *l){
	t_nodo *e;
	while(*l)
	{
		e = *l;
		*l = e->sig;
		free(e);
	}
}

int cantidad_total(t_lista *l){
	int cantidadTotal = 0;
	while(*l){
		cantidadTotal += (*l)->info.cantidad;
		l = &(*l)->sig;
	}
	return cantidadTotal;
}

void mostrar(t_lista *l, t_titulo ti, t_imp imp){
	system("clear");
	printf("Cantidad Total %d: \n\n", cantidad_total(l));
	ti();
	imp(l);
}

int buscar_extension(t_lista *l, t_info *info, t_cmp clave){
	while(*l && clave(&(*l)->info, info) != 0 )
		l = &(*l)->sig;
	if(*l && !clave(&(*l)->info, info)){
		return 1;
	}
	return 0;
}


t_nodo **buscar_mayor_menor(t_lista *l, t_cmp tam_mayor_menor){
	t_nodo **mayor_menor = l;
	while(*l && (*l)->sig){
		if(tam_mayor_menor(&(*mayor_menor)->info, &(*l)->sig->info) < 0)
			mayor_menor = &(*l)->sig;
		l = &(*l)->sig;
	}
	return mayor_menor;
}


/**
	Funcion de lista para ordenar de mayor a menor por 
	tamaño de archivo
*/
void ordenar(t_lista *l, t_cmp tam_mayor_menor){
	t_nodo **mayor_menor = l;
	t_nodo *aux = (t_nodo *)malloc(sizeof(t_nodo));
	if(!aux)
		return;
	while(*l && (*l)->sig){
		mayor_menor = buscar_mayor_menor(l, tam_mayor_menor);
		aux->info = (*l)->info;
		(*l)->info = (*mayor_menor)->info;
		(*mayor_menor)->info = aux->info;
		l = &(*l)->sig;
	}
	free(aux);
}
