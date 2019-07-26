

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_SYS.C
*
*  PURPOSE:		MAP27 System
*
*  REMARKS:		
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		21.4.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#include <stdio.h>

#include "map27.h"
#include "map_sys.h"
#include "map_net.h"
#include "map_nets.h"
#include "map_dl.h"
#include "map_rs.h"
#include "map_modl.h"
#include "map_mod.h"
#include "map_tim.h"


/*---------------------------------------------------------------------------*/



void MAP_scheduler( void )		/* SDL Scheduler */
{
	/* MAP27 */

	PROCESS_data_link();
	PROCESS_network();

	PROCESS_modem_link();
}


/*---------------------------------------------------------------------------*/


void MAP_init( COM_CONFIG *cc  )	/* Initialize system */
{
	RS_init( cc->MAP27_port_address );
	timer_init();
	modem_init( cc->modem_port_address );
	MOD_init();
	DL_init();
	NET_init();
}


/*---------------------------------------------------------------------------*/



void MAP_restore( void )		/* Restore hardware */
{
	RS_restore();
	timer_restore();
	modem_restore();
}


/*---------------------------------------------------------------------------*/



