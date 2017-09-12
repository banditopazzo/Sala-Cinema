//STRUTTURA.H

#include <unistd.h> //per ssize_t

#ifndef STRUTTURA_HELP
#define STRUTTURA_HELP

typedef struct {
	char fila;
	int numero;
} posto;

typedef struct {
	int codice;
	posto p;
} prenotazione;

int isNumeroValido(int numero);
int isFilaValida(char *fila);
int isPostoValido(posto * p);
int isPostoInLista(posto p, posto *lista, int DIM);
int isPostoInListaPr(posto p, prenotazione *listaPr, int DIM);

ssize_t readIntero(int ds, int * numero);
ssize_t writeIntero(int ds, int * numero);

ssize_t readListaPosti(int ds, posto * lista, int numposti);
ssize_t writeListaPosti(int ds, posto * lista, int numposti);

#endif
