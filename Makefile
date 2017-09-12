SRSRC = server.c
CLSRC = client.c
CC = gcc

all: serverSD clientSD stampadatabaseSD

serverSD: server.o struttura.o
	$(CC) -o serverSD server.o struttura.o

clientSD: client.o mappa.o struttura.o
	$(CC) -o clientSD client.o mappa.o struttura.o

stampadatabaseSD: stampadatabase.o struttura.o
	$(CC) -o stampadatabaseSD stampadatabase.o struttura.o

server.o: $(SRSRC) struttura.h
	$(CC) -c -o server.o $(SRSRC)

client.o: $(CLSRC) mappa.h struttura.h
	$(CC) -c -o client.o $(CLSRC)

mappa.o: mappa.c mappa.h
	$(CC) -o mappa.o mappa.c -c

struttura.o: struttura.c struttura.h
	$(CC) -o struttura.o struttura.c -c

stampadatabase.o: stampadatabase.c struttura.h
	$(CC) -o stampadatabase.o stampadatabase.c -c

clean:
	rm -f *.o
	rm -f serverSD
	rm -f clientSD
	rm -f stampadatabaseSD

deletedatabase:
	rm -f ~/prenotazioni.db

install: install_client install_server install_stampadatabase

install_client:
	install -m755 clientSD /usr/bin/clientSD

install_server:
	install -m755 serverSD /usr/bin/serverSD

install_stampadatabase:
	install -m755 stampadatabaseSD /usr/bin/stampadatabaseSD

uninstall:
	rm -f /usr/bin/clientSD
	rm -f /usr/bin/serverSD
	rm -f /usr/bin/stampadatabaseSD
