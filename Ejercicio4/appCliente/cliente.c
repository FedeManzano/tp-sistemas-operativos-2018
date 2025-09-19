#include "lib/libCliente.c"

/**
	Alumno: 	Federico Manzano
	Ejercicio:	4
	TP:			3
*/
int main(int argc, char *argv[])
{
	if(argv[1] == NULL){
		printf("El parametro con el nombre de la materia que dicta es obligatorio\n" );
		printf("Consulte la ayuda para mas informacion\n" );
		printf("[ruta]cliente [-h] / [-help] / [-?]\n" );
		return 5;
	}

	if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-?") == 0){
		printf("%s\n", "AYUDA");
		printf("%s\n", "Aplicacion cliente para el sistema de administracion de notas");
		printf("%s\n", " ***  Ejecucion  ***");
		printf("%s\n", "[rutaCliente]cliente [ipServidor] [puerto] [materia]");
		printf("%s\n", "[rutaCliente]cliente [ipServidor] [materia] -> Puerto por defecto");
		printf("%s\n", "[rutaCliente]cliente [materia] -> IP y Puerto por defecto");
		printf("%s\n", "***  Complilacion ***");
		printf("%s\n", "[rutaMakeFile]make all");
		return 1;
	}

	if(argv[2] == NULL){
		int val = atoi(argv[1]);
		if(val != 0){
			printf("El parametro con el nombre de la materia que dicta es obligatorio\n" );
			printf("Consulte la ayuda para mas informacion\n" );
			printf("[ruta]cliente [-h] / [-help] / [-?]\n" );
			return 5;
		}

		char op = '0';
		printf("Los parametros insertados no son suficientes \n ");
		printf("Desea utilizar los valores por defecto (S/N) ?: ");
		fflush(stdin);
		scanf("%c", &op);

		if(op == 's' || op == 'S'){
			FILE *conf = fopen("conf/cliente.conf", "rt");
			if(!conf){
				printf("La configuracion por defecto fue borrada ...");
				return 10;
			}

			char ip[60];
			fgets(ip, sizeof(ip), conf);
			char puertoString[60];
			fgets(puertoString, sizeof(puertoString), conf);
			int puerto = atoi(puertoString);
			fclose(conf);
			iniciarCliente(ip, puerto, argv[2]);
			return 0;
		}
	}

	if(argv[1] != NULL && argv[2] != NULL && argv[3] == NULL){
		int valIp = atoi(argv[1]);
		int valNom = atoi(argv[2]);
		if(valIp == 0){
			printf("IP del servidor es erroneo\n" );
			return 10;
		}

		if(valNom > 0){
			printf("El nombre de la materia es erroneo\n" );
			return 10;
		}

		char op = '0';
		printf("Le falta ingresar el numero de puerto \n ");
		printf("Desea utilizar el puerto por defecto (S/N) ?: ");
		fflush(stdin);
		scanf("%c", &op);

		if(op == 's' || op == 'S'){
			FILE *conf = fopen("conf/cliente.conf", "rt");
			if(!conf){
				printf("La configuracion por defecto fue borrada ...");
				return 10;
			}
			char puertoS[100];
			fgets(puertoS, sizeof(puertoS), conf);
			fgets(puertoS, sizeof(puertoS), conf);
			int puerto = atoi(puertoS);
			fclose(conf);
			iniciarCliente(argv[1], puerto, argv[2]);
			return 0;
		}
	}

	if(argv[1] != NULL && argv[2] != NULL && argv[3] != NULL){
		printf("dasdasdasdsa\n");
		int valIp = atoi(argv[1]);
		int valPuerto = atoi(argv[2]);
		int valNom = atoi(argv[3]);

		if(valIp <= 0){
			printf("IP del servidor es erroneo\n" );
			return 10;
		}

		if(valPuerto <= 0){
			printf("El puerto es erroneo\n" );
			return 10;
		}

		if(valNom > 0){
			printf("El nombre de la materia es erroneo\n" );
			return 10;
		}
		int puerto = atoi(argv[2]);
		iniciarCliente(argv[1], puerto, argv[3]);
		return 0;
	}


    return 0;
}
