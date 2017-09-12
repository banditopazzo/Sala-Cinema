# Progetto per corso di Sistemi Operativi - Università degli Studi di Roma "La Sapienza"
### Sistema di prenotazione di una sala cinema scritto in C per UNIX, Linux, macOS.

Il sistema composto da server e client, con funzioni di:
* visualizzazione dei posti liberi
* prenotazione di uno o più posti
* cancellazione prenotazione possedendo un codice di prenotazione

## Caratteristiche

* ***Server Multiprocesso.***
Il sistema è stato realizzato con un server multiprocesso, ovvero quando un client si connette
il server genera un processo figlio che si occupa di gestire la connessione.

* ***Semaforo per accesso al database.***
E’ stato necessario quindi l’uso di un semaforo per la sincronizzazione dell’accesso al
database ed è stato usato un semaforo di tipo IPC Unix System V. Quando un processo che si
occupa di gestire un client si trova nella sua sezione critica (accesso al database), gli altri
processi che tentano di entrare nelle loro sezioni critiche vengono bloccati e fatti ripartire
solo quando il primo termina la sua sezione critica.

* ***Comunicazione client/server.***
La comunicazione tra client e server avviene tramite sockets nel dominio AF_INET in
modalità connessione. I numeri interi vengono convertiti in uint32_t prima di essere inviati e
ripristinati all’arrivo.

* ***Gestione dei principali segnali di terminazione, SIGPIPE e SIGCHLD.***
Quando il server viene chiuso, il processo principale attende la chiusura dei processi figli per
poi terminare anch’esso. Se i processi figli ricevono il segnale di terminazione mentre si
trovano nella propria sezione critica, la concludono prima di terminare. Il client o i processi
figli sul server vengono interrotti all’arrivo di un segnale SIGPIPE, notificando l’accaduto.
Quando un processo figlio termina, invia un segnale SIGCHLD al padre che lo gestisce e fa
terminare completamente il figlio evitando che nel sistema rimangano processi zombie. La
gestione dei segnali avviene in modo affidabile tramite la struttura sigaction.

* ***Strutture dati.*** I posti sono rappresentati con una struct formata da un carattere ad indicare
la fila e un intero per il numero di poltrona. Le prenotazioni sono rappresentate con una struct formata
da un elemento di tipo struct posto e un intero per il codice di prenotazione.

* ***Database.***
Il database si trova in nella cartella puntata dalla variabile d’ambiente $HOME ed è
realizzato come un insieme di prenotazioni. Possono esserci più prenotazioni con lo stesso
codice identificativo, ma solo una prenotazione per posto. La lettura o scrittura del database
avviene tramite le primitive a basso livello e la lettura avviene in RAM dinamica.

## Mappa dei posti disponibili

```
       01 02 03 04 05        06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25        26 27 28 29 30
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   A  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  A
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   B  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  B
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   C  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  C
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   D  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  D
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   E  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  E
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   F  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  F
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+


      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   G  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  G
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   H  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  H
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
   I  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦  ¦      ¦  ¦  ¦  ¦  ¦  ¦  I
      +--+--+--+--+--+      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+      +--+--+--+--+--+
       01 02 03 04 05        06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25        26 27 28 29 30




                 +----------------------------------------------------------------------------------+
                 |                                   S C H E R M O                                  |
                 +----------------------------------------------------------------------------------+


                                   +--+                                 +--+
                                   ¦><¦ OCCUPATO                        ¦  ¦ DISPONIBILE
                                   +--+                                 +--+
```


## Come compilare e installare

* Compilare:

  `$ make`

* Installare (necessari privilegi di amministratore):

  `# make install`

* Disinstallare (necessari privilegi di amministratore):

  `# make uninstall`

* Rimuovere programmi compilati:

  `$ make clean`

* Eliminare il database delle prenotazioni:

  `$ make deletedatabase`


## Come eseguire

* Avviare il server:

  `$ serverSD -p porta`

* Avviare il client e connetterlo al server:

  `$ clientSD -a indirizzo -p porta`

* Stampare il contenuto del database sul terminale:

  `$ stampadatabaseSD`
