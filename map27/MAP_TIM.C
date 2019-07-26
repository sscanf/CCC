

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_TIM.C
*
*  PURPOSE:		MAP27 Timer
*
*  REMARKS:		IRQ0
*
*  AUTHOR:		P. Kettunen
*
*  LAST UPDATE:		8.5.1993
*
*
*		KUUMIC OY (C) 1993
*
******************************************************************************/


#include <dos.h>

#include "map27.h"
#include "map_tim.h"


/*---------------------------------------------------------------------------*/


#define TICKS		18		/* ticks/s */
#define TIMVECT		0x08

void interrupt (* orig_tim_isr ) ( void );

/*---------------------------------------------------------------------------*/


struct {
	int counter;			/* timeout counter */
	BOOLEAN *timeout;		/* timeout flag */
}
timers[ NO_OF_TIMERS ];	


BOOLEAN null_timer;


/*---------------------------------------------------------------------------*/










/******************************************************************************
*
*	*** SAP ***
*
******************************************************************************/


void TIMESTART( int timer, int timeout_sec, volatile BOOLEAN *timeout )
{
	disable();
	if ( timeout_sec < 100 )
		timeout_sec = timeout_sec * TICKS;
	else 
		timeout_sec = timeout_sec / 100;

	timers[timer].counter = timeout_sec;
	timers[timer].timeout = (BOOLEAN *) timeout;
	*(timers[timer].timeout) = FALSE;
	enable();
}


void TIMESTOP( int timer )
{
	disable();
	timers[timer].counter = 0;
	*(timers[timer].timeout) = FALSE;
	enable();
}

/*---------------------------------------------------------------------------*/










/******************************************************************************
*
*	*** SERVICE PROVIDER ***
*
******************************************************************************/


void interrupt timer_isr( void )	/* TIMER ISR */
{
	int timer;
	
	
	for ( timer=0; timer<NO_OF_TIMERS; timer++ )
		if ( timers[timer].counter >0 ) 
			if ( --(timers[timer].counter) == 0 ) 
				*(timers[timer].timeout) = TRUE;

	(*orig_tim_isr)();		/* jump to the original handler */
}

/*---------------------------------------------------------------------------*/


void timer_init( void )
{
	int timer;

	
	for ( timer=0; timer<NO_OF_TIMERS; timer++ ) {
		timers[timer].counter = 0;
		timers[timer].timeout = &null_timer;
	}

	disable();
	orig_tim_isr = getvect( TIMVECT );
	setvect( TIMVECT, timer_isr );
	enable();
}


void timer_restore( void )
{
	disable();
	setvect( TIMVECT, orig_tim_isr );
	enable();
}

/*---------------------------------------------------------------------------*/


