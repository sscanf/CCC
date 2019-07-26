

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_NET.H
*
*  PURPOSE:		MAP27 Network Layer
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


#define SIGNAL_NETWORK_READY		0x01
#define SIGNAL_NETWORK_FAIL		0x02


#define SIGNAL_NETWORK_LAYER_RESET	0x01

/*---------------------------------------------------------------------------*/


typedef enum { SEND_IDLE=0, SEND_PENDING, SEND_IN_SERVICE, SEND_IN_PROGRESS, SEND_OK, SEND_FAIL };


typedef struct {
	int len;
	OCTET message_type;
	OCTET msg[ MAX_DATA_LEN -1 ];
}
GENERIC_NETWORK_MESSAGE;


typedef struct {
	int len;
	OCTET message_type;
	OCTET msg[ MAX_DATA_LEN -1 ];
}
GENERIC_APPL_MESSAGE;	


typedef GENERIC_APPL_MESSAGE 	CALL_SETUP_MESSAGE;


/*---------------------------------------------------------------------------*/








/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


BOOLEAN receive_network_message( GENERIC_NETWORK_MESSAGE * );
BOOLEAN receive_network_signal( int );
void send_network_signal( int );
BOOLEAN send_network_message( GENERIC_APPL_MESSAGE * );
/*BOOLEAN send_long_network_message( CALL_SETUP_MESSAGE *, FILE *, BOOLEAN );*/
int receive_network_status( void );


/*---------------------------------------------------------------------------*/




