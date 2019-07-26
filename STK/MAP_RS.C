

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*
*  FILE:		MAP_RS.C
*
*  PURPOSE:		MAP27 RS Link Driver
*
*  REMARKS:		COM IRQ
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		8.5.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/

#define LINT_ARGS

#include "map27.h"
#include "map_rs.h"
#include "v25lib.h"
#include "cgc.h"

/*---------------------------------------------------------------------------*/

		/*** 8250 COM1 I/O Decodes ***/

#define	COMDATA		0x00		/* Tx/Rx Data Buffer (DLAB=0) */
#define  COMIER      0x01     /* Interrupt enable */
#define	COMLCR		0x03		/* Line Control */
#define	COMMCR		0x04		/* Modem Control */
#define  COMLSR 		0x05		/* Line Status */
#define	COMTHRE		0x20



#define SCCR		0xE0 | 0x03 | 0x00 | 0x00	/* 9600,8,1,N */

#define IER		0x21
#define EOI		0x20

/*---------------------------------------------------------------------------*/

#define SYN	0x16
#define DLE	0x10
#define STX	0x02
#define ETX	0x03


#define EC	0xec

/*---------------------------------------------------------------------------*/

#define RB_SIZE		10

/*---------------------------------------------------------------------------*/

typedef enum { STATE_SYN, STATE_DLE, STATE_STX, 	/* PROCESS */
		STATE_data, STATE_DLE2,
		STATE_CRC1, STATE_CRC2,
		STATE_ready } prueba;

/*---------------------------------------------------------------------------*/

typedef enum { STATE_EC=0, STATE_A1, STATE_FLOTA,
					STATE_INDIH, STATE_INDIL,
					STATE_MENSAJE, STATE_NUMBYTES,
					STATE_DATA,STATE_READY} prueba2;

typedef struct {
	BYTE len;
	BYTE data[ LINK_HEADER_LEN + MAX_DATA_LEN ];
}
RS_BUFF;

typedef struct {
	BYTE len;
	BYTE header[ LINK_HEADER_LEN ];
	BYTE data[ MAX_DATA_LEN ];
}
RS_PACKET_TYPE;


/*extern tareas *ptrtareas;*/


/*****************************************************************************/


WORD mtab[256];				/* CRC16 */

int comport;

WORD comdata;
WORD comier;
WORD comlcr;
WORD commcr;
WORD comlsr;

int comvect;
BYTE commask8259;
BYTE comnmask8259;


extern int tarea;
/*--------------------------------------------------------------------------*/


RS_BUFF RS_recv_buff[ RB_SIZE ];	/* FROM RU */
volatile int RS_recv_qin, RS_recv_qout;

BYTE *rb;
int rbi;

/*---------------------------------------------------------------------------*/


int RS_state;           /* PROCESS */
int CGC_state=STATE_EC;

BYTE rc, rsts;
BYTE rCRC1, rCRC2;
WORD rCRC16, tCRC16;

/*****************************************************************************/


void RS_recv_buff_dequeue( OCTET *header, OCTET *data, int *data_len );
BOOLEAN RS_recv_buff_empty( void );

void rCRC( BYTE b );
void tCRC( BYTE b );

void RS_transmit_ctrl( BYTE c );
void RS_transmit_data( BYTE c );


/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/



void send_RS_packet( OCTET *header, OCTET *data, int data_len )
{
	int i;


	tCRC16 = 0xffff;

	RS_transmit_ctrl( SYN );		/* frame header */
	RS_transmit_ctrl( DLE );
	RS_transmit_ctrl( STX );

	for ( i=0; i<LINK_HEADER_LEN; i++ )
		RS_transmit_data( header[i] );

	for ( i=0; i<data_len; i++ )
		RS_transmit_data( data[i] );

	RS_transmit_ctrl( DLE ); tCRC( DLE );	/* frame tailer */
	RS_transmit_ctrl( ETX ); tCRC( ETX );

	tCRC16 ^= 0xFFFF;

	RS_transmit_ctrl( (BYTE)HIBYTE( tCRC16 ) );
	RS_transmit_ctrl( (BYTE)LOBYTE( tCRC16 ) );
	sti();
}


BOOLEAN receive_RS_packet( OCTET *header, OCTET *data, int *data_len )
{
	if ( ! RS_recv_buff_empty() ) {

		RS_recv_buff_dequeue( header, data, data_len );
		return TRUE;
	}
	else
		return FALSE;		/* no data */
}


/*---------------------------------------------------------------------------*/










/******************************************************************************
*
*	*** SERVICE PROVIDER ***
*
******************************************************************************/



void create_table( WORD *mtab )		/* CRC16 */
{
	WORD btab[8];
	WORD i,j, q, shreg, carry, bit;

	carry = 1;
	shreg = 0;
	for ( i=0; i<8; i++ ) {
		if ( carry ) shreg ^= 0xA001;
		btab[i] = (shreg<<8) | (shreg>>8);
		carry = shreg & 1;
		shreg >>= 1;
	}

	for ( i=0; i<256; i++ ) {
		q = 0;
		bit = 0x80;
		for ( j=0; j<8; j++ ) {
			if ( bit & i ) q ^= btab[j];
			bit >>= 1;
		}
		*mtab++ = q;
	}
}



void rCRC( BYTE b )
{
	WORD q;

	q = *(mtab+(b ^ (rCRC16>>8)));
	rCRC16 = ((q&0xFF00) ^ (rCRC16<<8)) | (q&0x00ff);
}


void tCRC( BYTE b )
{
	WORD q;

	q = *(mtab+(b ^ (tCRC16>>8)));
	tCRC16 = ((q&0xFF00) ^ (tCRC16<<8)) | (q&0x00ff);
}


