

/******************************************************************************
*
*  PROJECT:		R40 DISPATCHER / NOKIA TELECOMMUNICATIONS
*	
*  FILE:		MAP_SYS.C
*
*  PURPOSE:		MAP27 System
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
#define LINT_ARGS
#define RSTACK_SIZE   0x600       /*stacksize for reg. bank switch */
											/* stack         */


#include "map27.h"
#include "map_sys.h"
#include "map_net.h"
#include "map_nets.h"
#include "map_dl.h"
#include "map_rs.h"
#include "map_modl.h"
#include "map_mod.h"
#include "map_tim.h"
#include "v25lib.h"


/*---------------------------------------------------------------------------*/
extern void seint (void);
extern void stinti (void);
extern void RS_receive_isr (char);
extern void RS_receive_CGC (char);

void InitPorts();
void InitCom();
void InitSys();

extern int far *rbsstack;     /* pointer to stackspace for reg. bank switch   */

void MAP_scheduler( void )		/* SDL Scheduler */
{
	/* MAP27 */

	PROCESS_data_link();
	PROCESS_network();

	PROCESS_modem_link();
}


/*---------------------------------------------------------------------------*/


void MAP_init( void ) /* Initialize system */
{
	RS_init();
	MOD_init();
	DL_init();
	NET_init();
	sti();
}


/*---------------------------------------------------------------------------*/




/***************** RUTINAS DE INICIALIZACION PARA EL V25 ********************/

static unsigned int mib[][2] = {
	  {0x40, 0}, /* Default data segment will be mapped there by LOCATE */
	  {0x0000, 0x0000}  /* list terminator                */
};

void InitSys (void)
{
	  static struct ccb ccb;

	  ccb.wtc.i = 0xffff;
	  ccb.prc.c = 0x08;
	  ccb.rfm.c = 0xfc;
	  ccb.idb   = 0xff;

	  sysinit(16000, &ccb, *mib,V_HALT,V_HALT);
}

void InitPorts()
{
	int rc;

	static struct pcb pcb;

	 pcb.pmc0.c = 0;
	 pcb.pm0.c  = 0;

	 pcb.pmc1.c = V_SCK0 + V_TOUT;
	 pcb.pm1.c = 0xff;

	 pcb.pmc2.c = 0;
	 pcb.pm2.c  = 0;
	 portinit(&pcb);
}

static struct scb scb1 = {

	  V_ASYNC + V_NOPAR + V_RXE + V_TXE,  /* SCM    */
	  0x04,                       /* SCC    */
	  0,                          /* BRG    */
	  V_IPR3,                     /* SEIC      */
	  0x07,                       /* SRIC      */
	  0x07,                       /* STIC      */
	  0,                          /* SRMS      */
	  0,                          /* STMS      */
	  NULL,                       /* mscpr     */
	  NULL,                       /* mscpt     */
	  seint,                      /* errint    */
	  RS_receive_isr,             /* recint    */
	  stinti,                     /* traint    */
	  NULL                        /* rbsstack  */
};

static struct scb scb0 = {

	  V_ASYNC + V_NOPAR + V_RXE + V_TXE,  /* SCM    */
	  0x04,                       /* SCC    */
	  0,                          /* BRG    */
	  V_IPR3,                     /* SEIC      */
	  0x07,                       /* SRIC      */
	  0x07,                       /* STIC      */
	  0,                          /* SRMS      */
	  0,                          /* STMS      */
	  NULL,                       /* mscpr     */
	  NULL,                       /* mscpt     */
	  seint,                      /* errint    */
	  RS_receive_CGC,             /* recint    */
	  stinti,                     /* traint    */
	  NULL                        /* rbsstack  */
};


void InitCom (void)
{
	  unsigned int rc;
	  scb1.rbsstack = rbsstack+RSTACK_SIZE;
	  scb0.rbsstack = rbsstack+RSTACK_SIZE;

	  rc = cominit(0, &scb0, 9600, 'N', 8, 1);
	  rc = comtdisa(0);
	  rc = comtstart(0);

	  rc = cominit(1, &scb1, 9600, 'N', 8, 1);
	  rc = comtdisa(1);
     rc = comtstart(1);
}
