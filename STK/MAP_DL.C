

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_DL.C
*
*  PURPOSE:		MAP27 Data Link Layer
*
*  REMARKS:		UADG Version 1.2 / August 1993
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		23.12.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#include <stdlib.h>

#include "map27.h"
#include "map_dl.h"
#include "map_rs.h"
#include "map_tim.h"
#include "cgc.h"

/*---------------------------------------------------------------------------*/

#define sig( s )	link_signals |= s
#define reset_sig( s )	link_signals &= ~s
#define is_sig( s )	(link_signals & s) != 0

/*---------------------------------------------------------------------------*/

#define LR		0x01		/* Link Request */
#define LA		0x02		/* Link Ack */
#define LT		0x04		/* Link Transfer */

/*---------------------------------------------------------------------------*/


#define N2		10		/* max # of retransmissions */
#define N3		5		/* max activity count */

#define	T0		MS100		/* establishment phase retry timer */
#define	T1		MS400		/* transfer phase retry timer */
#define	T2		MS300		/* acknowledgement timer */
#define	T3		5		/* activity timer */


#define DLB_SIZE	10	
#define txB_SIZE	10

/*---------------------------------------------------------------------------*/


#define EXIT( p )	DL_STATE = p


typedef enum { reset_wait, link_wait, ready } enum1;  /* PROCESS */


typedef enum {				/* INPUT */
	INPUT_power_on,

	INPUT_link_establishment_timer_expired,
	INPUT_retry_timeout,
	INPUT_activity_timeout,
	INPUT_acknowledgement_timeout,

	INPUT_link_request,
	INPUT_link_acknowledge,
	INPUT_link_transfer,

	INPUT_network_layer_reset,
	INPUT_network_layer_packet,
	INPUT_credit_value,

	INPUT_NONE
}enum2;


typedef enum {				/* OUTPUT */
	link_request,
	link_acknowledge,
	link_transfer,

	link_ready,
	link_failure,
	packet_accepted,
	packet_rejected,
	network_layer_packet
}enum3;


/*---------------------------------------------------------------------------*/


typedef struct {
	OCTET packet_type;
	OCTET p1;
	OCTET p2;
	OCTET p3;
} 
GENERIC_HEADER_TYPE;	

typedef struct {
	OCTET packet_type;
	OCTET N1;
	OCTET k;
	OCTET VERSION;
} 
LR_HEADER_TYPE;	

typedef struct {
	OCTET packet_type;
	OCTET NR;
	OCTET Nk;
	OCTET reserved;
} 
LA_HEADER_TYPE;	

typedef struct {
	OCTET packet_type;
	OCTET NS;
	OCTET AR;
	OCTET reserved;
} 
LT_HEADER_TYPE;	


/*****************************************************************************/


int DL_STATE;				/* PROCESS */
int DL_INPUT;


/*---------------------------------------------------------------------------*/


BYTE current_version;			

BYTE LT_ack_request;			/* AR */
int maximum_length;			/* N1 */
int retry_count;			/* C2 */
BYTE activity_count;			/* C3 */
BYTE window_size;			/* k */
BYTE tx_sequence_number;		/* N(S) */
BYTE receive_credit;			/* R(k) */	
BYTE send_credit;			/* S(k) */
BYTE stored_NR;				/* SN(R) */
BYTE stored_Rk;				/* SR(k) */
BYTE send_state;			/* V(S) */
BYTE receive_state;			/* V(R) */


/*---------------------------------------------------------------------------*/


BYTE last_transmitted_NS;
BYTE last_transmitted_NR;
BYTE last_transmitted_Nk;
BYTE last_received_NS;
BYTE last_received_NR, last_but_one_received_NR;


/*---------------------------------------------------------------------------*/


typedef struct {
	int data_len;
	OCTET data[ MAX_DATA_LEN + LINK_HEADER_LEN ];
}
DL_BUFF;				/* TO NETWORK */

DL_BUFF DL_recv_buff[ DLB_SIZE ];
int DL_recv_qin, DL_recv_qout;

/*---------------------------------------------------------------------------*/


struct {
	int data_len;
	GENERIC_HEADER_TYPE header;			
	OCTET data[ MAX_DATA_LEN ];
}
link_packet;


