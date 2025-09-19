all: servidor cliente
servidor: 
	gcc -o servidor ./appServidor/servidor.c -pthread -lrt
cliente:
	gcc -o cliente ./appCliente/cliente.c -pthread
clean:
	rm servidor cliente 2> /dev/null
recomp: clean all

