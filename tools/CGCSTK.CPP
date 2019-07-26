#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <process.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <graphics.h>
#include <process.h>

#define NumBase 2872;
#define LOBYTE( p ) 		p & 0xFF
#define HIBYTE( p ) 		(p >> 8) & 0xFF
#define MKWORD( p1, p2 )	((p1 << 8) | p2)

const DES      = 1;
const OCU      = 2;
const PAR      = 3;
const FIN      = 4;
const ERR      = 5;
const EME      = 6;
const VEU      = 7;
const TAN      = 8;
const MEC      = 9;
const LLI      = 0x10;
const SQL_ON   = 0x11;
const SQL_OFF  = 0x22;

const vPTT_ON  = 0x20;
const vPTT_OFF = 0x21;
const vASI     = 0x15;
const vVEU     = 0x0a;
const vFI_VEU  = 0x0b;
const vPROG_IND= 0x0c;
const vMOD     = 0x16;
const vANU     = 0x09;
const vTEST    = 0x10;

const bard     = 0x080; /* Registro de acceso al registro divisor 8250   */
const tdl      = 0x001;
const lpe      = 0x002;
const out2     = 0x008;
const cop1     = 0x021;
const cop2     = 0x020;

const irq4     = 0x010;
const irq3     = 0x00;/* 0xef*/

// ---------------------------- CONFIGURACION PARA COM1 --------------------

const com1     = 0x3f8; // entrada datos com1
const rccm1    = 0x3fd;
const rcm1     = 0x3fc;
const rrb1     = 0x3f8; // Registro de recepci¢n 8250
const rat1     = 0x3f8; // Registro de almac‚n de transmisi¢n 8250
const rcl1     = 0x3fb; // Registro de control de l¡nea 8250
const LineStat1= 0x3fe; // Line status RS232
const rci1     = 0x3f9; // Registro de activaci¢n de interrupciones 8250
const relin1   = 0x3fd; // Registro de estado de l¡nea 8250
const rdl1     = 0x3f8; // Registro divisor 8250 LSB
const rdm1     = 0x3f9; // Registro divisor 8250 MSB



// ---------------------------- CONFIGURACION PARA COM2 ---------------------

const com2     = 0x2f8; // entrada datos com1
const rccm2    = 0x2fd;
const rcm2     = 0x2fc;
const rrb2     = 0x2f8; // Registro de recepci¢n 8250
const rat2     = 0x2f8; // Registro de almac‚n de transmisi¢n 8250
const rcl2     = 0x2fb; // Registro de control de l¡nea 8250
const LineStat2= 0x2fe; // Line status RS232
const rci2     = 0x2f9; // Registro de activaci¢n de interrupciones 8250
const relin2   = 0x2fd; // Registro de estado de l¡nea 8250
const rdl2     = 0x2f8; // Registro divisor 8250 LSB
const rdm2     = 0x2f9; // Registro divisor 8250 MSB


/* -------------- CONFIGURACION PARA PUERTO 1 DE PLACAS MOXA --------

const com2     = 0x280; // entrada datos com2 2F8
const rccm2    = 0x285; // 2FD
const rcm2     = 0x284; // 2FC
const rrb2     = 0x280; // Registro de recepci¢n 8250
const rat2     = 0x280; // Registro de almac‚n de transmisi¢n 8250
const rcl2     = 0x283; // 2FB Registro de control de l¡nea 8250
const LineStat2= 0x286; // 2FE Line status RS232
const rci2     = 0x281; // 2F9 Registro de activaci¢n de interrupciones 8250
const relin2   = 0x285; // 2FD Registro de estado de l¡nea 8250
const rdl2     = 0x280; // 2F8 Registro divisor 8250 LSB
const rdm2     = 0x281; // 281 Registro divisor 8250 MSB

*/

void interrupt (*ViejIntCOM1)(...);   //  vieja interrupci¢n com1:
void interrupt (*ViejIntCOM2)(...);   //  vieja interrupci¢n com2:
void interrupt (*ViejIntTIMER)(...);  //  vieja interrupci¢n Timer:
void interrupt IntCom1 (...);
void interrupt IntCom2 (...);
void interrupt IntTimer (...);