struct {
	int data_len;
	OCTET data[ MAX_DATA_LEN ];
}
network_packet;


/*---------------------------------------------------------------------------*/


BOOLEAN link_ok;

BOOLEAN signal_power_on;

WORD link_signals;

BOOLEAN network_layer_reset_request;
BOOLEAN network_credit_value_request;
BOOLEAN network_credit_value;
BOOLEAN network_layer_packet_request;

/*---------------------------------------------------------------------------*/

volatile BOOLEAN let_expired;
volatile BOOLEAN retry_timeout;
volatile BOOLEAN acknowledgement_timeout;
volatile BOOLEAN activity_timeout;

/*---------------------------------------------------------------------------*/


struct {
	int stored_packet_number;
	int data_len;
	OCTET data[ MAX_DATA_LEN ];
} 
tx_messages_buffer[ txB_SIZE ];		/* tx window buffer */

int tx_messages_qin, tx_messages_qout;
int tx_message_pointer;

/*****************************************************************************/


void DL_recv_buff_dequeue( OCTET *data, int *data_len );
BOOLEAN DL_recv_buff_empty( void );
void DL_recv_buff_init( void );

void tx_messages_buffer_init( void );



/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


void send_link_signal( int s, int par )
{
	switch ( s ) {
	
	case SIGNAL_LINK_RESET:
		network_layer_reset_request = TRUE;
		break;
	
	case SIGNAL_CREDIT_VALUE:
		network_credit_value_request = TRUE;
		network_credit_value = par;
		break;

	default:
		break;
	}		
}


BOOLEAN receive_link_signal( int s )
{
	if ( link_signals == 0 )
		return FALSE;
	else {		
		if ( is_sig( s ) ) {
			reset_sig( s );		/* clear */
			return TRUE;
		}
		else
			return FALSE;
	}
}


void send_link_packet( OCTET *data, int data_len )
{
	int i;
	
	for ( i=0; i<data_len; i++ )
		network_packet.data[i] = data[i];
	network_packet.data_len = data_len;
	network_layer_packet_request = TRUE;
}	


