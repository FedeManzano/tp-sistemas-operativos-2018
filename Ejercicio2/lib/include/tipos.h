#ifndef _tipos_H_
#define _tipos_H_
	
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
	int cantidad;
	uid_t nombre;
}t_dueno;

typedef t_dueno t_info_dueno;

typedef struct {
	char ext[5];
	long tam_menor;
	long tam_mayor;
	long tam;
	int cantidad;
	t_dueno dueno;
}t_extension;

typedef t_extension t_info;


#endif