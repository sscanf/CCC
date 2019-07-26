

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_TIM.H
*
*  PURPOSE:		MAP27 Timer
*
*  REMARKS:		
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		18.6.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#define NO_OF_TIMERS	5

typedef enum { 				/* Timers */

	link_establishment_timer, 
	retry_timer,
	acknowledgement_timer,
	activity_timer,

	response_timer
};	


/*---------------------------------------------------------------------------*/


#define MS100		300		/* x 55ms */
#define MS400		800
#define MS300		600

/*---------------------------------------------------------------------------*/









/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


void TIMESTART( int timer, int timeout_sec, volatile BOOLEAN *timeout );
void TIMESTOP( int timer );


/*---------------------------------------------------------------------------*/


void timer_init( void );
void timer_restore( void );

/*---------------------------------------------------------------------------*/


