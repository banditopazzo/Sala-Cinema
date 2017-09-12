#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <netdb.h>						//gethostbyname, ecc
#include <stdio.h>						//input-output printf
#include <string.h>						//operazioni sulle stringhe (strcmp)
#include <stdlib.h>						//exit
#include <unistd.h>						//misc unix - esempio close
#include <signal.h>						//per segnali

#include "struttura.h"
#include "mappa.h"

#define DIM           (1000)

//Variabili Globali
char *porta; //porta server
char *indirizzo; //indirizzo server

//Prototipi
void analisiArgomenti(int argc, char *argv[]);
void messaggioTerminazione(int sig);
void messPipe(int sig);

int main(int argc, char *argv[])
{
	//Variabili
	int conn_sock;					//socket connessione
	struct sockaddr_in serv_addr;	//indirizzo server
	struct hostent *hp;				//per risoluzione indirizzo
	char buff[DIM];					//buffer per lettura
	int opzione;					//opzione MENU
	int numeroPosti;
	posto p;						//per cicli
	posto *lista;					//lista posti dinamica
	int i;							//contatore
	char *map;						//contenitore mappa
	int codice;						//codice prenotazione
	int nonValido;
	int responso;					//responso server
	hp=NULL;						//nullo per gethostbyname
	lista=NULL;						//nullo per realloc
	struct sigaction signal_terminazione, signal_pipe;

	sigfillset(&signal_terminazione.sa_mask);
	signal_terminazione.sa_handler=messaggioTerminazione;
	signal_terminazione.sa_flags=0;

	sigfillset(&signal_pipe.sa_mask);
	signal_pipe.sa_handler=messPipe;
	signal_pipe.sa_flags=0;

	//SEGNALI - Principali Segnali di terminazione e SIGPIPE
	if (sigaction(SIGINT, &signal_terminazione, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGHUP, &signal_terminazione, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGTERM, &signal_terminazione, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGQUIT, &signal_terminazione, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGABRT, &signal_terminazione, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGPIPE, &signal_pipe, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}

	//Start
	analisiArgomenti(argc, argv);
	printf("Connessione al server %s sulla porta %s\n", indirizzo, porta);

	//Socket
	if ((conn_sock=socket(AF_INET, SOCK_STREAM,0)) < 0)
	{
		printf("Errore nella creazione del socket\n");
		exit(1);
	}

	//Indirizzo
	memset(&serv_addr, 0, sizeof(serv_addr)); //Settaggio tutti i byte a zero
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(porta));
	if ( inet_aton(indirizzo, &serv_addr.sin_addr) <= 0 ) //IP o risoluzione
	{
		printf("client: indirizzo IP non valido.\nclient: risoluzione nome...");

		if ((hp=gethostbyname(indirizzo)) == NULL)
		{
			printf("fallita.\n");
  			exit(1);
		}
		printf("riuscita.\n");
		serv_addr.sin_addr = *((struct in_addr *)hp->h_addr);
	}
	printf("indirizzo server: %s\n", inet_ntoa(serv_addr.sin_addr));

	//Connessione
	if (connect(conn_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Errore durante la connessione\n");
		exit(1);
	}


    printf("\n\nBENVENUTO");
	//CICLO MENU
	while(1)
	{
		printf("\n\nSeleziona l'operazione\n\n");
		printf("1 - Mostra mappa dei posti disponibili\n");
		printf("2 - Effettua una prenotazione\n");
		printf("3 - Disdici una prenotazione conoscendo il codice\n");
		printf("4 - Esci dal programma\n\n");
		printf("Inserisci il numero corrispondente all'operazione scelta:  ");

		//AZZERAMENTO VARIABILI
		numeroPosti=0;
		opzione=0;
		responso=0;
		codice=0;
		nonValido=0;
		lista=NULL;
		memset(&p,0, sizeof(posto));
		memset(buff, 0, sizeof(buff));

		//LETTURA OPZIONE
		fgets(buff, DIM, stdin);
		buff[strlen(buff)-1]=0;
		opzione=atoi(buff);


		//SWITCH SU OPZIONE
		switch (opzione)
		{
			case 1: //MOSTRA MAPPA
				map=genMappa();
				writeIntero(conn_sock, &opzione);  //INVIO SELEZIONE
				readIntero(conn_sock, &responso);
				if (responso != 333)
					{
						printf("\n******Errore durante la ricezione dei dati. Riprovare...******\n\n");
						break;
					}
				readIntero(conn_sock, &numeroPosti);
				lista=(posto*)malloc(numeroPosti*sizeof(posto));
				readListaPosti(conn_sock, lista, numeroPosti);

				//CONTROLLLO VALIDITA
				for (i=0; i<numeroPosti; i++)
				{
					if (!isPostoValido(&lista[i]))
					{
						nonValido=1;
						break;
					}
				}
				if (nonValido)
				{
					printf("\n******Errore nei dati ricevuti******\n");
					free(lista);
					break;
				}
				printf("\n******Ricezione dati sala completata******\n");
				for (i=0; i<numeroPosti; i++)
				{
					marcaPosto(map, lista[i]);
				}
				printf("%s", map);
				free(lista);
				free(map);
				break;
			case 2: //PRENOTA
				writeIntero(conn_sock, &opzione);  //INVIO SELEZIONE
				//LEGGI DA TASTIERA POSTI E CREA LISTA
				printf("Inserire il posto che si desidera prenotare\n");
				while (1)
				{
					memset(buff, 0, sizeof(buff));
					fgets(buff, DIM, stdin);
					buff[strlen(buff)-1]=0;
					if (strcmp(buff, "prenota") ==0 || strcmp(buff, "PRENOTA") ==0)
					{
						if (lista != NULL) break; //TEST LISTA VUOTA
						else
						{
							printf("La lista è vuota, inserire almeno un posto che si desidera prenotare\n");
							continue;
						}
					}
					p.fila=buff[0];
					p.numero=atoi(buff+1);
					if (!isPostoValido(&p)) //TESTA VALIDITA' POSTO
					{
						printf("Posto non riconosciuto, inserire un altro posto o digitare PRENOTA per inviare\n");
						continue;
					}
					if (isPostoInLista(p, lista, numeroPosti)) //TEST DOPPIONE
					{
						printf("Hai già selezionato questo posto, inserire un altro posto o digitare PRENOTA per inviare\n");
						continue;
					}
					numeroPosti++;
					lista=(posto *)realloc(lista, (numeroPosti*sizeof(posto)));
					lista[numeroPosti-1]=p;
					printf("Aggiungere un altro posto o digitare PRENOTA per inviare\n");
				}

				//INVIA LISTA
				writeIntero(conn_sock, &numeroPosti);
				writeListaPosti(conn_sock, lista, numeroPosti);
				free(lista);
				readIntero(conn_sock, &responso);
				switch (responso)
				{
					case 333:
						readIntero(conn_sock, &responso);
						printf("\n******Prenotazione effettuata.******\n******Codice prenotazione: %d ******\n", responso);
						break;
					case 999:
						printf("\n******Qualche posto è gia occupato. Riprovare...******\n");
						break;
					case 555:
						printf("\n******Qualche posto indicato non era valido. Riprovare...******\n");
						break;
					default:
						printf("\n******Errore durante l'operazione. Riprovare...******\n");
						break;
				}
				break;
			case 3: //DISDICI
				writeIntero(conn_sock, &opzione);  //INVIO SELEZIONE

				//LETTURA CODICE
				printf("\nInserire il codice della prenotazione che si vuole annullare:   ");
				fgets(buff, DIM, stdin);
				buff[strlen(buff)]=0;
				codice=atoi(buff);

				writeIntero(conn_sock, &codice); //INVIO CODICE
				readIntero(conn_sock, &responso); //LETTURA RESPONSO
				switch (responso)
				{
					case 333:
						printf("\n******Prenotazione annullata******\n");
						break;
					case 999:
						printf("\n******Nessuna prenotazione corrispondente trovata******\n");
						break;
					default:
						printf("\n******Errore durante l'operazione. Riprovare...\n******");
						break;
				}
				break;
			case 4: //ESCI
				writeIntero(conn_sock, &opzione);  //INVIO SELEZIONE
				close(conn_sock);
				exit(0);
			default:
				printf("Opzione non riconosciuta. Riprovare...");
				break;
		}
	}
	exit(0);
}

void analisiArgomenti(int argc, char *argv[])
{
	int n=1;
	while (n<argc)
	{
		if (!strncmp(argv[n], "-p", 2))
			porta = argv[n+1];
		else
			if (!strncmp(argv[n], "-a", 2))
				indirizzo = argv[n+1];
			else
				if (!strncmp(argv[n], "-h", 2))
			{
				printf("Sintassi:\n\n");
				printf("    client -a (indirizzo remoto) -p (porta remota) [-h].\n\n");
				exit(0);
			}

		n++;
	}
	if (argc!=5)
	{
		printf("Sintassi:\n\n");
		printf("    client -a (indirizzo remoto) -p (porta remota) [-h].\n\n");
	    exit(1);
	}
}

void messPipe(int sig)
{
	printf("\nErrore durante la comunicazione con il server. Il processo è stato terminato\n");
	exit(1);
}

void messaggioTerminazione(int sig)
{
	printf("\nProcesso terminato a causa di un segnale di terminazione\n");
	exit(0);
}
