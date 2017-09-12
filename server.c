#include <sys/socket.h>		/*  socket definitions        */
#include <sys/types.h>		/*  socket types - sem        */
#include <arpa/inet.h>		/*  inet (3) funtions         */
#include <fcntl.h>				//operazioni sui file - open
#include <stdio.h>				//input-output printf
#include <string.h>				//operazioni sulle stringhe (strcmp)
#include <stdlib.h>				//exit
#include <unistd.h>				//misc unix - esempio close
#include <time.h>					//per time()
#include <sys/ipc.h>			//per semafori
#include <sys/sem.h>			//per semafori
#include <signal.h>				//per segnali
#include <sys/wait.h>			//per wait
#include <errno.h>
#include "struttura.h"

#define MAX_LINE           (1000)

//Variabili globali
char *porta;				//porta server
char database[1000];		//pathname database
int utentiConnessi=0;
int ls_sock; 				//listening socket (qua in globali per chiuderlo da azione segnale)
int conn_sock; 				//connection socket (qua in globali per chiuderlo da azione segnale)

//Prototipi
void analisiArgomenti(int argc, char *argv[]);
void gestioneClient(int *ds_sock, char *client_ip, int semaforo);
void terminazionePadre(int sig);
void terminazioneFiglio(int sig);
void terminazioneGestioneClient(int sig);
void messPipe(int sig);

