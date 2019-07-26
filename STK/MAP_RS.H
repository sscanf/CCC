

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_RS.H
*
*  PURPOSE:		MAP27 RS Link Driver
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



/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/

	
void send_RS_packet( OCTET *header, OCTET *data, int data_len );
BOOLEAN receive_RS_packet( OCTET *header, OCTET *data, int *data_len );

void RS_recv_buff_init( void );

/*---------------------------------------------------------------------------*/

void RS_reset( void );

void RS_init( WORD );
void RS_restore( void );
void send_CGC_message (char *mensaje, int len);

/*---------------------------------------------------------------------------*/



