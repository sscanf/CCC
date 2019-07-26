

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_NET.C
*
*  PURPOSE:		MAP27 Network Layer
*
*  REMARKS:		
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		6.10.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#include <stdio.h>
#include <string.h>

#include "map27.h"
#include "map_net.h"
#include "map_nets.h"
#include "map_dl.h"
#include "map_modl.h"
#include "map_tim.h"

/*---------------------------------------------------------------------------*/

#define sig( s )	net_signals |= s
#define reset_sig( s )	net_signals &= ~s
#define is_sig( s )	(net_signals & s) != 0

/*---------------------------------------------------------------------------*/

#define T4		70		/* RESPONSE TIMEOUT */
#define T6		3		/* disconnect timeout */

#define NWB_SIZE	10

/*---------------------------------------------------------------------------*/


#define EXIT( p )	NW_STATE = p


typedef enum { link_down, idle, message_wait, outcall_wait, active, disconnect_wait };	/* PROCESS */


typedef enum {				/* INPUT */

	INPUT_response_timeout,

	INPUT_SendA_Status_SST_MST,
	INPUT_SetupA_Voice_Modem,
	INPUT_SetupA_Emergency,
	INPUT_SendA_Disconnect,
	INPUT_ReceiveA_Reset,
	INPUT_CancelA_Message,
	INPUT_RadioA_Interrogation,
	INPUT_RadioA_Management,

	INPUT_link_ready,
	INPUT_ReceiveD_Reset,
	INPUT_packet_accepted,
	INPUT_packet_rejected,

	INPUT_ReceiveR_Status_SST_MST,
	INPUT_MessageR_Success_ACK,
	INPUT_MessageR_Int_ACK,
	INPUT_MessageR_Fail_ACK,
	INPUT_MessageR_Cancelled,
	INPUT_ReceiveR_Cleared,
	INPUT_SetupR_Progress_Int,
	INPUT_SetupR_Progress_OK,
	INPUT_SetupR_Progress_Fail,

	INPUT_RadioR_Personality,
	INPUT_NumberingR_Info,
	INPUT_OperR_Condition,
	INPUT_RadioR_Settings,
	INPUT_ProtocolR_Info,

	INPUT_MessageM_Success_ACK,
	INPUT_MessageM_Fail_ACK,

	INPUT_NONE,
	INPUT_Incoming_Voice_Modem

};


typedef enum {				/* OUTPUT */

	network_ready,
	network_fail,
	pass_to_appl,

	network_layer_reset,	
	credit,			
	link_layer_packet,

	enable_modem,
	disable_modem
};


#define info_appl	pass_to_appl


/*---------------------------------------------------------------------------*/


typedef struct {
	int len;
	OCTET msg[ MAX_DATA_LEN ];
} 
NET_BUFF;

typedef struct {
	int len;
	OCTET msg[ MAX_DATA_LEN ];
} 
APPL_BUFF;

/*---------------------------------------------------------------------------*/


typedef struct {
	int len;
	OCTET message_type;
	OCTET a[3];
	OCTET cause;
}
DISCONNECT_MESSAGE;

		
/*****************************************************************************/


int NW_STATE;				/* PROCESS */
int NW_INPUT;

WORD net_signals;
int send_STATE;

volatile BOOLEAN response_timeout;

BYTE rec_credit;


BOOLEAN appl_reset_request;

/*---------------------------------------------------------------------------*/


NET_BUFF net_packet;	

NET_BUFF NW_recv_buff[ NWB_SIZE ];	/* TO APPL */
int NW_recv_qin, NW_recv_qout;

/*---------------------------------------------------------------------------*/


APPL_BUFF appl_packet;	

APPL_BUFF NW_send_buff[ NWB_SIZE ];	/* FROM APPL */
int NW_send_qin, NW_send_qout;

/*---------------------------------------------------------------------------*/


DISCONNECT_MESSAGE disco;

BOOLEAN modem_call;
BOOLEAN modem_success;


/*****************************************************************************/


void NW_recv_buff_dequeue( NET_BUFF *m );
BOOLEAN NW_recv_buff_empty( void );
void NW_send_buff_enqueue( APPL_BUFF *m );



