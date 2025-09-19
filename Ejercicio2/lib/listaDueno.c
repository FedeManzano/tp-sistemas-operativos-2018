#include "include/lista.h"

void crear_lista_dueno(t_lista_dueno *l){
	*l = NULL;
}

int insertar_dueno(t_lista_dueno *l, t_info_dueno *info, t_cmp clave){
	while(*l && clave(&(*l)->info, info))
		l = &(*l)->sig;
	if(*l && !clave(&(*l)->info, info)){
		(*l)->info.cantidad ++;
		return 2;
	}
	t_nodo_dueno *nuevo = (t_nodo_dueno *)malloc(sizeof(t_nodo_dueno));
	if(!nuevo)
		return 0;
	info->cantidad ++;
	nuevo->info = *info;
	nuevo->sig = *l;
	*l = nuevo;
	return 1;
}

int sacar_mayor(t_lista_dueno *l, t_info_dueno *info){
	
	if(!*l)
		return 0;
	t_info_dueno iaux = (*l)->info;

	while(*l){
		if((*l)->info.cantidad > iaux.cantidad)
			iaux = (*l)->info;
		l = &(*l)->sig;
	}
	*info = iaux;
	return 1;
}

void vaciar_lista_dueno(t_lista_dueno *l){
	t_nodo_dueno *e;
	while(*l)
	{
		e = *l;
		*l = e->sig;
		free(e);
	}
}