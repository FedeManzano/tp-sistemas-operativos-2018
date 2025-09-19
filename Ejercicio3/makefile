all: camaras estac
camaras: 
	gcc -o camaras ./archivosc/camaras.c -pthread
estac:
	gcc -o estac ./archivosc/estac.c -pthread
clean:
	rm camaras estac 2> /dev/null
re: clean all