void InitComs(void);
void restaura ();
void TxMsgSt (char *msg,int leng);
void sacabyte(unsigned char byte);
void DoKbhit(void);
void WaitCTS(void);
void PrintIndi();
void indicativo (char *ptr);

char TxMsg [] = {0xec,0xa1,0x20,0,1,vPTT_OFF,0,
					  0xec,0xa1,0x20,0,1,vPTT_ON, 0,
//					  0xac,0xa1,0x20,0,1,0,      0,
					  0xec,0xa1,0x20,0,1,vVEU,	   0,
					  0xec,0xa1,0x20,0,1,vFI_VEU, 0,
					  0xec,0xa1,0x20,0,1,vANU,    0,
					  0xec,0xa1,0x20,0,1,vASI,    3,
					  0xec,0xa1,0x20,0,1,vTEST,   0,
					  0xec,0xa1,0x20,0,1,vMOD,		1};

char BuffSerie[11];

char cabe    []={0xec,0xa1};
char fichero []={"0000001\n11\n1111\n1234\nOscar Casamitjana Vazquez\n123456789012345\nB-1754-FM\nSeat panda\n1234\nEn el maletero\nPepito de los palotes\nC/.Granvia\nMalgrat de mar\n123\n000001\n"};
char fichero2[]={"X860095\n19\nPART\nPART\n PRUEBA CGC NOM PRUEBA*CGC\nASDFASDF\n\n\nEXT\n   \nDSF\nPRUEBA SITUACION CGC\nBARCELONA\nCTA\n\n"};
char mod     []={"0000MOD\n22\nMODI\nMODI\nMODI\n\n"};

char CTS=0;
int  CntTimer;

unsigned int inbyte(unsigned int port);
int MsgRx=0;
char FinRx=0;

union	REGS ent,sal;

void main()
{
	clrscr();
	InitComs();
	int n;
	for (;;)
	{
		while (FinRx==0)
		{
			gotoxy (25,25);
			printf ("CTS = ");

			gotoxy (25,1);
			puts("1.- Tx PTT ON");
			gotoxy (25,2);
			puts("2.- Tx PTT OFF");
			gotoxy (25,3);
			puts("3.- Tx VEU");
			gotoxy (25,4);
			puts("4.- Tx Fi VEU");
			gotoxy (25,5);
			puts("5.- Tx ANU");
			gotoxy (25,6);
			puts("6.- Tx Programaci¢n Indicativo");
			gotoxy (25,7);
			puts("7.- Tx MOD");
			gotoxy (25,8);
			puts("8.- Tx ASI");
			gotoxy (25,9);
			puts("9.- Test");
			gotoxy (25,10);
			puts("0.- Salir");

			while (!kbhit() && FinRx==0);


			if (kbhit()) DoKbhit();
		}
		PrintIndi();

		if (BuffSerie[3]<SQL_ON)
		{
			for (n=0;n<2;n++)
			{
				sound (800);
				delay (20);
				nosound();
				delay (70);
			}
		}

		CntTimer=10;
		gotoxy (1,25);
		switch (BuffSerie[3])
		{
			case DES:
				printf ("Recibido DES *");
				FinRx=0;
			break;
			case MEC:
				printf ("Recibido MEC *");
				FinRx=0;
				gotoxy (1,14);
				printf ("MECANICO %s",BuffSerie+7);
			break;
			case OCU:
				printf ("Recibido OCU *");
				FinRx=0;
			break;
			case PAR:
				printf ("Recibido PAR *");
				FinRx=0;
			break;
			case FIN:
				printf ("Recibido FIN *");
				FinRx=0;
			break;
			case EME:
				printf ("Recibido EME * ");
				FinRx=0;
			break;
			case VEU:
				printf ("Recibido VEU * ");
				FinRx=0;
			break;
			case TAN:
				printf ("Recibido TAN * ");
				FinRx=0;
			break;
			case LLI:
				printf ("Recibido LLI * ");
				FinRx=0;
			break;
			case SQL_ON:
				gotoxy (50,25);
				printf ("Recibido SQL ON ");
				FinRx=0;
			break;
			case SQL_OFF:
				gotoxy (50,25);
				printf ("Recibido SQL OFF ");
				FinRx=0;
			break;
			case ERR:
				printf ("Recibido ERR * ");
				FinRx=0;
			break;
			default:
				printf ("Recibido n.d.");
				FinRx=0;
			break;
		}
	}
}