/*---------------------------------------------------------------------------*/


/* MESSAGES FROM RU */


void RS_recv_buff_enqueue( void )
{
	RS_recv_buff[ RS_recv_qin ].len = (BYTE)rbi;		/* len */

	if ( ++RS_recv_qin == RB_SIZE ) RS_recv_qin = 0;
}


void RS_recv_buff_dequeue( OCTET *header, OCTET *data, int *data_len )
{
	int i;
	RS_PACKET_TYPE *p = (RS_PACKET_TYPE *) &(RS_recv_buff[ RS_recv_qout ]);


	for ( i=0; i<RB_SIZE; i++ )		/* copy header */
		header[i] = p->header[i];

	for ( i=0; i<p->len-LINK_HEADER_LEN; i++ )	/* copy data */
		data[i] = p->data[i];
	*data_len = p->len - LINK_HEADER_LEN;


	if ( ++RS_recv_qout == RB_SIZE ) RS_recv_qout = 0;
}


BOOLEAN RS_recv_buff_empty( void )
{
	if ( RS_recv_qin == RS_recv_qout )
		return TRUE;
	else
		return FALSE;		
}


BYTE * RS_recv_buff_alloc( void )
{
	return RS_recv_buff[ RS_recv_qin ].data;  /* ??? OVERFLOW */
}


void RS_recv_buff_init( void )
{
	cli();
	RS_state = STATE_SYN;
	RS_recv_qin = RS_recv_qout = 0;
	sti();
}

void CGC_recv_buff_init( void )
{
	cli();
	RS_state = STATE_SYN;
	RS_recv_qin = RS_recv_qout = 0;
	CGC_state = STATE_EC;
	sti();
}

/*---------------------------------------------------------------------------*/


void RS_transmit_ctrl( BYTE c )
{
	int rc;

	while ((rc=comtrans (1,c)));
}

void RS_transmit_data( BYTE c )
{
	RS_transmit_ctrl( c );
	if ( c == DLE )
		RS_transmit_ctrl( DLE );	/* DLE ---> DLE DLE */

	tCRC( c );				/* compute FCS */
}

/*---------------------------------------------------------------------------*/



void RS_receive_CGC(char rc)     /* RECEIVE CGC */
{
	static int nBytes=0;
	static unsigned char IndiL;
	static unsigned char IndiH;
	static unsigned char flota;
	static unsigned char mensaje;

	switch ( CGC_state )
	{

		case STATE_EC:
			if (rc=='\xec') CGC_state = STATE_A1;
		break;

		case STATE_A1:
			if (rc=='\xa1') CGC_state = STATE_FLOTA;
			else
				CGC_state=STATE_EC;
		break;

		case STATE_FLOTA:
			flota = rc;
			CGC_state = STATE_INDIH;
		break;

		case STATE_INDIH:
			IndiH = rc;
			CGC_state = STATE_INDIL;
		break;

		case STATE_INDIL:
			IndiL = rc;
			CGC_state = STATE_MENSAJE;
		break;


		case STATE_MENSAJE:
			mensaje=rc;
			CGC_state = STATE_NUMBYTES;
		break;

		case STATE_NUMBYTES:
			EncolaTarea (flota,IndiH, IndiL, mensaje);
			CGC_recv_buff_init();
		break;

		default:
		break;
	}

}



void RS_receive_isr(char rc)     /* RECEIVE ISR */
{
	switch ( RS_state ) {         /* frame */

	case STATE_SYN:

		if ( rc == SYN ) {
			rb = RS_recv_buff_alloc(); rbi = 0;
			rCRC16 = 0xFFFF;
			RS_state = STATE_DLE;
		}
		break;

	case STATE_DLE:
		if ( rc == DLE )
			RS_state = STATE_STX;
		else
			RS_state = STATE_SYN;
		break;

	case STATE_STX:
		if ( rc == STX )
			RS_state = STATE_data;
		else
			RS_state = STATE_SYN;
		break;

	case STATE_data:

      rb[rbi++] = rc;         /* STORE */ 
		if ( rbi >= LINK_HEADER_LEN + MAX_DATA_LEN )
			RS_state = STATE_SYN;	/* overflow ??? */

		rCRC( rc );
		if ( rc == DLE )
			RS_state = STATE_DLE2;
		break;

	case STATE_DLE2:
		if ( rc == DLE )
			RS_state = STATE_data;		/* DLE DLE ---> DLE */
		else {
			if ( rc == ETX ) {
				rbi--;
				RS_state = STATE_CRC1;	/* DLE ETX */
			}
			else
				RS_state = STATE_SYN;
		}
		break;

	case STATE_CRC1:
		rCRC1 = rc;
		RS_state = STATE_CRC2;
		break;

	case STATE_CRC2:
		rCRC2 = rc;

	case STATE_ready:
		rCRC( ETX );
		if ( (rCRC16^0xFFFF) == MKWORD( rCRC1, rCRC2 ) )	/* OK ? */
			RS_recv_buff_enqueue();

		RS_state = STATE_SYN;
		break;

	default:
		break;

	}
}


/*---------------------------------------------------------------------------*/



void RS_reset( void )
{
	cli();

	CGC_recv_buff_init();
	RS_recv_buff_init();
	RS_state = STATE_SYN;
	sti();
}


void RS_init()
{
	int irq;


	create_table( mtab );		/* CRC lookup */

	cli();

	RS_recv_buff_init();
	RS_state = STATE_SYN;
	sti();
}

unsigned stinti ()
{
	return(0);
}

void seint ()
{
}

/*---------------------------------------------------------------------------*/


