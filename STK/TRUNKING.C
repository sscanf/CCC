/* Data segment at 40h*/

#define LINT_ARGS

/*#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>*/
#include <v25lib.h>
#include "cgc.h"
#include <malloc.h>

#include "MAP27.H"
#include "map_sys.h"
#include "MAP_DL.H"
#include "MAP_NET.H"

#define RSTACK_SIZE   0x100       stacksize for reg. bank switch */

extern void MAP_init( void );  /* Initialize system */
extern void send_link_signal( int s, int par );

void GestioStatus(void);
void GestionConnect(void);
void SendMST (void);
void SendClear (void);
void TareaSetup(void);
void busy(void);
void empty(void);
void WatchDog(void);
void TareaPttOff (void);
void TareaPttOn  (void);
void TareaTrucSoci (void);
void TxCGC (struct MensToCGC MensajeCGC, unsigned char mensaje);

/*						0,  1,  2,  3,  4,   5,  6,  7,  8,  9*/
char tabla[10] = {0x0,0x6,0x1,0x2,0x10,0x3,0x4,0x8,0xa,0x0b};

BOOLEAN RadioManagement (char control);
void SetupVOICE (void);
void disconnect(void);


void EjecutaTarea (void);
int  SendStatus (int, char);

BYTE  flota;
WORD  ident;

int far *rbsstack;     /* pointer to stackspace for reg. bank switch   */
tareas ptrtareas;

int tarea;	/* N£mero de tareas en cola y apunta a la £ltima tarea */

char Port0;


int i;
int bsize;
char string[10];
int numr;
int reset=0;
int r;
int conectado;

GENERIC_NETWORK_MESSAGE *mens;
struct MensToCGC MensajeCGC;
struct MovilActual movil;

void main(void)
{

	InitSys();
	InitPorts();
	InitCom();
	timer_init();
	MAP_init();


	tarea=0;
	flota=0x14;
	reset=0;
	Port0=0;
	conectado=0;

	MensajeCGC.flota   = 0;
	MensajeCGC.IndiH   = 0;
	MensajeCGC.IndiL   = 0;
	TxCGC (MensajeCGC,RESET);

	movil.flota = 0;
	movil.IndiH = 0;
	movil.IndiL = 0;

	empty();

	for (;;)
	{
		WatchDog();

		MAP_scheduler ();
		if (tarea) EjecutaTarea();

		if (receive_network_message(mens))
		{

			switch (mens->message_type)
			{
				case RECEIVE_STATUS:
					MensajeCGC.flota   = mens->msg[0];
					MensajeCGC.IndiH   = mens->msg[1];
					MensajeCGC.IndiL   = mens->msg[2];

					TxCGC (MensajeCGC,tabla[mens->msg[4]]);
				break;


				case RADIO_SETTINGS:

					if (mens->msg[0]!=0xa0) EncolaTarea (0,0,0,T_INI);

				break;

				case INCOMING_VOICE_MODEM:

					EncolaTarea (mens->msg[0],mens->msg[1],mens->msg[2],(long int)T_DESCON);

				break;

				case CLEARED_A:
				case CLEARED_N:
					empty();

					MensajeCGC.flota = ptrtareas.flota;
					MensajeCGC.IndiH = ptrtareas.IndiH;
					MensajeCGC.IndiL = ptrtareas.IndiL;

					TxCGC (MensajeCGC,mNOSQL);
					TxCGC (MensajeCGC,mCLEARED);
				break;

				default:
				break;
			}
		}

		r=receive_network_status();
		switch (r)
		{
			case SEND_FAIL:
					MensajeCGC.flota = ptrtareas.flota;
					MensajeCGC.IndiH = ptrtareas.IndiH;
					MensajeCGC.IndiL = ptrtareas.IndiL;

					TxCGC (MensajeCGC,mERR);
					ptrtareas.flota = movil.flota;
					ptrtareas.IndiH = movil.IndiH;
					ptrtareas.IndiL = movil.IndiL;

				conectado=0;
				empty();
			break;

			case SEND_OK:

				MensajeCGC.flota  = ptrtareas.flota;
				MensajeCGC.IndiH  = ptrtareas.IndiH;
				MensajeCGC.IndiL 	= ptrtareas.IndiL;

				TxCGC (MensajeCGC,mNOSQL);
				TxCGC (MensajeCGC,mCLEARED);

				empty();
			break;

			case SEND_IDLE:
				conectado=0;
				empty();
			break;

			case SEND_IN_PROGRESS:
				if (conectado==0)
				{
					MensajeCGC.flota  = ptrtareas.flota;
					MensajeCGC.IndiH  = ptrtareas.IndiH;
					MensajeCGC.IndiL 	= ptrtareas.IndiL;

					TxCGC (MensajeCGC,mSQL);
					TxCGC (MensajeCGC,mSEND_PROG);

					empty();
				}
				conectado=1;
				empty();
			break;


			case SEND_PENDING:
			break;

			default:
			break;
		}


	}
}


void TxCGC (struct MensToCGC MensajeCGC, unsigned char mensaje)
{

	int n;
	int rc;

	unsigned char array[7];

	array[0] = 0xec;
	array[1] = 0xa1;
	array[2] = MensajeCGC.flota;
	array[3] = MensajeCGC.IndiH;
	array[4] = MensajeCGC.IndiL;
	array[5] = mensaje;
	array[6] = 0;

	for (n=0;n<7;)
	{
		rc=comtrans (0,array[n]);
		if (!rc) n++;
	}

}

