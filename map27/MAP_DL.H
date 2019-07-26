


/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_DL.H
*
*  PURPOSE:		MAP27 Data Link Layer
*
*  REMARKS:		
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		13.3.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#define SIGNAL_LINK_READY		0x01
#define SIGNAL_LINK_FAIL		0x02
#define SIGNAL_LINK_PACKET_ACK		0x04
#define SIGNAL_LINK_PACKET_REJ		0x08

#define SIGNAL_LINK_RESET		0x01
#define SIGNAL_CREDIT_VALUE		0x02


/*---------------------------------------------------------------------------*/









/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/
	

void send_link_signal( int s, int par );
BOOLEAN receive_link_signal( int s );
void send_link_packet( OCTET *data, int data_len );
BOOLEAN receive_link_packet( OCTET *data, int *data_len );

int DL_recv_buff_status( void );

/*---------------------------------------------------------------------------*/


void PROCESS_data_link( void );
void DL_init( void );

/*---------------------------------------------------------------------------*/




