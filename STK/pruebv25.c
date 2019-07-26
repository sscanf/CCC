#include <y:\v25\include\v25lib.h>
#include <stdio.h>

#define LINT_ARGS
#define RSTACK_SIZE   0x100      /* stacksize for reg. bank switch */
											/* stack         */

extern void PTTON(unsigned char);
extern void INICIALCOM(void);

void InitSYS (void);
void InitPorts (void);
void InitCom (void);
void InitTimer (void);

int far *rbsstack;     /* pointer to stackspace for reg. bank switch   */
int pepe;

char text[256] = "Esto es una prueba de transmision serie\n\r";
char *stptr = text;        /* serial transmit buffer pointer */

struct msc msc;

void main (void)
{
	int rc;
	int far *puntero;

	InitSYS();
	InitPorts();
	sti();
	InitCom();
	InitTimer();

	comtdisa (1);
	comtstart (1);

	while (1);

}
/*********************** INTERRUPCION DE RECEPCION SERIE *******************/

void srint(cc)
char cc;    /* received character */
{
	comtrans (1,cc);
}

/*********************** INTERRUPCION DE TRANSMISION SERIE *****************/

unsigned int stinti()
{
	  return(0);
}

/*
 * seint
 * This function will be called on serial error interrupt.
 * We increment a counter and save the error register contents.
 */

/************************ INTERRUPCION DEL TIMER ***************************/

void tint0 ()
{
	static int cnt=0;
	unsigned int rc;

	if (cnt==0)
	{
	  rc = portwrite (0,0xaa);
	  cnt=1;
	}
	else
	{
	  rc = portwrite (0,0);
	  cnt=0;
	}
}


void seint(sce)
char sce;       /* error code */
{
}

/************************ RUTINAS DE INICIALIZACION ************************/

static unsigned int mib[][2] = {
	  {0x40, 0x050}, /* Default data segment will be mapped there by LOCATE */
	  {0x0000, 0x0000}  /* list terminator                */
};

void InitSYS (void)
{
	  static struct ccb ccb;

	  ccb.wtc.i = 0xffff;
	  ccb.prc.c = 0x0c;
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
	  srint,                      /* recint    */
	  stinti,                     /* traint    */
	  NULL                        /* rbsstack  */
};


void InitCom (void)
{
	  unsigned int rc;
	  scb1.rbsstack = rbsstack+RSTACK_SIZE;

	  rc = cominit(1, &scb1, 9600, 'N', 8, 1);
}

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
	  tint0,             /*  timinta     */
	  NULL,              /*  timintb     */
	  NULL               /*  rbsstack    */
};

void InitTimer()
{
	unsigned int rc;
	rc = timinit(0, &tcb0, 2000, V_USEC, 0, 0);
	rc = timstart(0);
}

