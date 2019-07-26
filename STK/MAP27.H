/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*
*  FILE:		MAP27.H
*
*  PURPOSE:		MAP27 General Definitions
*
*  REMARKS:
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		22.4.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#define LOBYTE( p ) 		p & 0xFF
#define HIBYTE( p ) 		(p >> 8) & 0xFF
#define MKWORD( p1, p2 )	((p1 << 8) | p2)

/*---------------------------------------------------------------------------*/

typedef unsigned char 	BYTE;
typedef unsigned  	WORD;

typedef BYTE		OCTET;


typedef enum { FALSE=0, TRUE=1 }	BOOLEAN;

/*---------------------------------------------------------------------------*/


#define COM1	0x3F8
#define COM2	0x2f8
#define COM3	0x3E8
#define COM4	0x2E8


typedef struct {

	WORD MAP27_port_address;
	WORD modem_port_address;

} COM_CONFIG;

/*---------------------------------------------------------------------------*/

#define UNIT_MAXIMUM_LENGTH 		10	/* N1 */
#define UNIT_MAXIMUM_WINDOW_SIZE 	1	/* k */
#define UNIT_VERSION			1

/*---------------------------------------------------------------------------*/

#define LINK_HEADER_LEN		4
#define MAX_DATA_LEN		16*( UNIT_MAXIMUM_LENGTH+1 )

/*---------------------------------------------------------------------------*/


/* MESSAGE TYPES */

#define SEND_STATUS		0x80
#define SEND_SST		0x81
#define SEND_MST		0x82

#define RECEIVE_STATUS		0x80
#define RECEIVE_SST		0x81
#define RECEIVE_MST		0x82


#define STATUS_SST_MST_ACKP	0xC0

#define ACK1	0x00


#define STATUS_SST_MST_ACKQ	0xD0

#define ACKQ1	0x02
#define ACKQ2	0x0A
#define ACKT1	0x26
#define ALocal	0x22


#define STATUS_SST_MST_ACKN	0xE0

#define ACK2	0x08
#define ACKX1	0x03
#define ACKX2	0x0B
#define ACKV1	0x04
#define ACKV2	0x0C
#define ACKT2	0x06
#define ACKT3	0x16
#define ACKT4	0x0E
#define AAbort	0x0D


#define INCOMING_VOICE_MODEM	0xA4
#define SETUP_VOICE_MODEM		0xA4
#define SETUP_EMERGENCY			0xA5

#define DISCONNECT				0x86
#define DISCONNECT_C				0xA6

#define RQX1						0x08
#define RQX4						0x0C
#define MAINT						0x07


#define CLEARED_N					0x86
#define CLEARED_A					0xA6

#define SETUP_PROGRESS_P		0xC4
#define SETUP_PROGRESS_Q		0xD4
#define SETUP_PROGRESS_N		0xE4

#define RADIO_INTERROGATION	0xB0
#define RADIO_PERSONALITY		0xB0

#define NUMBERING_INFO			0xB1

#define OPERATING_CONDITION	0xB2

#define RADIO_MANAGEMENT		0xB3
#define RADIO_SETTINGS			0xB3

#define PROTOCOL_INFO			0xB4

#define RADIO_CONTROL			0xB2


/*---------------------------------------------------------------------------*/

