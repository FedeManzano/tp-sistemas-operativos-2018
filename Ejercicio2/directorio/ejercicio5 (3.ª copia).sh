#!/bin/bash
IFS="$(printf '\n\t')"
AYUDA="0"
CANTIDAD="0"
PARAMETROS="0"
EXTENSION="0"
ARCHIVOS="0"
OPERAICION="0"
PERMISOS="0"
VACIO="0"

#Validaciones

VAL_DNI="0"
VAL_NOMBRE="0"
VAL_APELLIDO="0"


function ayuda(){
	echo -e "\n\n\e[1;31m**** Ayuda *****\e[0m"
        echo -e "\n\e[1;34m###Ejecucion###\e[0m"
        echo -e "[Ruta del Script]/ejercicio5.sh par1 par2 par3 par4 par5 par6 par7"
        echo -e "./ejercicio5.sh par1 par2 par3 par4 par5 par6 par7"
	echo -e "\n\e[1;34m###Extension###\e[0m"
	echo -e "[.txt]: Extension de la base personas.txt"
	echo -e "[.txt]: Extension de la base paises.txt"
	echo -e "\e[0;36mNOTA: La extension de los archivos de entrada tienen que ser [.txt] Obligatoriamente\e[0m"	
        echo -e "\n\e[1;34m###Parametros###\e[0m"
        echo -e "\e[0;36mPrimero: [ruta de personas.txt]\e[0m"  
	echo -e "\e[0;36mSegundo: [ruta de paises.txt]\e[0m"
        echo -e "\e[0;36m[-d]\e[0m          : Muestra los datos de la persona segun el DNI"
        echo -e "\e[0;36m[-a]\e[0m          : Inserta a la base de datos una nueva persona"
        echo -e "\e[0;36m[-e]\e[0m          : Elimina de la base de datos a una persona"
        echo -e  "\n\e[1;34m###Ejemplo###\e[0m"
	echo -e "\e[0;36m**** Muestra los datos de la persona con el dni ingresado.\e[0m"
        echo -e "./ejercicio5 [Ruta]/personas.txt [Ruta]/paises.txt [-d] [dni]\n"
	echo -e "\e[0;36m**** Agrega a una persona con los datos ingresados.\e[0m"
	echo -e "./ejercicio5 [Ruta]/personas.txt [Ruta]/paises.txt [-a] [dni] [npmbre] [apellido] [pais]\n"
	echo -e "\e[0;36m**** Elimina a una persona por el dni ingresado.\e[0m"
	echo -e "./ejercicio5 [Ruta]/personas.txt [Ruta]/paises.txt [-e] [dni]\n"
	echo -e "\e[0;36m**** Muestra a todas las personas de un determinado pais.\e[0m"
	echo -e "./ejercicio5 [Ruta]/personas.txt [Ruta]/paises.txt [-p] [pais]\n\n"
	echo -e "\e[1;31mIMPORTANTE: La ejecucion del script solo podrá efectuarse si se respeta el orden y presencia de los parametros\e[0m"

}

function mensajesError(){
	echo -e "\e[1;31m::::ERROR::::\e[0m"
	echo -e "\e[1;31m$1\e[0m\n"	
	echo -e "\e[1;31mConsulte la ayuda para mas información\e[0m\n"
	echo -e "\e[1;34mAYUDA: [Ruta del script]/ejercicio5.sh [-h] [-help] [-?]\e[0m"
	echo -e "\e[1;31m:::::::::::::::::::::::::::::::::::::::\e[0m\n\n\n"

}

function mostrarPersonaPais(){
	clear
	awk -v d="$3" -f "./awk/mostrar.awk" "$1" "$2"	
}

function validaExtension(){
	archivo="$1"
	nombre="${archivo##*/}"
	extension="${nombre##*.}"
	if [ $extension != "txt" ]; then
		mensajesError "La extensión del archivo $1 es erronea"
		$EXTENSION="1"
	fi
}


function agregarPais(){
	awk -v p="$2" -f "./awk/paises.awk" "$1"
}
function validarDni(){
	es_numero='^[0-9]+$'
	if ! [[ $1 =~ $es_numero ]] ; then
		mensajesError "El DNI($1) Ingresado no posee el formato correcto"
	else
		VAL_DNI="1"
	fi
}

function validarApellido(){
        es_letra='^([a-zA-Z])+(\ *[a-zA-Z]+)+$'
        if ! [[ $1 =~ $es_letra ]] ; then
                mensajesError "El APELLIDO($1) Ingresado no posee el formato correcto"
        else
                VAL_APELLIDO="1"
        fi
}

function validarNombre(){
        es_letra='^([a-zA-Z])+(\ *[a-zA-Z]+)+$'
	if ! [[ $1 =~ $es_letra ]] ; then
                mensajesError "El NOMBRE($1) Ingresado no posee el formato correcto"
        else
                VAL_NOMBRE="1"
        fi
}


