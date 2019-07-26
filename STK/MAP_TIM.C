

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
#define LINT_ARGS

#include "map27.h"
#include "map_tim.h"
#include "v25lib.h"

/*---------------------------------------------------------------------------*/


#define TICKS		18		/* ticks/s */
#define TIMVECT		0x08

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
   cli();
	if ( timeout_sec < 100 )
		timeout_sec = timeout_sec * TICKS;
	else 
		timeout_sec = timeout_sec / 100;

	timers[timer].counter = timeout_sec;
	timers[timer].timeout = (BOOLEAN *) timeout;
	*(timers[timer].timeout) = FALSE;
	sti();
}


void TIMESTOP( int timer )
{
	cli();
	timers[timer].counter = 0;
	*(timers[timer].timeout) = FALSE;
	sti();
}

/*---------------------------------------------------------------------------*/


/******************************************************************************
*
*	*** SERVICE PROVIDER ***
*
******************************************************************************/


void timer_isr( void ) /* TIMER ISR */
{
	int timer;
	
	for ( timer=0; timer<NO_OF_TIMERS; timer++ )
		if ( timers[timer].counter >0 ) 
			if ( --(timers[timer].counter) == 0 ) 
				*(timers[timer].timeout) = TRUE;
}

/*---------------------------------------------------------------------------*/

static struct tcb tcb0 = {
	  0x4444,            /* this will be set by init   TM reg.     */
	  0x3333,            /* this will be set by init   MD reg.     */
	  V_TCLK,            /* TMC reg.    */
	  V_IPR4,            /* set priority         TMICA  reg.  */
	  V_IMK,             /* disable this interrupt     TMICB  reg.  */
	  0,                 /*  TMMSA  reg.  */
	  0,                 /*  TMMSB  reg.  */
	  0,                 /*  mscpa    */
	  NULL,              /*  mscpb    */
	  timer_isr,         /*  timinta     */
	  NULL,              /*  timintb     */
	  NULL               /*  rbsstack    */
};

void timer_init( void )
{
	int timer;
	unsigned int rc;

	
	for ( timer=0; timer<NO_OF_TIMERS; timer++ ) {
		timers[timer].counter = 0;
		timers[timer].timeout = &null_timer;
	}

	rc = timinit(0, &tcb0, 18, V_MSEC, 0, 0);
	rc = timstart(0);
}

/*---------------------------------------------------------------------------*/


