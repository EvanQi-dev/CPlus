all:recv

client: client.c msock.c
	gcc -o client client.c msock.c

server: server.c msock.c
	gcc -o server server.c msock.c

recv: MulticastRecv.c MulticastClient.c msock.c MulticastMsg.c FecUnpad.c fec.c
	gcc -g -o recv -lpthread MulticastRecv.c MulticastClient.c msock.c MulticastMsg.c FecUnpad.c fec.c

clean:
	rm recv
