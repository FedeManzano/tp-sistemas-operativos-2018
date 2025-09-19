#include "lib/libServidor.c"

/**
	Alumno: 	Federico Manzano
	Ejercicio:	4
	TP:			3
*/

int main(int argc, char *argv[])
{
	if(argv[1] != NULL){

		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-?") == 0 ){
			printf("%s\n", "AYUDA");
			printf("%s\n", "Aplicacion servidor para el sistema de administracion de notas");
			printf("%s\n", " ***  Ejecucion  ***");
			printf("%s\n", "[rutaServidor]servidor -> Puerto por defecto 'conf/servidor.conf' ");
			printf("%s\n", "[rutaServidor]servidor [puerto] ");
			printf("%s\n", "***  Complilacion ***");
			printf("%s\n", "[rutaMakeFile]make all");
			return 1;
		}
		int valPuerto = atoi(argv[1]);
		if(valPuerto > 0)
			iniciarServidor(valPuerto);
		else
			printf("El numero de puerto es incorrecto\n");
		return 0;
	}

	if(argv[1] == NULL){
		char op = '0';
		printf("Los parametros no poseen numero de puerto \n ");
		printf("Desea utilizar los valores por defecto (S/N) ?: ");
		fflush(stdin);
		scanf("%c", &op);
		if(op == 'S' || op == 's'){
			FILE *conf = fopen("conf/servidor.conf","rt");
			if(!conf){
				printf("La configuracion por defecto fue borrada\n");
				return 10;
			}

			char l[100];
			fgets(l,sizeof(l), conf);
			int puerto = atoi(l);
			iniciarServidor(puerto);
			close(conf);
		}
	}


    return 0;
}