/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


BOOLEAN receive_network_message( GENERIC_NETWORK_MESSAGE *msg )
{
	if ( ! NW_recv_buff_empty() ) {
		NW_recv_buff_dequeue( (NET_BUFF *) msg );
		return TRUE;
	}
	else
		return FALSE;		/* no data */
}


BOOLEAN receive_network_signal( int s )
{
	if ( net_signals == 0 )
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


void send_network_signal( int s )
{
	switch ( s ) {

	case SIGNAL_NETWORK_LAYER_RESET:
		appl_reset_request = TRUE;
		break;

	default:
		break;
	}		
}


BOOLEAN send_network_message( GENERIC_APPL_MESSAGE *m )
{
	APPL_BUFF t;
	int i;


	if ( NW_STATE == link_down )
		return FALSE;
	

	switch ( m->message_type ) {

	case SEND_STATUS:
	case SEND_SST:
	case SEND_MST:
	case SETUP_VOICE_MODEM:
	case SETUP_EMERGENCY:
		if ( ! ( send_STATE == SEND_IDLE || 
			send_STATE == SEND_OK || send_STATE == SEND_FAIL ) )
			return FALSE;	/* appl error ??? */
		else
			send_STATE = SEND_PENDING;
		break;

	case DISCONNECT:
	case DISCONNECT_C:
	case RADIO_INTERROGATION:
	case RADIO_MANAGEMENT:
		break;

	default:
		return FALSE;		/* unknown message type ??? */
	}

	modem_call = FALSE;


	t.msg[0] = m->message_type;
	for ( i=0; i<m->len; i++ )	/* copy message */
		t.msg[i+1] = m->msg[i];
	t.len = m->len +1;

	NW_send_buff_enqueue( &t );


	return TRUE;			/* message accepted */
}


BOOLEAN send_long_network_message( CALL_SETUP_MESSAGE *m, FILE *fp, BOOLEAN new )
{
	if ( m->message_type != SETUP_VOICE_MODEM )
		return FALSE;

	if ( send_network_message( m ) ) {
		send_modem_message( fp, new );
		modem_call = TRUE;
		modem_success = FALSE;
		return TRUE;
	}
	else
		return FALSE;
}


int receive_network_status( void )
{
	int t = send_STATE;


	if ( (send_STATE == SEND_OK) || (send_STATE == SEND_FAIL) )
		send_STATE = SEND_IDLE;	/* reset */
		
	return t;
}


/*---------------------------------------------------------------------------*/












/******************************************************************************
*
*	*** SERVICE PROVIDER ***
*
******************************************************************************/


void appl_signal( int s )		/* Notify applications */
{
	switch ( s ) {

	case network_ready:
		sig( SIGNAL_NETWORK_READY );
		break;
		
	case network_fail:
		reset_sig( SIGNAL_NETWORK_READY );	
		sig( SIGNAL_NETWORK_FAIL );
		break;

	default:
		break;
	}
}			

/*---------------------------------------------------------------------------*/


/* MESSAGES TO APPLICATION */


void NW_recv_buff_enqueue( NET_BUFF *m )
{
	int i;
	int b = NW_recv_qin;
	
		
	for ( i=0; i<m->len; i++ )
		NW_recv_buff[b].msg[i] = m->msg[i];
	NW_recv_buff[b].len = m->len;

	if ( ++NW_recv_qin == NWB_SIZE ) NW_recv_qin = 0;
}


void NW_recv_buff_dequeue( NET_BUFF *m )
{
	int i;
	int b = NW_recv_qout;
	
	
	for ( i=0; i<NW_recv_buff[b].len; i++ )
		m->msg[i] = NW_recv_buff[b].msg[i];
	m->len = NW_recv_buff[b].len -1;

	if ( ++NW_recv_qout == NWB_SIZE ) NW_recv_qout = 0;
}


BOOLEAN NW_recv_buff_empty( void )
{
	if ( NW_recv_qin == NW_recv_qout )
		return TRUE;
	else
		return FALSE;		
}


void NW_recv_buff_init( void )
{
	NW_recv_qin = NW_recv_qout = 0;
}



/* MESSAGES FROM APPLICATION */


void NW_send_buff_enqueue( APPL_BUFF *m )
{
	int i;
	int b = NW_send_qin;
	
		
	for ( i=0; i<m->len; i++ )
		NW_send_buff[b].msg[i] = m->msg[i];
	NW_send_buff[b].len = m->len;

	if ( ++NW_send_qin == NWB_SIZE ) NW_send_qin = 0;
}


void NW_send_buff_dequeue( APPL_BUFF *m )
{
	int i;
	int b = NW_send_qout;
	
	
	for ( i=0; i<NW_send_buff[b].len; i++ )
		m->msg[i] = NW_send_buff[b].msg[i];
	m->len = NW_send_buff[b].len;

	if ( ++NW_send_qout == NWB_SIZE ) NW_send_qout = 0;
}


BOOLEAN NW_send_buff_empty( void )
{
	if ( NW_send_qin == NW_send_qout )
		return TRUE;
	else
		return FALSE;		
}


void NW_send_buff_init( void )
{
	NW_send_qin = NW_send_qout = 0;
}


/*---------------------------------------------------------------------------*/


void send_appl_message( NET_BUFF *msg )
{
	NW_recv_buff_enqueue( msg );
}

/*---------------------------------------------------------------------------*/


BOOLEAN receive_appl_message( APPL_BUFF *msg )
{
	if ( ! NW_send_buff_empty() ) {
		NW_send_buff_dequeue( msg );
		return TRUE;
	}
	else
		return FALSE;		/* no data */
}

/*---------------------------------------------------------------------------*/


int ACK_cause( void )
{
	return net_packet.msg[ net_packet.len-1 ];	/* last octet = CAUSE */
}	

/*****************************************************************************/



int INPUT( void )
{

	if ( response_timeout ) {
		if ( NW_STATE != active ) {
			response_timeout = FALSE;
			return INPUT_response_timeout;
		}
	}



	/* FROM APPLICATION */

	if ( receive_appl_message( &appl_packet ) ) {

		switch( ((GENERIC_APPL_MESSAGE*) &appl_packet)->message_type ) {

		case SEND_STATUS:
		case SEND_SST:
		case SEND_MST:
			send_STATE = SEND_IN_SERVICE;
			return INPUT_SendA_Status_SST_MST;

		case SETUP_VOICE_MODEM:
			send_STATE = SEND_IN_SERVICE;
			return INPUT_SetupA_Voice_Modem;

		case SETUP_EMERGENCY:
			send_STATE = SEND_IN_SERVICE;
			return INPUT_SetupA_Emergency;

		case DISCONNECT:
		case DISCONNECT_C:
			return INPUT_SendA_Disconnect;

		case RADIO_INTERROGATION:
			return INPUT_RadioA_Interrogation;

		case RADIO_MANAGEMENT:
			return INPUT_RadioA_Management;

		default:
			break;
		}						
	}



	if ( appl_reset_request ) {
		appl_reset_request = FALSE;
		return INPUT_ReceiveA_Reset;
	}

	
	
	/* FROM DATA LINK */
	
	if ( receive_link_signal( SIGNAL_LINK_READY ) )
		return INPUT_link_ready;

	if ( receive_link_signal( SIGNAL_LINK_FAIL ) )
		return INPUT_ReceiveD_Reset;

	if ( receive_link_signal( SIGNAL_LINK_PACKET_ACK ) )	//???
		return INPUT_packet_accepted;

	if ( receive_link_signal( SIGNAL_LINK_PACKET_REJ ) )	//???
		return INPUT_packet_rejected;


	if ( receive_link_packet( net_packet.msg, &net_packet.len ) ) {

		rec_credit = DL_recv_buff_status();
		send_link_signal( SIGNAL_CREDIT_VALUE, rec_credit );	

		
		switch( ((GENERIC_NETWORK_MESSAGE *) &net_packet)->message_type ) {

		case RECEIVE_STATUS:
		case RECEIVE_SST:
		case RECEIVE_MST:
			return INPUT_ReceiveR_Status_SST_MST;


		case INCOMING_VOICE_MODEM:
			return INPUT_Incoming_Voice_Modem;


		case STATUS_SST_MST_ACKP:
			return INPUT_MessageR_Success_ACK;

		case STATUS_SST_MST_ACKQ:
			return INPUT_MessageR_Int_ACK;

		case STATUS_SST_MST_ACKN:
			return INPUT_MessageR_Fail_ACK;


		case CLEARED_N:
		case CLEARED_A:
			return INPUT_ReceiveR_Cleared;


		case SETUP_PROGRESS_Q:
			return INPUT_SetupR_Progress_Int;

		case SETUP_PROGRESS_P:
			return INPUT_SetupR_Progress_OK;

		case SETUP_PROGRESS_N:
			return INPUT_SetupR_Progress_Fail;

		case RADIO_PERSONALITY:
			return INPUT_RadioR_Personality;

		case NUMBERING_INFO:
			return INPUT_NumberingR_Info;

		case OPERATING_CONDITION:
			return INPUT_OperR_Condition;

		case RADIO_SETTINGS:
			return INPUT_RadioR_Settings;

		case PROTOCOL_INFO:
			return INPUT_ProtocolR_Info;

		default:
			//??? UNKNOWN MESSAGE TYPE
			break;
		}
	}



	/* FROM MODEM LINK */

	if ( receive_modem_signal( SIGNAL_MODEM_SEND_OK ) )	
		return INPUT_MessageM_Success_ACK;

	if ( receive_modem_signal( SIGNAL_MODEM_SEND_FAIL ) )	
		return INPUT_MessageM_Fail_ACK;



	return INPUT_NONE;			
}


/*---------------------------------------------------------------------------*/


void OUTPUT( int p )
{
	switch( p ) {


	/* TO APPLICATIONS */

	case network_ready:
	case network_fail:
		appl_signal( p );
		break;

	case pass_to_appl:
		send_appl_message( &net_packet );
		break;



	/* TO DATA LINK */

	case network_layer_reset:	
		send_link_signal( SIGNAL_NETWORK_LAYER_RESET, 0 );
		break;

	case credit:			
		send_link_signal( SIGNAL_CREDIT_VALUE, rec_credit );
		break;
		
	case link_layer_packet:
		send_link_packet( appl_packet.msg, appl_packet.len ); 	
		break;	



	/* TO MODEM LINK */

	case enable_modem:
		send_modem_signal( SIGNAL_MODEM_ENABLE );
		break;

	case disable_modem:
		send_modem_signal( SIGNAL_MODEM_DISABLE );
		break;


	default:
		break;
	}

}


/*---------------------------------------------------------------------------*/

/* TASKs */


void TASK_inform_appl( void )
{
	send_STATE = SEND_IN_PROGRESS;
}


void TASK_inform_last( BOOLEAN ok )
{
	if ( ok )
		send_STATE = SEND_OK;
	else
		send_STATE = SEND_FAIL;		
}


void TASK_inform_modem_last( BOOLEAN ok )
{
	modem_success = ok;
}


void TASK_cancel_all( void )
{
	send_STATE = SEND_FAIL;
}


void TASK_init_all( void )
{
	NET_init();			
	//???
}


void TASK_appl_error( void )
{
	//???
}		


void TASK_pass_abort( void )
{
	send_STATE = SEND_FAIL;
}


void TASK_pass_disconn( void )
{
	if ( modem_call ) {
		if ( modem_success )
			send_STATE = SEND_OK;
		else
			send_STATE = SEND_FAIL;
	}
	else
		send_STATE = SEND_OK;
}


void TASK_disconnect( void )
{
	disco.message_type = DISCONNECT;
	/* NOTE: address copied from the setup message */
	disco.cause = MAINT;
	disco.len = 5;

	NW_send_buff_enqueue( (APPL_BUFF *) &disco );
}


void TASK_disconnect_cancel( void )
{
	disco.message_type = DISCONNECT_C;
	/* NOTE: address copied from the send message */
	disco.cause = modem_call ? RQX4 : RQX1;		// ???
	disco.len = 5;

	NW_send_buff_enqueue( (APPL_BUFF *) &disco );
}


void TASK_clear_send( void )
{
	if ( ! ( send_STATE == SEND_OK || send_STATE == SEND_FAIL ) )
		send_STATE = SEND_FAIL;
}


void TASK_store_address( void )
{
	int i;

	for ( i=0; i<3; i++ )
		disco.a[i] = appl_packet.msg[i+1];
}



/*---------------------------------------------------------------------------*/


/* IFs */

BOOLEAN is_modem_call( void ) 
{
	return modem_call;
}


/*****************************************************************************/



void STATE_link_down( void )
{

	switch( NW_INPUT = INPUT() ) {

	case INPUT_link_ready:			// from data link
		OUTPUT( network_ready );	// to applications
		EXIT( idle );
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void STATE_idle( void )
{

	switch( NW_INPUT = INPUT() ) {

	case INPUT_ReceiveR_Status_SST_MST:
		OUTPUT( pass_to_appl );
		EXIT( idle );
		break;

	case INPUT_SendA_Status_SST_MST:
		TASK_store_address();		/* for DISCONNECT */
		OUTPUT( link_layer_packet );	/* send */
		TIMESTART( response_timer, T4, &response_timeout );	
		EXIT( message_wait );
		break;		

	case INPUT_SetupA_Voice_Modem:
	case INPUT_SetupA_Emergency:
		TASK_store_address();		/* for DISCONNECT */
		OUTPUT( link_layer_packet );	/* setup call */
		TIMESTART( response_timer, T4, &response_timeout );	
		EXIT( outcall_wait );
		break;		

	case INPUT_SendA_Disconnect:
		OUTPUT( link_layer_packet );
		EXIT( idle );
		break;

	case INPUT_ReceiveA_Reset:				
		OUTPUT( network_layer_reset );
		TASK_init_all();
		EXIT( idle );
		break;

	case INPUT_RadioA_Interrogation:
		OUTPUT( link_layer_packet );	/* send interr */
		EXIT( idle );
		break;

	case INPUT_RadioA_Management:
		OUTPUT( link_layer_packet );	/* send manag */
		EXIT( idle );
		break;

	case INPUT_ReceiveD_Reset:		
		TASK_cancel_all();		/* reset all */
		OUTPUT( network_fail );		
		EXIT( link_down );
		break;

	case INPUT_RadioR_Personality:
		OUTPUT( info_appl );		
		EXIT( idle );
		break;

	case INPUT_NumberingR_Info:
		OUTPUT( info_appl );		
		EXIT( idle );
		break;

	case INPUT_OperR_Condition:
		OUTPUT( info_appl );		
		EXIT( idle );
		break;

	case INPUT_RadioR_Settings:
		OUTPUT( info_appl );		
		EXIT( idle );
		break;

	case INPUT_ProtocolR_Info:
		OUTPUT( info_appl );
		EXIT( idle );
		break;

	case INPUT_Incoming_Voice_Modem:
		OUTPUT( info_appl );
		EXIT( idle );
		break;

	case INPUT_ReceiveR_Cleared:
		OUTPUT( info_appl );
		EXIT( idle );
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void STATE_message_wait( void )
{

	switch( NW_INPUT = INPUT() ) {

	case INPUT_ReceiveR_Status_SST_MST:
		OUTPUT( pass_to_appl );
		EXIT( message_wait );
		break;

	case INPUT_SendA_Status_SST_MST:	
		TASK_appl_error();		
		EXIT( message_wait );
		break;
		
	case INPUT_MessageR_Int_ACK:
		TASK_inform_appl();
		TIMESTART( response_timer, T4, &response_timeout );	/* restart */	
		EXIT( message_wait );
		break;

	case INPUT_MessageR_Success_ACK:
		TIMESTOP( response_timer );	
		TASK_inform_last( TRUE );
		EXIT( idle );
		break;

	case INPUT_MessageR_Fail_ACK:
		TIMESTOP( response_timer );	
		TASK_inform_last( FALSE );
		EXIT( idle );
		break;

	case INPUT_CancelA_Message:		
		//???				/* cancel mess */
		EXIT( idle );
		break;

	case INPUT_SendA_Disconnect:
		TIMESTOP( response_timer );	
		OUTPUT( link_layer_packet );
		TASK_cancel_all();
		EXIT( idle );
		break;
		
	case INPUT_ReceiveR_Cleared:
		TIMESTOP( response_timer );	
		TASK_pass_abort();
		EXIT( idle );
		break;
		
	case INPUT_ReceiveA_Reset:				
		TIMESTOP( response_timer );	
		OUTPUT( network_layer_reset );
		TASK_init_all();
		EXIT( idle );
		break;

	case INPUT_RadioA_Interrogation:
		OUTPUT( link_layer_packet );	/* send interr */
		EXIT( message_wait );
		break;

	case INPUT_RadioA_Management:
		OUTPUT( link_layer_packet );	/* send manag */
		EXIT( message_wait );
		break;

	case INPUT_response_timeout:
		TASK_disconnect_cancel();	/* DISCONNECT */
		TASK_cancel_all();
		EXIT( idle );
		break;

	case INPUT_ReceiveD_Reset:		
		TIMESTOP( response_timer );	
		TASK_cancel_all();		/* reset all */	
		OUTPUT( network_fail );		
		EXIT( link_down );
		break;

	case INPUT_RadioR_Personality:
		OUTPUT( info_appl );		
		EXIT( message_wait );
		break;

	case INPUT_NumberingR_Info:
		OUTPUT( info_appl );		
		EXIT( message_wait );
		break;

	case INPUT_OperR_Condition:
		OUTPUT( info_appl );		
		EXIT( message_wait );
		break;

	case INPUT_RadioR_Settings:
		OUTPUT( info_appl );		
		EXIT( message_wait );
		break;

	case INPUT_ProtocolR_Info:
		TIMESTOP( response_timer );	
		TASK_cancel_all();
		OUTPUT( info_appl );		
		EXIT( idle );
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void STATE_outcall_wait( void )
{

	switch( NW_INPUT = INPUT() ) {

	case INPUT_ReceiveR_Status_SST_MST:
		OUTPUT( pass_to_appl );
		EXIT( outcall_wait );
		break;

	case INPUT_SetupR_Progress_Int:
		/* TASK_inform_appl(); */
		TIMESTART( response_timer, T4, &response_timeout );	/* restart */
		EXIT( outcall_wait );
		break;

	case INPUT_SetupR_Progress_OK:
		TIMESTOP( response_timer );
		TASK_inform_appl();		/* inform last */
		if ( is_modem_call() ) {
			OUTPUT( enable_modem );		/* START SENDING */
			TIMESTART( response_timer, T5, &response_timeout );
		}
		EXIT( active );
		break;

	case INPUT_SetupR_Progress_Fail:
		TIMESTOP( response_timer );
		TASK_inform_last( FALSE );
		EXIT( idle );
		break;

	case INPUT_SendA_Disconnect:
		OUTPUT( link_layer_packet );
		TASK_cancel_all();
		EXIT( idle );
		break;

	case INPUT_ReceiveR_Cleared:
		TIMESTOP( response_timer );
		TASK_pass_abort();
		EXIT( idle );
		break;

	case INPUT_ReceiveA_Reset:
		OUTPUT( network_layer_reset );
		TASK_init_all();
		EXIT( idle );
		break;

	case INPUT_RadioA_Interrogation:
		OUTPUT( link_layer_packet );	/* send interr */
		EXIT( outcall_wait );
		break;

	case INPUT_RadioA_Management:
		OUTPUT( link_layer_packet );	/* send manag */
		EXIT( outcall_wait );
		break;

	case INPUT_ReceiveD_Reset:
		TIMESTOP( response_timer );
		TASK_cancel_all();		/* reset all */
		OUTPUT( network_fail );
		EXIT( link_down );
		break;

	case INPUT_RadioR_Personality:
		OUTPUT( info_appl );
		EXIT( outcall_wait );
		break;

	case INPUT_NumberingR_Info:
		OUTPUT( info_appl );
		EXIT( outcall_wait );
		break;

	case INPUT_OperR_Condition:
		OUTPUT( info_appl );
		EXIT( outcall_wait );
		break;

	case INPUT_response_timeout:
		TASK_disconnect_cancel();	/* DISCONNECT */
		TASK_cancel_all();
		EXIT( idle );
		break;

	case INPUT_RadioR_Settings:
		OUTPUT( info_appl );
		EXIT( outcall_wait );
		break;

	case INPUT_ProtocolR_Info:
		TIMESTOP( response_timer );
		TASK_cancel_all();
		OUTPUT( info_appl );
		EXIT( idle );
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void STATE_active( void )
{

	switch( NW_INPUT = INPUT() ) {

//****************************************

	case INPUT_MessageM_Success_ACK:	/* file sent OK */
		TASK_disconnect();
		TASK_inform_modem_last( TRUE );
		EXIT( active );
		break;

	case INPUT_MessageM_Fail_ACK:		/* failed */
		TASK_disconnect();
		TASK_inform_modem_last( FALSE );
		EXIT( active );
		break;

//****************************************

	case INPUT_ReceiveR_Status_SST_MST:
		OUTPUT( pass_to_appl );
		EXIT( active );
		break;

	case INPUT_SendA_Disconnect:		/* disconnect */
		TIMESTOP( response_timer );
		OUTPUT( link_layer_packet );
		OUTPUT( disable_modem );
		TIMESTART( response_timer, 2*T6, &response_timeout );
		EXIT( disconnect_wait );
		break;

	case INPUT_ReceiveR_Cleared:		/* disconnect */
		TIMESTOP( response_timer );
		OUTPUT( disable_modem );
		TIMESTART( response_timer, 2*T6, &response_timeout );
		EXIT( disconnect_wait );
		break;

	case INPUT_ReceiveA_Reset:
		OUTPUT( network_layer_reset );
		TASK_init_all();
		OUTPUT( disable_modem );
		EXIT( idle );
		break;

	case INPUT_RadioA_Interrogation:
		OUTPUT( link_layer_packet );	/* send interr */
		EXIT( active );
		break;

	case INPUT_RadioA_Management:
		OUTPUT( link_layer_packet );	/* send manag */
		EXIT( active );
		break;

	case INPUT_ReceiveD_Reset:
		OUTPUT( network_fail );
		TASK_clear_send();		/* reset all */
		EXIT( link_down );
		break;

	case INPUT_RadioR_Personality:
		OUTPUT( info_appl );
		EXIT( active );
		break;

	case INPUT_NumberingR_Info:
		OUTPUT( info_appl );
		EXIT( active );
		break;

	case INPUT_OperR_Condition:
		OUTPUT( info_appl );
		EXIT( active );
		break;

	case INPUT_RadioR_Settings:
		OUTPUT( info_appl );
		EXIT( active );
		break;

	case INPUT_ProtocolR_Info:
		TIMESTOP( response_timer );
		TASK_clear_send();
		OUTPUT( disable_modem );
		OUTPUT( info_appl );
		EXIT( idle );
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void STATE_disconnect_wait( void )
{

	switch( NW_INPUT = INPUT() ) {

	case INPUT_ReceiveR_Cleared:		/* disconnected ok */
		TIMESTOP( response_timer );
		TIMESTART( response_timer, T6, &response_timeout );
		EXIT( disconnect_wait );	/* WAIT FOR RU READY !!! */
		break;

	case INPUT_response_timeout:
		TIMESTOP( response_timer );
		TASK_pass_disconn();
		EXIT( idle );			/* finished */
		break;

	default:
		break;

	}

}

/*---------------------------------------------------------------------------*/



void PROCESS_network( void )
{
	switch( NW_STATE ) {

	case link_down:
		STATE_link_down();
		break;

	case idle:
		STATE_idle();
		break;

	case message_wait:
		STATE_message_wait();
		break;

	case outcall_wait:
		STATE_outcall_wait();
		break;

	case active:
		STATE_active();
		break;

	case disconnect_wait:
		STATE_disconnect_wait();
		break;

	default:
		break;

	}
}


/*---------------------------------------------------------------------------*/


void NET_init( void )
{
	NW_recv_buff_init();
	NW_send_buff_init();

	NW_STATE = link_down;		//???
	send_STATE = SEND_IDLE;
	net_signals = 0;

	response_timeout = FALSE;

	appl_reset_request = FALSE;

	modem_call = FALSE;
}

/*---------------------------------------------------------------------------*/