//Main
int main(int argc, char *argv[])
{
	//Variabili
	struct sockaddr_in my_addr; //server
	struct sockaddr_in cl_addr; //client
	socklen_t sin_size; 		//buffer taglia accept
	int fd; 					//file descriptor per database
	int id_semaforo;
	long chiave_sem=5546543543562;
	struct sigaction signal_terminazionePadre;
	struct sigaction signal_terminazioneFiglio;
	sin_size = sizeof(struct sockaddr_in);

	//SEGNALI PADRE - SIGCHLD e principali segnali di terminazione
	sigfillset(&signal_terminazionePadre.sa_mask);
	signal_terminazionePadre.sa_handler=terminazionePadre;
	signal_terminazionePadre.sa_flags=0;

	sigfillset(&signal_terminazioneFiglio.sa_mask);
	signal_terminazioneFiglio.sa_handler=terminazioneFiglio;
	signal_terminazioneFiglio.sa_flags=SA_RESTART; //per accept

	//routine che chiude i processi figli e evita che rimangano processi <defunct> zombie
	if (sigaction(SIGCHLD, &signal_terminazioneFiglio, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	//gestione principali segnali di terminazione
	if (sigaction(SIGINT, &signal_terminazionePadre, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGHUP, &signal_terminazionePadre, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGTERM, &signal_terminazionePadre, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGQUIT, &signal_terminazionePadre, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGABRT, &signal_terminazionePadre, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}

	//POSIZIONE DATABASE
	memset(database, 0, sizeof(database));
	strcat(strcpy(database, getenv("HOME")), "/prenotazioni.db");
	//---Start---

	//CONTROLLO DATABASE PRENOTAZIONI
	if ((fd=open(database, O_RDONLY | O_CREAT, 0666)) < 0)
	{
		printf("\nErrore di accesso al database\nControllare se si dispone dei permessi per poter scrivere nella cartella $HOME\n\n");
		exit(1);
	}
	close(fd);

	//Semaforo
	id_semaforo=semget(chiave_sem, 1, IPC_CREAT|IPC_EXCL |0666);
	if( id_semaforo == -1 )
	{
		id_semaforo = semget(chiave_sem, 1, IPC_CREAT|0666);
		semctl(id_semaforo,IPC_RMID,0);
		id_semaforo = semget(chiave_sem, 1, IPC_CREAT|IPC_EXCL|0666);
		if( id_semaforo == -1 )
		{
			printf("Errore nella creazione del semaforo\n");
			exit(1);
		}
	}

	if (semctl(id_semaforo,0,SETVAL,1) == -1 )
	{
		printf("Errore nell settaggio del semaforo\n");
		exit(1);
	}

	//Lettura Comandi
	analisiArgomenti(argc, argv);

	//Socket
	if ((ls_sock=socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Errore nella creazione del socket\n");
		exit(1);
	}

	//Indirizzo
	memset(&my_addr, 0, sizeof(my_addr)); //Settaggio tutti i byte a zero
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(porta));
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//Bind
	if (bind(ls_sock, (struct sockaddr *)&my_addr, sizeof(my_addr))<0)
	{
		printf("Errore nella bind\n");
		exit(1);
	}

	//Listen
	if (listen(ls_sock, 1024)<0)
	{
		printf("Errore nella listen\n");
		exit(1);
	}

	printf("Server avviato sulla porta: %s\n", porta);
	printf("Per fermare il server premere Control+C\n");

	//Ciclo infinito per accettazione connessioni
	while (1)
	{
		if ((conn_sock=accept(ls_sock, ((struct sockaddr *)&cl_addr), &sin_size)) < 0)
		{
			printf("Errore accettazione connessione\n");
			close(conn_sock);
			continue;
		}

		//Figlio - Gestione Client
		if (fork()==0)
		{
			close(ls_sock);
			printf("Connessione da %s\n", inet_ntoa(cl_addr.sin_addr));

			gestioneClient(&conn_sock, inet_ntoa(cl_addr.sin_addr), id_semaforo);

			close(conn_sock);
			printf("Connessione da %s chiusa\n", inet_ntoa(cl_addr.sin_addr));
			exit(0);
		}

		//Padre - Chiusura socket connesso
		utentiConnessi++;
		printf("Utenti connessi: %d\n", utentiConnessi);
		if (close(conn_sock) < 0)
		{
			printf("Errore durante la chiusura del socket\n");
			printf("Il processo principale sta per essere terminato\n");
			terminazionePadre(SIGTERM);
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
			if (!strncmp(argv[n], "-h", 2))
			{
				printf("Sintassi:\n\n");
				printf("    server -p (porta) [-h].\n\n");
				exit(0);
			}
		n++;
	}
	if (argc!=3)
	{
		printf("Sintassi:\n\n");
		printf("    server -p (porta) [-h].\n\n");
	    exit(1);
	}
}

void gestioneClient(int *ds_sock, char* client_ip, int semaforo)
{
	int fd; //file descriptor
	int numeroPosti;
	int i; //contatore
	prenotazione pr; //per cicli
	posto p; //per cicli
	posto *lista;
	char buff[MAX_LINE];
	int codice;
	prenotazione *listaPr;
	int numeroPrenotazioni;
	int nonValido;
	int trovato;
	int responso;
	int opzione;
	struct sembuf accesso;
	struct sembuf uscita;
	sigset_t allmask, maskpipe;
	struct sigaction signal_terminazioneGestioneClient, signal_pipe;

	sigfillset(&allmask);
	sigemptyset(&maskpipe);
	sigaddset(&maskpipe, SIGPIPE);

	sigfillset(&signal_terminazioneGestioneClient.sa_mask);
	signal_terminazioneGestioneClient.sa_handler=terminazioneGestioneClient;
	signal_terminazioneGestioneClient.sa_flags=0;

	sigfillset(&signal_pipe.sa_mask);
	signal_pipe.sa_handler=messPipe;
	signal_pipe.sa_flags=0;

	//SEGNALI FIGLIO - Principali Segnali di terminazione e SIGPIPE
	if (sigaction(SIGINT, &signal_terminazioneGestioneClient, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGHUP, &signal_terminazioneGestioneClient, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGTERM, &signal_terminazioneGestioneClient, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGQUIT, &signal_terminazioneGestioneClient, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGABRT, &signal_terminazioneGestioneClient, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}
	if (sigaction(SIGPIPE, &signal_pipe, NULL) == -1) {printf("Errore sigaction\n"); exit(1);}

	//OPERAZIONI SEMAFORO
	accesso.sem_num=0;
	accesso.sem_op=-1;	//decremena 1, bloccante
	accesso.sem_flg=0;
	uscita.sem_num=0;
	uscita.sem_op=1;	//aumenta 1
	uscita.sem_flg=0;
	//semop(semaforo, &accesso, 1);
	//semop(semaforo, &uscita, 1);

	//SELEZIONE OPZIONE
	while (1)
	{
		//AZZERAMENTO VARIABILI
		opzione=0;
		numeroPosti=0;
		numeroPrenotazioni=0;
		nonValido=0;
		trovato=0;
		responso=0;
		codice=0;
		memset(buff, 0, sizeof(buff));
		memset(&p, 0, sizeof(posto));
		memset(&pr, 0, sizeof(prenotazione));
		lista=NULL;		//Deve essere nullo per realloc
		listaPr=NULL; 	//Deve essere nullo per realloc

		printf("Attesa Opzione da %s\n", client_ip);
		readIntero(*ds_sock, &opzione); //RICEZIONE SELEZIONE

		//printf("Attesa opzione");
		switch (opzione)
		{
			case 1: //MOSTRA MAPPA
				printf("Richiesta mappa da %s\n", client_ip);

				//SEGNALI per Sez.Critica
				sigprocmask(SIG_BLOCK, &allmask, NULL);

				//CRITICA - APERTURA FILE
				semop(semaforo, &accesso, 1);
				if ((fd=open(database, O_RDONLY)) < 0)
				{
					semop(semaforo, &uscita, 1); //RIPRISTINO SEMAFORO PRIMA DI CHIUDERE
					printf("Errore apertura database da parte di %d. Chiusura processo...\n", getpid());
					close(*ds_sock);
					exit(1);
				}

				//CONTA E MEMORIZZA PRENOTAZIONI/POSTI
				while(read(fd, &pr, sizeof(prenotazione)))
				{
					numeroPosti++;
					lista=(posto*)realloc(lista, (numeroPosti*sizeof(posto)));
					lista[numeroPosti-1]=pr.p;
				}

				//CRITICA - CHIUSURA FILE
				close(fd);
				semop(semaforo, &uscita, 1);
				//SEGNALI sblocco per Sez.Critica
				sigprocmask(SIG_UNBLOCK, &allmask, NULL);

				responso=333;
				writeIntero(*ds_sock, &responso);
				writeIntero(*ds_sock, &numeroPosti);
				writeListaPosti(*ds_sock, lista, numeroPosti);

				free(lista);
				printf("Mappa inviata a %s\n", client_ip);
				break;
			case 2: //PRENOTA
				readIntero(*ds_sock, &numeroPosti);
				lista=(posto*)malloc(numeroPosti*sizeof(posto));
				readListaPosti(*ds_sock, lista, numeroPosti);		//RICEZIONE LISTA POSTI
				printf("Numero posti ricevuti da %s: %d\n", client_ip, numeroPosti);


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
					printf("Nessuna modifica, qualche posto non valido\n");
					responso=555;
					writeIntero(*ds_sock, &responso);
					free(lista);
					break;
				}

				//SEGNALI per Sez.Critica
				sigprocmask(SIG_BLOCK, &allmask, NULL);
				//CRITICA - APERTURA FILE
				semop(semaforo, &accesso, 1);
				if ((fd=open(database, O_RDONLY)) < 0)
				{
					semop(semaforo, &uscita, 1); //RIPRISTINO SEMAFORO PRIMA DI CHIUDERE
					printf("Errore apertura database da parte di %d. Chiusura processo...\n", getpid());
					close(*ds_sock);
					exit(1);
				}

				//CONTA E MEMORIZZA PRENOTAZIONI
				while (read(fd, &pr, sizeof(prenotazione)))
				{
					numeroPrenotazioni++;
					listaPr=(prenotazione*)realloc(listaPr, (numeroPrenotazioni*sizeof(prenotazione)));
					listaPr[numeroPrenotazioni-1]=pr;
				}

				//CONTROLLO SE QUALCHE POSTO E' OCCUPATO
				for (i=0; i<numeroPosti; i++)
				{
					if (isPostoInListaPr(lista[i], listaPr, numeroPrenotazioni))
					{
						trovato=1;
						break;
					}
				}

				//SCEGLI SE AGGIORNARE DATABASE
				if (trovato)
				{
					printf("Nessuna modifica, qualche posto è occupato\n");
				}
				else
				{
					codice=(getpid() | time(NULL) | rand());
					//PASSAGGIO DA LETTURA A SCRITTURA-APPEND
					close(fd);
					fd=open(database, O_WRONLY | O_APPEND);
					for (i=0; i<numeroPosti; i++)
					{
						pr.codice=codice;
						pr.p=lista[i];
						write(fd, &pr, sizeof(prenotazione));
					}
					printf("Elenco prenotazioni aggiornato\n");
				}

				//CRITICA - CHIUSURA FILE
				close(fd);
				semop(semaforo, &uscita, 1);

				//SBLOCCAGGIO solo SIGPIPE
				sigprocmask(SIG_UNBLOCK, &maskpipe, NULL);

				//INVIO RESPONSO/CODICE
				if (trovato)
				{
					responso=999;
					writeIntero(*ds_sock, &responso);
				}
				else
				{
					responso=333;
					writeIntero(*ds_sock, &responso);
					writeIntero(*ds_sock, &codice); //INVIO CODICE DI PRENOTAZIONE
				}

				//SEGNALI sblocco per Sez.Critica e codice prenotazione
				sigprocmask(SIG_UNBLOCK, &allmask, NULL);

				free(lista);
				free(listaPr);
				break;
			case 3: //DISDICI
				readIntero(*ds_sock, &codice); //Ricezione codice prenotazione

				//SEGNALI per Sez.Critica
				sigprocmask(SIG_BLOCK, &allmask, NULL);
				//CRITICA - APERTURA FILE
				semop(semaforo, &accesso, 1);
				if ((fd=open(database, O_RDONLY)) < 0)
				{
					semop(semaforo, &uscita, 1); //RIPRISTINO SEMAFORO PRIMA DI CHIUDERE
					printf("Errore apertura database da parte di %d. Chiusura processo...\n", getpid());
					close(*ds_sock);
					exit(1);
				}

				//CONTA E MEMORIZZA PRENOTAZIONI
				while (read(fd, &pr, sizeof(prenotazione)))
				{
					numeroPrenotazioni++;
					listaPr=(prenotazione*)realloc(listaPr, (numeroPrenotazioni*sizeof(prenotazione)));
					listaPr[numeroPrenotazioni-1]=pr;
				}

				//CONTROLLA SE ESISTE UNA PRENOTAZIONE CON IL CODICE RICEVUTO
				for (i=0; i<numeroPrenotazioni; i++)
				{
					if (listaPr[i].codice == codice)
					{
						trovato=1;
						break;
					}
				}

				//SCEGLI SE MODIFICARE DATABASE
				if (trovato)
				{
					//PASSAGGIO DA LETTURA A SOVRASCRITTURA
					close(fd);
					fd=open(database, O_WRONLY | O_TRUNC);
					for (i=0; i<numeroPrenotazioni; i++)
					{
						if (listaPr[i].codice != codice)
							write(fd, &listaPr[i], sizeof(prenotazione));
					}
					printf("Elenco prenotazioni aggiornato\n");
				}
				else
				{
					printf("Nessuna modifica, prenotazione non trovata\n");
				}

				//CRITICA - CHIUSURA FILE
				close(fd);
				semop(semaforo, &uscita, 1);

				//SBLOCCAGGIO solo SIGPIPE
				sigprocmask(SIG_UNBLOCK, &maskpipe, NULL);

				//INVIA RESPONSO
				if (trovato)
				{
					responso=333;
					writeIntero(*ds_sock, &responso);
				}
				else
				{
					responso=999;
					writeIntero(*ds_sock, &responso);
				}
				//SEGNALI sblocco per Sez.Critica e risposta al client
				sigprocmask(SIG_UNBLOCK, &allmask, NULL);

				free(listaPr);
				break;
			case 4: //ESCI
				return;
				break;
			default:
				printf("Opzione non riconosciuta da %s . Il client in questione verrà disconnesso e il processo relativo terminato.\n", client_ip);
				close(*ds_sock);
				exit(0);
		}
	}
}

//GESTIONE SEGNALI per PADRE
void terminazionePadre(int sig)
{
	close(ls_sock);  //Blocca nuovi tentativi di connessione
	close(conn_sock);
	printf("\nProcesso principale in chiusura. Attesa terminazione processi figli\n");
	printf("Utenti connessi: %d\n", utentiConnessi);
	while (utentiConnessi!=0)
	{
		wait(0);
		utentiConnessi--;
		printf("Utenti connessi: %d\n", utentiConnessi);
	}
	printf("Chiusura di tutti i processi terminata\nServer terminato correttamente\n");
	exit(0);
}

void terminazioneFiglio(int sig)
{
  wait(NULL);
  utentiConnessi--;
  printf("Utenti connessi: %d\n", utentiConnessi);
}

//GESTIONE SEGNALI per FIGLIO
void terminazioneGestioneClient(int sig)
{

	printf("\nTerminazione processo gestione client\n");
	exit(0);
}

void messPipe(int sig)
{
	printf("Errore durante la comunicazione con il client. Il processo in questione verrà terminato\n");
	exit(1);
}
