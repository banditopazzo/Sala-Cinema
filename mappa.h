//MAPPA.H

#include "struttura.h"

#ifndef MAPPA_HELP
#define MAPPA_HELP

char * ottieniMappaDaFile(); //NON USATA

int offsetFila(char *mappa, char fila);
void marcaPosto(char *mappa, posto p);

char * genMappa();

#endif
