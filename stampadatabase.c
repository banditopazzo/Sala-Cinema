#include <sys/socket.h>		/*  socket definitions        */
#include <sys/types.h>		/*  socket types              */
#include <arpa/inet.h>		/*  inet (3) funtions         */


#include <fcntl.h>			//operazioni sui file
#include <stdio.h>			//input-output printf
#include <string.h>			//operazioni sulle stringhe (strcmp)
#include <stdlib.h>			//exit
#include <unistd.h>			//misc unix - esempio close
#include "struttura.h"



int main()
{
	int fd;
	char database[1000];
	strcat(strcpy(database, getenv("HOME")), "/prenotazioni.db");
	if ((fd=open(database, O_RDONLY)) < 0)   
	{
		printf("\n\nErrore di accesso al database\nControllare se effettivamente esiste o se si dispone dei permessi per poter scrivere nella cartella $HOME\n\n");
		exit(1);
	}
	prenotazione pr;
	printf("\n");
	while (read(fd, &pr,sizeof(prenotazione)))
	{
		printf("Prenotazione n. %d\n", pr.codice);
		printf("Fila %c   Posto %d\n\n", pr.p.fila, pr.p.numero);
	}
}