void EjecutaTarea (void)
{
	busy();
	switch (ptrtareas.mensaje)
	{

		case T_PTT_ON:
			if (conectado) TareaPttOn();
			empty();
		break;

		case T_PTT_OFF:

			if (conectado)
			{
				MensajeCGC.flota  = ptrtareas.flota;
				MensajeCGC.IndiH  = ptrtareas.IndiH;
				MensajeCGC.IndiL 	= ptrtareas.IndiL;

				TxCGC (MensajeCGC,mSQL);

				empty();
				TareaPttOff();
			}
			empty();
		break;

		case T_VEU:
			SetupVOICE ();
		break;

		case T_FI_VEU:
		case T_DESCON:

			disconnect();
			MensajeCGC.flota  = ptrtareas.flota;
			MensajeCGC.IndiH  = ptrtareas.IndiH;
			MensajeCGC.IndiL 	= ptrtareas.IndiL;

			TxCGC (MensajeCGC,mNOSQL);
		break;

		case T_INI:
			TareaSetup ();
		break;

		case T_TRUC_GRUP:
			TareaTrucSoci();
		break;

		default:
			empty();
			tarea=0;
		break;
	}
}


/*---------------------------- GESTION TAREAS -------------------------------*/

void TareaTrucSoci (void)
{
	GENERIC_APPL_MESSAGE *mensaje;

	mensaje->len=7;
	mensaje->message_type = SEND_STATUS;
	mensaje->msg[0]=ptrtareas.flota;	/* Called party PFIX1 */
	mensaje->msg[1]=ptrtareas.IndiH; /* Called party IDENT1*/
	mensaje->msg[2]=ptrtareas.IndiL; /* Called party IDENT1*/
	mensaje->msg[3]=1;
	mensaje->msg[4]=0;
	mensaje->msg[5]=CONFIRMACIO_AVIS_SOCI;

	if (send_network_message( mensaje )) tarea=0;

}

void TareaSetup(void)
{

	BOOLEAN status;


		status=RadioManagement (0xa0); /* Prepara el equipo para rececpci¢n de estados.*/

		if (status==TRUE)
				tarea=0;
}


void TareaPttOn (void)
{

	Port0|=PTT;
	portwrite (0,Port0);
	tarea=0;

}

void TareaPttOff (void)
{
	char a;

	Port0&=~PTT;
	portwrite (0,Port0);
	tarea=0;
}


/* ------------------------ GESTION MENSAJES TRUNKING -----------------------*/


BOOLEAN RadioManagement (char control)
{
	GENERIC_APPL_MESSAGE *mensaje;

	mensaje->len=4;
	mensaje->message_type = RADIO_MANAGEMENT;
	mensaje->msg[0]=control;
	mensaje->msg[1]=0;
	mensaje->msg[2]=0;

	if (send_network_message( mensaje )==FALSE)
		return (FALSE);



	return (TRUE);

}

void SetupVOICE (void)
{

	GENERIC_APPL_MESSAGE *mensaje;

	MensajeCGC.flota  = ptrtareas.flota;
	MensajeCGC.IndiH  = ptrtareas.IndiH;
	MensajeCGC.IndiL 	= ptrtareas.IndiL;

	mensaje->len=7;
	mensaje->message_type = SETUP_VOICE_MODEM;
	mensaje->msg[0]=ptrtareas.flota;
	mensaje->msg[1]=ptrtareas.IndiH;
	mensaje->msg[2]=ptrtareas.IndiL;
	mensaje->msg[3]=1;
	mensaje->msg[4]=0;
	mensaje->msg[5]=0x8;

	if (!send_network_message( mensaje ))
	{
			TxCGC (MensajeCGC,mERR);
			ptrtareas.flota = movil.flota;
			ptrtareas.IndiH = movil.IndiH;
			ptrtareas.IndiL = movil.IndiL;
	}
	tarea=0;

}


void disconnect(void)
{
	GENERIC_APPL_MESSAGE *mensaje;

	mensaje->message_type = DISCONNECT;

	if (send_network_message( mensaje ))
		tarea=0;

}


void EncolaTarea (unsigned char flota, unsigned char IndiH, unsigned char IndiL, unsigned int mensaje)
{
	movil.flota = ptrtareas.flota;
	movil.IndiH = ptrtareas.IndiH;
	movil.IndiL = ptrtareas.IndiL;

	ptrtareas.flota = flota;
	ptrtareas.IndiH = IndiH;
	ptrtareas.IndiL = IndiL;
	ptrtareas.mensaje = mensaje;
	ptrtareas.status = LIBRE;
	tarea=1;

}

/* ------------------------- HAND  -----------------------------------*/

void empty (void)
{

	Port0|=1;
	portwrite (0,Port0);
}

void busy (void)
{
	Port0&=0xfe;
	portwrite (0,Port0);
}


/* --------------------------- WATCH DOG -----------------------------------*/

void WatchDog (void)
{
	Port0|=WATCH_DOG;
	portwrite (0,Port0);

	Port0&=~WATCH_DOG;
	portwrite (0,Port0);
}