void DoKbhit(void)
{
	char *ptr=TxMsg;
	int tecla;
	char string[4];
	char indi;
//	int  x,y;
	int CntLn;
	int lng,n;

	tecla=getch();

	switch (tecla)
	{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		{
			tecla-=0x31;
			tecla*=7;
			ptr+=tecla;

			if (tecla>7)
			{
				indicativo (ptr);
			}

			TxMsgSt(ptr,7);
		}
		break;

		case '7':
		{
			ptr+=49;

			indicativo(ptr);

			TxMsgSt (ptr,7);		// Indica a la ST que va a transmitir un ASI.

			ptr = mod;

			lng=strlen (mod);
			lng/=48;
			lng++;

			CntLn=0;
			FinRx=0;
			for (n=0;n<lng&&FinRx==0;n++)
			{
				WaitCTS ();		   // Espera que la ST est‚ lista.
				if (FinRx==0)
				{
					TxMsgSt (cabe,2);
					TxMsgSt (ptr,48);
					ptr+=48;
				}
			}
			FinRx=0;
		}
		break;

		case '8':
		{
			ptr+=35;
			indicativo(ptr);

			TxMsgSt (ptr,7);		// Indica a la ST que va a transmitir un ASI.

			ptr = fichero2;

			lng=strlen (fichero2);
			lng/=48;
			lng++;

			CntLn=0;
			FinRx=0;
			for (n=0;n<lng&&FinRx==0;n++)
			{
/*
// ESTA PARTE SOLO ES PARA QUE SAQUE LOS BLOQUES POR PANTALLA

					clrscr();
					for (int i=0;i<48;i++)
					{

						printf ("%02x ",ptr[i]);
						x=wherex();
						y=wherey();
						gotoxy (x+20,y);

						if (ptr[i]=='\n'||ptr[i]=='\a'||ptr[i]=='\0')
							printf (".");
						else
							printf ("%c",ptr[i]);

						CntLn++;

						gotoxy (x,y);
						if (CntLn==6)
						{
							 printf ("\n");
							 CntLn=0;
						}

					}
*/
				WaitCTS ();		   // Espera que la ST est‚ lista.
				if (FinRx==0)
				{
					TxMsgSt (cabe,2);
					TxMsgSt (ptr,48);
					ptr+=48;
				}
			}
			FinRx=0;
		}
		break;

		case '9':
		{
			indicativo(ptr);
			ptr+=42;
			TxMsgSt (ptr,7);		// Indica a la ST que va a transmitir un ASI.
		}
		break;

		case '0':
		{
			restaura (); // Restatura vectores interrupci¢n
			exit (0);
		}
		break;

		case ' ':
		{
			execl ("c:\\dos\\MODE.COM","80",0);
			perror ("EXEC:");
			getch();
			clrscr();
			PrintIndi();
		}
		break;

	}
}

/*
	ch=inp(LineStat1);

  if (((lineStatAnt^ch)&0x02)==0x02)
  { //  canvi d'estat DSR
	lineStatAnt=ch;
	if (comHost==OK) comHost=NOTOK;
	else comHost=OK;
  }
*/

void PrintIndi()
{
	char aflota[3];
	char aindi[3];

	unsigned int flota=(char)BuffSerie[0];
	unsigned int indiH =(char)BuffSerie[1];
	unsigned int indiL =(char)BuffSerie[2];

//	itoa (flota,aflota,3);
//	itoa (indi,aindi,3);

	indiL=indiL & 0x00ff;
	indiH=indiH<<8;
	indiH=indiH | indiL;
	gotoxy (1,12);
	printf ("FLOTA %u    \n",flota);
	printf ("INDIC.%u    ",indiH);
}

void TxMsgSt (char *msg,int leng)
{
	int n;

	outp (rci1,0x0);         /* desactiva interrupciones de la UART */

	for (n=0;n<leng;n++)
		sacabyte(msg[n]);

  outp (rci1,0x09);      /*ACTIVA INTERRUPCIONES UART */

}

