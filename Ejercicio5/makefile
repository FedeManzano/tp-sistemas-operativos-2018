all: servidor cliente
servidor:
	gcc -o servidor ./serv/servidor.c -pthread -lrt
cliente:
	gcc -o cliente ./cli/cliente.c -pthread -lrt
clean:
	rm servidor cliente 2> /dev/null
