all: master whoClient whoServer

master: master.c
	gcc -g -o master master.c fifo_func.c worker.c HashBuck.c HashTable.c bt.c func.c Heap.c

whoClient: whoClient.c
	gcc -pthread -g -o whoClient whoClient.c

whoServer: whoServer.c
		gcc -pthread -g -o whoServer whoServer.c

HashBuck.o: HashBuck.c
	gcc -c HashBuck.c func.c bt.c

func.o: func.c
	gcc -c func.c Heap.c HashBuck.c HashTable.c

bt.o: bt.c
	gcc -c bt.c func.c

Heap.o: Heap.c
	gcc -c Heap.c

HasTable.o: HashTable.c
	gcc -c HashTable.c func.c

fifo_func.0: fifo_func.c
	gcc -c fifo_func.c

clean:
	rm -f master whoClient whoServer
	rm -f *.o
