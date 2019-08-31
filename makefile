FILES = server.c client.c

test: $(FILES)
	cc -g -c -o server.o server.c
	cc -g -c -o client.o client.c
	cc -o client client.o
	cc -o server server.o

build:
	cc -g -c -o server.o server.c
	cc -g -c -o client.o client.c
	cc -o client client.o
	cc -o server server.o
	

serv:
	./server.exe

cli:
	./client.exe 127.0.0.1