BOOLEAN receive_link_packet( OCTET *data, int *data_len )
{
	if ( ! DL_recv_buff_empty() ) {
		DL_recv_buff_dequeue( data, data_len );
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


void send_link_request( OCTET max_len, OCTET max_win_size, OCTET vers )
{
	LR_HEADER_TYPE header;
	OCTET NULL_data[1];

	header.packet_type	= LR;
	header.N1 		= max_len;
	header.k 		= max_win_size;
	header.VERSION 		= vers;

	send_RS_packet( (OCTET *) &header, NULL_data, 0 );
}


void send_link_ack( OCTET rec_state, OCTET rec_credit )
{
	LA_HEADER_TYPE header;
	OCTET NULL_data[1];

	header.packet_type 	= LA;
	header.NR 		= rec_state;	last_transmitted_NR = header.NR;
	header.Nk 		= rec_credit;	last_transmitted_Nk = header.Nk;
	header.reserved 	= 0;

	send_RS_packet( (OCTET *) &header, NULL_data, 0 );
}


void send_link_transfer( OCTET tx_seq, OCTET ack_req, OCTET *data, int data_len )
{
	LT_HEADER_TYPE header;

	header.packet_type 	= LT;
	header.NS 		= tx_seq;	last_transmitted_NS = header.NS;
	header.AR 		= ack_req;
	header.reserved 	= 0;

	send_RS_packet( (OCTET *) &header, data, data_len );
}


/*---------------------------------------------------------------------------*/



void network_signal( int s )		/* Notify network layer */
{
	switch ( s ) {

	case link_ready:
		sig( SIGNAL_LINK_READY );
		break;

	case link_failure:
		sig( SIGNAL_LINK_FAIL );
		break;

	case packet_accepted:
		sig( SIGNAL_LINK_PACKET_ACK );
		break;

	case packet_rejected:
		sig( SIGNAL_LINK_PACKET_REJ );
		break;

	default:
		break;
	}
}

/*---------------------------------------------------------------------------*/


/* TX WINDOW BUFFER QUEUE */


void tx_messages_buffer_init( void )
{
	tx_messages_qin = tx_messages_qout = tx_message_pointer = 0;
}


void inc_tx_messages_qin( void )
{
	if ( ++tx_messages_qin == txB_SIZE ) tx_messages_qin = 0;
}


void inc_tx_messages_qout( void )
{
	if ( ++tx_messages_qout == txB_SIZE ) tx_messages_qout = 0;
}


void inc_tx_message_pointer( void )
{
	if ( ++tx_message_pointer == txB_SIZE ) tx_message_pointer = 0;
}


void dec_tx_message_pointer( void )
{
	if ( tx_message_pointer-- == 0 ) tx_message_pointer = txB_SIZE-1;
}

/*---------------------------------------------------------------------------*/


/* MESSAGES TO NETWORK */


void DL_recv_buff_enqueue( OCTET *data, int data_len )
{
	int i;
	int b = DL_recv_qin;


	for ( i=0; i<data_len; i++ )
		DL_recv_buff[b].data[i] = data[i];
	DL_recv_buff[b].data_len = data_len;

	if ( ++DL_recv_qin == DLB_SIZE ) DL_recv_qin = 0;
}


void DL_recv_buff_dequeue( OCTET *data, int *data_len )
{
	int i;
	int b = DL_recv_qout;


	for ( i=0; i<DL_recv_buff[b].data_len; i++ )
		data[i] = DL_recv_buff[b].data[i];
	*data_len = DL_recv_buff[b].data_len;

	if ( ++DL_recv_qout == DLB_SIZE ) DL_recv_qout = 0;
}


BOOLEAN DL_recv_buff_empty( void )
{
	if ( DL_recv_qin == DL_recv_qout )
		return TRUE;
	else
		return FALSE;		
}


int DL_recv_buff_status( void )
{
	int a = DL_recv_qin - DL_recv_qout;	

	if ( a<0 ) a = DLB_SIZE - a;		/* # of queued items */	
	return( DLB_SIZE - a );			
}


void DL_recv_buff_init( void )
{
	DL_recv_qin = DL_recv_qout = 0;
}


/*---------------------------------------------------------------------------*/


void send_net_packet( OCTET *data, int data_len )
{
	DL_recv_buff_enqueue( data, data_len );

}

/*****************************************************************************/



static int INPUT( void )
{

	if ( signal_power_on ) {
		signal_power_on = FALSE;
		return INPUT_power_on;
	}
	
	if ( let_expired ) {
		let_expired = FALSE;
		return INPUT_link_establishment_timer_expired;
	}

	if ( retry_timeout ) {
		retry_timeout = FALSE;
		return INPUT_retry_timeout;
	}

	if ( activity_timeout ) {
		activity_timeout = FALSE;
		return INPUT_activity_timeout;
	}

	if ( acknowledgement_timeout ) {
		acknowledgement_timeout = FALSE;
		return INPUT_acknowledgement_timeout;
	}


	/* FROM RS */

	if ( receive_RS_packet( (OCTET *) &link_packet.header,
				link_packet.data, &link_packet.data_len ) ) {

		switch ( link_packet.header.packet_type ) {

		case LR:
			return INPUT_link_request;

		case LA:
			last_but_one_received_NR = last_received_NR;
			last_received_NR = ((LA_HEADER_TYPE *) &link_packet.header)->NR;
			return INPUT_link_acknowledge;

		case LT:
			last_received_NS = ((LT_HEADER_TYPE *) &link_packet.header)->NS;
			return INPUT_link_transfer;
		
		default:
			break;
		}			
	}		


	/* FROM NETWORK */
	
	if ( network_layer_reset_request ) {
		network_layer_reset_request = FALSE;
		return INPUT_network_layer_reset;
	}		

	if ( network_credit_value_request ) {
		network_credit_value_request = FALSE;
		return INPUT_credit_value;
	}		

	if ( network_layer_packet_request ) {
		network_layer_packet_request = FALSE;
		return INPUT_network_layer_packet;
	}


	return INPUT_NONE;
}


/*---------------------------------------------------------------------------*/


static void OUTPUT( int p )
{
	switch( p ) {

	/* TO NETWORK */

	case link_ready:
	case link_failure:
	case packet_accepted:
	case packet_rejected:
		network_signal( p );
		break;

	case network_layer_packet:
		send_net_packet( link_packet.data, link_packet.data_len );
		break;


	/* TO RS */

	case link_request:
		send_link_request( UNIT_MAXIMUM_LENGTH,
										UNIT_MAXIMUM_WINDOW_SIZE,
						  UNIT_VERSION );
		break;

	case link_acknowledge:
		send_link_ack( receive_state, receive_credit );
		break;

	case link_transfer:
		send_link_transfer( tx_sequence_number, LT_ack_request,
			tx_messages_buffer[tx_message_pointer].data,
			tx_messages_buffer[tx_message_pointer].data_len );
		inc_tx_message_pointer();
		break;


	default:
		break;

	}
}


/*---------------------------------------------------------------------------*/


/* TASKs */

void TASK_adjust_link_parameters( void )
{
	LR_HEADER_TYPE *p = (LR_HEADER_TYPE *) &link_packet.header;

	maximum_length = min( p->N1, UNIT_MAXIMUM_LENGTH );
	window_size = min( p->k, UNIT_MAXIMUM_WINDOW_SIZE );
	current_version = min( p->VERSION, UNIT_VERSION );
}


void TASK_decrement_activity_count( void )
{
	activity_count--;
}


void TASK_decrement_receive_credit( void )
{
	receive_credit--;	
}


void TASK_decrement_retry_count( void )
{
	retry_count--;
}

	
void TASK_decrement_send_credit( void )
{
	send_credit--;
}


void TASK_delete_acknowledged_packets( void )
{
	while ( tx_messages_qout != tx_message_pointer ) {
		if ( tx_messages_buffer[tx_messages_qout].
			stored_packet_number != last_received_NR )   /* < */
			/* delete */
			inc_tx_messages_qout();
		else
			break;			
	}
}


void TASK_increment_receive_state( void )
{
	receive_state = ( receive_state + 1 ) % 256;
}


void TASK_increment_send_state( void )
{
	send_state = ( send_state + 1 ) % 256;
}


void TASK_initialise_variables( void )
{
	send_state = 1;
	receive_state = 1;
	receive_credit = 1;		/* k */
	send_credit = 0;
	stored_NR = 1;
	DL_recv_buff_init();		/* clear message buffers */
	tx_messages_buffer_init();
	RS_recv_buff_init();
	retry_count = N2;
	activity_count = N3;
}	


void TASK_initialise_rs232_port( void )
{
	RS_reset();
}


void TASK_maximise_link_parameters( void )
{
	maximum_length = UNIT_MAXIMUM_LENGTH;
	window_size = UNIT_MAXIMUM_WINDOW_SIZE;
	current_version = UNIT_VERSION;
}


void TASK_record_send_credit( void )
{
	LA_HEADER_TYPE *p = (LA_HEADER_TYPE *) &link_packet.header;
	
	send_credit = p->Nk;
}


void TASK_rewind_packet_number( void )
{
	tx_sequence_number = last_received_NR;
	while ( tx_message_pointer != tx_messages_qout ) {
		if ( tx_messages_buffer[tx_message_pointer].
			stored_packet_number == last_received_NR ) 
			/* next message to be transmitted */
			break;
		else
			dec_tx_message_pointer();
	}
}


void TASK_set_activity_count( void )
{
	activity_count = N3;
}


void TASK_set_retry_count( void )
{
	retry_count = N2;
}


void TASK_store_ackd_rx_seq_number( void )
{
	stored_NR = last_received_NR;
}


void TASK_store_packet( void )
{
	int b = tx_messages_qin;
	int i;
		
	for ( i=0; i<network_packet.data_len; i++ )
		tx_messages_buffer[b].data[i] = network_packet.data[i];
	tx_messages_buffer[b].data_len = network_packet.data_len;
	tx_messages_buffer[b].stored_packet_number = receive_state;

	inc_tx_messages_qin();
}


void TASK_store_receive_credit( void )
{
	stored_Rk = receive_credit;
}


void TASK_update_receive_credit( void )
{
	receive_credit = min( UNIT_MAXIMUM_WINDOW_SIZE, network_credit_value );
}					


void TASK_set_retransmission( void )
{
	dec_tx_message_pointer();		/* repeat the previous */
}


void TASK_set_tx_seq( void )
{
	tx_sequence_number = send_state;	/* N(S) := V(S) */
}


/*---------------------------------------------------------------------------*/


/* IFs */

BOOLEAN acknowledgement_inside_window( void )
{
	BYTE a, b;


	a = stored_NR;			/* SN(R) */
	b =  send_state;		/* V(S) */

	if ( a <= b ) {
		if ( ( last_received_NR >= a ) && ( last_received_NR <= b ) )
			return TRUE;		
		else
			return FALSE;		
	}
	else {
		if ( ( last_received_NR >= a ) || ( last_received_NR <= b ) )
			return TRUE;		
		else
			return FALSE;		
	}			
}


BOOLEAN activity_count_zero( void )
{
	if ( activity_count == 0 )
		return TRUE;
	else
		return FALSE;		
}

	
BOOLEAN all_transmitted_packet_acknowledged( void ) 
{
	if ( last_received_NR == send_state )	
		return TRUE;
	else
		return FALSE;			
}


BOOLEAN immediate_reply_requested( void )
{
	LT_HEADER_TYPE *p = (LT_HEADER_TYPE *) &link_packet.header;

	if ( p->AR == 1 )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN packet_out_of_sequence( void )
{
	if ( last_received_NS != receive_state )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN received_parameters_acceptable( void ) 
{
	LR_HEADER_TYPE *p = (LR_HEADER_TYPE *) &link_packet.header;
	
	if ( ( p->N1 <= UNIT_MAXIMUM_LENGTH ) &&
	     ( p->k <= UNIT_MAXIMUM_WINDOW_SIZE ) &&
	     ( p->VERSION <= UNIT_VERSION ) ) 
	     
	     	return TRUE;
	else
		return FALSE;		
}	


BOOLEAN repeated_link_acknowledge( void )
{
	if ( (last_received_NR == last_but_one_received_NR) && 
	     (last_received_NR != send_state) )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN receive_credit_available( void ) 
{
	if ( receive_credit >0 )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN receive_credit_changed( void ) 
{
	if ( receive_credit > stored_Rk )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN retry_count_zero( void )
{
	if ( retry_count == 0 )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN send_credit_available( void )
{
	if ( send_credit >0 )
		return TRUE;
	else
		return FALSE;		
}


BOOLEAN packet_outside_window( void )
{
	BYTE a, b;


	a = last_transmitted_NR;
	b = ( last_transmitted_NR + last_transmitted_Nk ) % 256;

	if ( a <= b ) {
		if ( ( last_received_NS >= a ) && ( last_received_NS <= b ) )
			return FALSE;		
		else
			return TRUE;		
	}
	else {
		if ( ( last_received_NS >= a ) || ( last_received_NS <= b ) )
			return FALSE;		
		else
			return TRUE;		
	}			
}


BOOLEAN repeat_ok( void )              /*??? */
{
	if ( ( tx_messages_qin != tx_messages_qout ) &&
		( tx_messages_buffer[tx_messages_qout].
			stored_packet_number == last_received_NR ) )

		return TRUE;
	else
		return FALSE;		
}



/*****************************************************************************/



void PROCEDURE_LRT( void )
{
	TIMESTOP( retry_timer );
	TIMESTOP( acknowledgement_timer );
	TIMESTOP( activity_timer );
	OUTPUT( link_request );
	TIMESTART( link_establishment_timer, T0, &let_expired );
}


/*****************************************************************************/



void STATE_reset_wait( void )
{
	switch( DL_INPUT = INPUT() ) {

	case INPUT_network_layer_reset:
	case INPUT_power_on:
		TASK_initialise_rs232_port();
		TASK_maximise_link_parameters();
		PROCEDURE_LRT();
		EXIT( reset_wait );
		break;

	case INPUT_link_request:
		TASK_adjust_link_parameters();
		OUTPUT( link_request );
		TIMESTART( link_establishment_timer, T0, &let_expired );
		EXIT( link_wait );
		break;

	case INPUT_link_acknowledge:
	case INPUT_link_transfer:
	case INPUT_link_establishment_timer_expired:
		OUTPUT( link_request );
		TIMESTART( link_establishment_timer, T0, &let_expired );
		EXIT( reset_wait );
		break;

	case INPUT_network_layer_packet:
		OUTPUT( packet_rejected );
		EXIT( reset_wait );
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void STATE_link_wait( void )
{
	switch( DL_INPUT = INPUT() ) {

	case INPUT_link_request:
		if ( received_parameters_acceptable() ) {
			TASK_adjust_link_parameters();
			TASK_initialise_variables();
			TIMESTOP( link_establishment_timer );
			OUTPUT( link_acknowledge );
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/*       OUTPUT( link_ready );       to network layer */
			link_ok = FALSE;
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
			TIMESTART( activity_timer, T3, &activity_timeout );
			EXIT( ready );
		}
		else {
			TASK_adjust_link_parameters();
			OUTPUT( link_request );
			TIMESTART( link_establishment_timer, T0, &let_expired  );
			EXIT( link_wait );
		}
		break;

	case INPUT_link_acknowledge:
		TASK_initialise_variables();
		TASK_record_send_credit();
		TIMESTOP( link_establishment_timer );
		OUTPUT( link_acknowledge );
		OUTPUT( link_ready ); link_ok = TRUE;
		TIMESTART( activity_timer, T3, &activity_timeout );
		EXIT( ready );
		break;

	case INPUT_link_establishment_timer_expired:
	case INPUT_link_transfer:
		OUTPUT( link_request );
		TIMESTART( link_establishment_timer, T0, &let_expired );
		EXIT( link_wait );
		break;

	case INPUT_network_layer_reset:
		TASK_initialise_rs232_port();
		TASK_maximise_link_parameters();
		PROCEDURE_LRT();
		EXIT( reset_wait );
		break;


	case INPUT_network_layer_packet:
		OUTPUT( packet_rejected );
		EXIT( link_wait );
		break;

	default:
		break;

	}
			
}

/*---------------------------------------------------------------------------*/



void STATE_ready( void )
{
	switch( DL_INPUT = INPUT() ) {

	case INPUT_network_layer_packet:
		if ( send_credit_available() ) {
			OUTPUT( packet_accepted );
			TASK_store_packet();
			TASK_set_retry_count();
			TASK_set_tx_seq();
			OUTPUT( link_transfer );
			TASK_increment_send_state();
			TASK_decrement_send_credit();	
			TIMESTART( retry_timer, T1, &retry_timeout );
		}
		else
			OUTPUT( packet_rejected );
		EXIT( ready );
		break;

	case INPUT_retry_timeout:
		TASK_decrement_retry_count();
		if ( retry_count_zero() ) {
			TASK_maximise_link_parameters();
			OUTPUT( link_failure );
			PROCEDURE_LRT();
			EXIT( reset_wait );
		}
		else {
			TASK_set_retransmission();
			OUTPUT( link_transfer );
			TIMESTART( retry_timer, T1, &retry_timeout );
			EXIT( ready );
		}
		break;

	case INPUT_link_acknowledge:
		if ( acknowledgement_inside_window() ) {
			TASK_set_activity_count();
			TASK_delete_acknowledged_packets();
			TASK_record_send_credit();
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
			if ( ! link_ok ) {
				link_ok = TRUE;
				OUTPUT( link_ready );
			}
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
			if ( all_transmitted_packet_acknowledged() ) {
				TIMESTOP( retry_timer );
				TASK_set_retry_count();
				TASK_store_ackd_rx_seq_number();
				EXIT( ready );
			}
			else {
				if ( repeated_link_acknowledge() ) {
					TASK_decrement_retry_count();
					if ( retry_count_zero() ) {
						TASK_maximise_link_parameters();
						OUTPUT( link_failure );
						PROCEDURE_LRT();
						EXIT( reset_wait );
					}
					else {
						if ( repeat_ok() ) {
							TASK_rewind_packet_number();
							OUTPUT( link_transfer );
							TIMESTART( retry_timer, T1, &retry_timeout );
							EXIT( ready );
						}
					}
				}
			}
		}
		else {
			TASK_maximise_link_parameters();
			OUTPUT( link_failure );
			PROCEDURE_LRT();
			EXIT( reset_wait );
		}
		break;	
		
	case INPUT_link_request:
		TASK_adjust_link_parameters();
		OUTPUT( link_failure );
		PROCEDURE_LRT();
		EXIT( reset_wait );
		break;

	case INPUT_power_on:
		TASK_initialise_rs232_port();
		TASK_maximise_link_parameters();
		OUTPUT( link_failure );
		PROCEDURE_LRT();
		EXIT( reset_wait );
		break;

	case INPUT_network_layer_reset:
		TASK_initialise_rs232_port();
		TASK_maximise_link_parameters();
		PROCEDURE_LRT();
		EXIT( reset_wait );
		break;

	case INPUT_link_transfer:
		TASK_set_activity_count();
		if ( packet_outside_window() ) {
			TASK_maximise_link_parameters();
			OUTPUT( link_failure );
			PROCEDURE_LRT();
			EXIT( reset_wait );
		}
		else {
			if ( packet_out_of_sequence() ) {
				TIMESTOP( acknowledgement_timer );
				OUTPUT( link_acknowledge );
				TIMESTART( activity_timer, T3, &activity_timeout );
			}
			else {
				if ( receive_credit_available() ) {
					OUTPUT( network_layer_packet );
					TASK_increment_receive_state();
					TASK_decrement_receive_credit();
					if ( immediate_reply_requested() ) {
						TASK_store_receive_credit();
						TIMESTOP( acknowledgement_timer );
						OUTPUT( link_acknowledge );
						TIMESTART( activity_timer, T3, &activity_timeout );
					}
					else {
						TASK_store_receive_credit();
						TIMESTART( acknowledgement_timer, T2, &acknowledgement_timeout );
					}
				}	
				else {
					TASK_store_receive_credit();
					TIMESTOP( acknowledgement_timer );
					OUTPUT( link_acknowledge );
					TIMESTART( activity_timer, T3, &activity_timeout );
				}
			}		
			EXIT( ready );
		}				
		break;

	case INPUT_acknowledgement_timeout:
		OUTPUT( link_acknowledge );
		TIMESTART( activity_timer, T3, &activity_timeout );
		EXIT( ready );
		break;

	case INPUT_activity_timeout:
		TASK_decrement_activity_count();
		if ( activity_count_zero() ) {
			TASK_maximise_link_parameters();
			OUTPUT( link_failure );
			PROCEDURE_LRT();
			EXIT( reset_wait );
		}
		else {
			TIMESTOP( acknowledgement_timer );
			OUTPUT( link_acknowledge );
			TIMESTART( activity_timer, T3, &activity_timeout );
			EXIT( ready );
		}
		break;
		
	case INPUT_credit_value:
		TASK_update_receive_credit();
		if ( receive_credit_changed() ) {
			OUTPUT( link_acknowledge );
			TIMESTART( activity_timer, T3, &activity_timeout );
		}
		EXIT( ready );
		break;
		
	default:
		break;
		
	}
	
}

/*---------------------------------------------------------------------------*/




void PROCESS_data_link( void )
{
	switch( DL_STATE ) {

	case reset_wait:
		STATE_reset_wait();
		break;

	case link_wait:
		STATE_link_wait();
		break;

	case ready:
		STATE_ready();
		break;

	default:
		break;

	}
}


/*---------------------------------------------------------------------------*/


void DL_init( void )
{
	DL_STATE = reset_wait;
	DL_recv_buff_init();
	tx_messages_buffer_init();
	link_signals = 0; sig( SIGNAL_LINK_FAIL );

	network_layer_reset_request =
	network_credit_value_request =
	network_credit_value =
	network_layer_packet_request = FALSE;

	let_expired =
	retry_timeout =
	acknowledgement_timeout =
	activity_timeout = FALSE;
	LT_ack_request = 1;


	last_transmitted_Nk = 1;
	last_transmitted_NS =
	last_transmitted_NR = 1;	
		
	last_received_NS =
	last_received_NR = last_but_one_received_NR = 0;	


	signal_power_on = TRUE;		/* START LINK */

	link_ok = FALSE;
}

/*---------------------------------------------------------------------------*/