void WaitCTS(void)
{
	int n;
//	delay (100);
	for (n=0;n<6000;n++);
	while (CTS);
}

void InitComs(void)  // Inicializa los COMS
{
	unsigned char byte;

	ent.x.ax = 0xe3;        // 9600,n,8,2
	ent.x.dx = 0;           // com1
	int86(0x14,&ent,&sal);  // total (OPEN "COM2:9600,N,8,1")

	outp (rci1,0x00);		 /* Desactiva interrupciones */
	outp (rcm1,0x03);      /* Desactiva RTS */

	ViejIntTIMER=getvect (0x1c);
	setvect (0x1c,IntTimer);

	ViejIntCOM2=getvect (12);   /* Obtiene vector int. antigua */
	setvect(12,IntCom2);   	    /* Redefine vector int.Nueva */

	outp (rcm2, tdl | lpe | out2);

	byte = inp (cop1);
	byte &= irq4;
	outp (cop1, 0);

	outp (rci1,0x09);		 // Activa interrupciones: recepci¢
}

unsigned int inbyte(unsigned int port)
{
	unsigned char tmp=0;
	unsigned long int timeout=0;
	unsigned int bt,rcm;

	switch (port)
	{
	 case com1: rcm=rccm1;break;
	 case com2: rcm=rccm2;break;
	}

	while (tmp==0 && timeout<10000)
	{
		tmp = inportb (rcm);
		tmp &= 0x01;
		timeout++;
	}

	bt=inp (port);
	return(bt);
}

void sacabyte(unsigned char byte)
{
	unsigned char UartLibre=0;

	while (UartLibre!=0x60)
		UartLibre=inportb (relin1) & 0x60;
	outp (com1,byte);
}


void restaura ()
{
	setvect (12,ViejIntCOM2);
	setvect (0x1c,ViejIntTIMER);
}



void indicativo (char *ptr)
{
	char string[80];
	int indi;

	gotoxy (1,20);
	printf ("Entre indicativo ");
	gets (string);
	indi=atoi (string);
	indi=(indi-20)+NumBase;

	ptr[3] =HIBYTE (indi);
	ptr[4] =LOBYTE (indi);

}
// -------------------- INTERRUPCIONES ---------------------------------------

void interrupt IntCom2 (...)
{
  unsigned int ch=0,numBloques=0,length=0,trobat=0,i;
  static int ptrRx=-2;
  static int lineStatAnt=0;
  static int FinBuffer=4;

  char far *panta;

  panta=(char far*) MK_FP (0xb800,3900);

  outp (rci1,0x0);         /* desactiva interrupciones de la UART */

  ch=inp(LineStat1);

  if (((ch)&0x01)==0x01) // Ha habido un cambio en el CTS
  {
	 CTS=inp (LineStat1);
	 CTS&= 0x10;
	 CTS=CTS >> 4;
	 *panta=CTS+0x30;
  }

  ch=inbyte(com1);

  switch(ptrRx)
  {
	 case -2: /* FRAME es 0xec 0xa1*/
		if (ch==0xec) ptrRx++;
			break;
	 case -1:
		if(ch==0xa1) ptrRx++;
		else ptrRx=-2;
		break;

	 default:
		BuffSerie[ptrRx]=ch;
//		MsgRx = ch;

		if (ptrRx==4)
			FinBuffer+=ch;

		if (ptrRx==FinBuffer)
		{
			ptrRx    = -2;
			FinRx    =  1;
			FinBuffer=  4;
		}
		else
			ptrRx++;
		break;
  }// del switch



  outp (rcm1, tdl | lpe | out2);
  outp (cop2,0x20);  /* Indica Fin de interrupt */
  outp (rci1,9);      /*ACTIVA INTERRUPCIONES UART */
}

void interrupt IntCom1 (...)
{
  outp (rcm1, tdl | lpe | out2);
  outp (cop2,0x20);  /* Indica Fin de interrupt */
  outp (rci1,9);      /*ACTIVA INTERRUPCIONES UART */
}

void interrupt IntTimer (...)
{

  char far *panta;

  CntTimer--;

  if (CntTimer==0)
  {
	  panta=(char far*) MK_FP (0xb800,3866);
	  *panta=' ';
  }
}


