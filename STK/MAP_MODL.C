

//#pragma warn -par


/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*
*  FILE:		MAP_MODL.C
*
*  PURPOSE:		CCIR Modem Link Layer
*
*  REMARKS:		*** STUB ***
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		22.5.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#include "cgc.h"
#include "map27.h"
#include "map_modl.h"
#include "map_mod.h"
#include "map_tim.h"

extern tareas ptrtareas;
extern void TxCGC (unsigned char flota, unsigned char IndiH, unsigned char IndiL, unsigned char mensaje);

/*---------------------------------------------------------------------------*/
/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


/*void send_modem_message( FILE *fp, BOOLEAN new )
{
}
*/

void send_modem_signal( int s )
{
}


BOOLEAN receive_modem_signal( int s )
{
	return (FALSE);
}


/*---------------------------------------------------------------------------*/












/******************************************************************************
*
*	*** SERVICE PROVIDER ***
*
******************************************************************************/


void PROCESS_modem_link( void )
{
}


/*---------------------------------------------------------------------------*/


void MOD_init( void )
{
}

/*---------------------------------------------------------------------------*/


//#pragma warn .par





