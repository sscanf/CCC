

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_MODL.H
*
*  PURPOSE:		CCIR Modem Link Layer
*
*  REMARKS:		
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		11.3.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#define SIGNAL_MODEM_SEND_OK 		0x01
#define SIGNAL_MODEM_SEND_FAIL 		0x02	

#define SIGNAL_MODEM_ENABLE		0x04
#define SIGNAL_MODEM_DISABLE		0x08

/*---------------------------------------------------------------------------*/


#define T5		2		/* modem response timeout */

/*---------------------------------------------------------------------------*/













/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


void send_modem_message( FILE *fp, BOOLEAN new );

void send_modem_signal( int s );
BOOLEAN receive_modem_signal( int s );


/*---------------------------------------------------------------------------*/


void PROCESS_modem_link( void );
void MOD_init( void );

/*---------------------------------------------------------------------------*/




