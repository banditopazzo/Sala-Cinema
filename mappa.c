//MAPPA.C

#include "mappa.h"
#include "struttura.h"

#include <string.h>			//operazioni stringhe
#include <stdlib.h>			//malloc
#include <stdio.h>			//printf e EOF in ottieni mappa
#include <fcntl.h>			//operazioni sui file - open
#include <unistd.h>			//misc unix - esempio close

#define FILEMAPPA "mappa.view"

//---------->>>>>USARE FREE() DOPO L'USO<<<<<---------
char * ottieniMappaDaFile()  //NON USATA
{
    char buff[5000];  
    int letti=0;
    char *mappa=NULL;
    char c;
    int fd;
   
    if ((fd=open(FILEMAPPA, O_RDONLY)) < 0)
    {
		printf("Errore di accesso al file della mappa\n");
		exit(1);
	} 
    while (read(fd, &c, sizeof(char)))
    {
		if (c==EOF) break;
		buff[letti]=c;
        letti++;
    }

    close(fd);
    buff[letti+1]='\0';    
        
        
        
    mappa = (char *) malloc((letti+1)*sizeof(char));
    strcpy(mappa,buff);
    return mappa;
}

int offsetFila(char *mappa, char fila)
{
	int n=0;
	while (n<strlen(mappa))
	{
		if (mappa[n]==fila)
			return n+5;
		n++;
	}
	return -1;
}

void marcaPosto(char *mappa, posto p)
{
	int scalinata=0;
	if (p.numero>5)
		scalinata=8;
	if (p.numero>25)
		scalinata=16;
	int primoCarattere=offsetFila(mappa, p.fila)+(((p.numero-1)*4)+scalinata);
	mappa[primoCarattere]='>';
	mappa[primoCarattere+1]='<';
}

//---------->>>>>USARE FREE() DOPO L'USO<<<<<---------
char * genMappa()
{
	char *mappa=NULL;
	char *buff="\
\n\
\n\
       01 02 03 04 05        06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25        26 27 28 29 30\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   A  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  A\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   B  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  B\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   C  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  C\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   D  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  D\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   E  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  E\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   F  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  F\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
\n\
\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   G  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  G\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   H  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  H\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
   I  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  I\n\
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+\n\
       01 02 03 04 05        06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25        26 27 28 29 30\n\
\n\
\n\
\n\
\n\
                 +----------------------------------------------------------------------------------+\n\
                 |                                   S C H E R M O                                  |\n\
                 +----------------------------------------------------------------------------------+\n\
\n\
\n\
                                   +--+                                 +--+\n\
                                   ¦><¦ OCCUPATO                        ¦  ¦ DISPONIBILE\n\
                                   +--+                                 +--+\n";


	mappa=(char *)malloc(strlen(buff)+1);
	strcpy(mappa,buff);
	return mappa;
}