function agregarPersona(){
	agregarPais "$2" "$6"
	validarDni "$3"	
	validarNombre "$4"
	validarApellido "$5"

	if [ $VAL_DNI = "1" ] && [ $VAL_NOMBRE = "1" ] && [ $VAL_APELLIDO = "1" ]; then
		awk -v d="$3" -v n="$4" -v a="$5" -v p="$6" -f "./awk/add.awk" "$1" "$2"
	fi 
}

function borrarPersona(){
	awk -v d="$2" -f "./awk/borrar.awk" "$1" 
}

function validarArchivo(){
	if ! [ -f "$1" ]; then
		mensajesError "El archivo $1 no existe"
		ARCHIVOS="1"
	fi
}

function mostrarListadoPersonas(){
	awk -v p="$3" -f "./awk/listado.awk" "$1" "$2" 
}


if [ $# -ne 1 ] && [ $# -ne 4 ] && [ $# -ne 7 ]; then
	mensajesError "La cantidad de parametros es invalida"	
	CANTIDAD="1"
fi

if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "-?" ]; then
	ayuda
	AYUDA="1"
fi

validarArchivo "$1"
validarArchivo "$2"
if [ $ARCHIVOS = "1" ]; then
		echo "IdPersona;DNI;Apellido_y_Nombre;idPais" > "./personas.txt" 
        echo "idPais;nombre" > "./paises.txt"
        mensajesError "Los Archivos personas.txt y paises.txt \n fueron creados en el directorio de este script"
fi

if [ $ARCHIVOS = "0" ] && ([ ! -r "$1" ] || [ ! -w "$1" ]); then
	mensajesError "El archivo ( $1 ) no posee los permisos adecuados para su utilizacion"
	PERMISOS="1"
fi

if [ $ARCHIVOS = "0" ] && ([ ! -r "$2" ] || [ ! -w "$2" ]); then
	mensajesError "El archivo ( $2 ) no posee los permisos adecuados para su utilizacion"
	PERMISOS="1"
fi

if [ $PERMISOS = "0" ] && [ $ARCHIVOS = "0" ] && [ ! -s "$1" ]; then
	mensajesError "El archivo ( $1 ) esta vacio"
	VACIO="1"
fi

if [ $PERMISOS = "0" ] && [ $ARCHIVOS = "0" ] && [ ! -s "$2" ]; then
	mensajesError "El archivo ( $2 ) esta vacio"
	VACIO="1"
fi

if [ $ARCHIVOS = "0" ] && [ $VACIO = "0" ] && [ $PERMISOS = "0" ] && [ $AYUDA = "0" ] && [ $CANTIDAD = "0" ]; then
	if [ $# -eq 4 ] && [ "$3" = "-d" ];then
		validaExtension "$1"
		validaExtension "$2"
		if [ $ARCHIVOS = "0" ] && [ $EXTENSION = "0" ]; then 
			mostrarPersonaPais "$1" "$2" "$4"	
		fi
		OPERACION="1"
	elif [ "$3" = "-d" ]; then
		mensajesError "La cantidad de parametros para la funcion -d son erroneos"
	fi
	if [ $# -eq 7 ] && [ "$3" = "-a" ]; then
		validaExtension "$1"
        validaExtension "$2"
		if [ $ARCHIVOS = "0" ] && [ $EXTENSION = "0" ]; then 
            agregarPersona "$1" "$2" "$4" "$5" "$6" "$7" 
        fi
		OPERACION="1"
	elif [ $3 = "-a" ]; then
		mensajesError "La cantidad de parametros para la funcion -a son erroneos"
	fi
	if [ $# -eq 4 ] && [ "$3" = "-e" ]; then
        validaExtension "$1"
        validaExtension "$2"
		if [ $ARCHIVOS = "0" ] && [ $EXTENSION = "0" ]; then 
            if ! [ -d "./temp" ]; then
				mkdir "./temp" 
			fi
			borrarPersona "$1" "$4"
			if [ -f "./temp/personas.txt" ]; then
				rm "$1"
				cp  -a "./temp/personas.txt" "$1"
			fi
				rm -r "./temp"
            fi
            OPERACION="1"
	elif [ $3 = "-e" ]; then
        mensajesError "La cantidad de parametros para la funcion -e son erroneos"
	fi
	if [ $# -eq 4 ] && [ $3 = "-p" ]; then
        validaExtension "$1"
        validaExtension "$2"
		if [ $ARCHIVOS = "0" ] && [ $EXTENSION = "0" ]; then 
            mostrarListadoPersonas "$2" "$1" "$4" 
        fi
       	OPERACION="1"
	fi
fi

