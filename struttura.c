//STRUTTURA.C

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "struttura.h"

int isPostoValido(posto * p)
{
	if (isNumeroValido(p->numero) && isFilaValida(&p->fila))
		return 1;
	else
		return 0;
}
	

int isNumeroValido(int numero)
{
	if (numero>=1 && numero<=30)
		return 1;
	else
		return 0;
}

int isFilaValida(char *fila)
{
	if (*fila>='A' && *fila<='I')
		return 1;
	if (*fila>='a' && *fila<='i')
	{
		*fila=*fila-32;
		return 1;
	}
	return 0;
}

int isPostoInListaPr(posto p, prenotazione *listaPr, int DIM)
{
	int i;
	for (i=0; i<DIM; i++)
	{
		if ((listaPr[i].p.fila == p.fila) && (listaPr[i].p.numero == p.numero))
			return 1;
	}
	return 0;
}

int isPostoInLista(posto p, posto *lista, int DIM)
{
	int i;
	for (i=0; i<DIM; i++)
	{
		if ((lista[i].fila == p.fila) && (lista[i].numero == p.numero))
			return 1;
	}
	return 0;
}

ssize_t writeIntero(int ds, int * numero)
{
	uint32_t tmp;
	tmp=htonl(*numero);
	return write(ds, &tmp, sizeof(uint32_t));
}

ssize_t readIntero(int ds, int * numero)
{
	uint32_t tmp;
	ssize_t ret;
	ret = read(ds, &tmp, sizeof(uint32_t));
	*numero=ntohl(tmp);
	return ret;
}

ssize_t writeListaPosti(int ds, posto * lista, int numposti)
{
	ssize_t tot=0;
	int i;
	for (i=0; i<numposti; i++)
	{
		tot=tot + write(ds, &lista[i].fila, 1);
		tot=tot + writeIntero(ds, &lista[i].numero);
	}
	return tot;
}

ssize_t readListaPosti(int ds, posto * lista, int numposti)
{
	ssize_t tot=0;
	int i;
	for (i=0; i<numposti; i++)
	{
		tot=tot + read(ds, &lista[i].fila, 1);
		tot=tot + readIntero(ds, &lista[i].numero);
	}
	return tot;
}
	





