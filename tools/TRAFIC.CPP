#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <process.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

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
const irq3     = 0xf7;/* 0xef*/

const com1     = 0x3f8; /* entrada datos com1 */
const rccm1    = 0x3fd;
const rcm1     = 0x3fc;
const rrb1     = 0x3f8; /* Registro de recepci¢n 8250                    */
const rat1     = 0x3f8; /* Registro de almac‚n de transmisi¢n 8250       */
const rcl1     = 0x3fb; /* Registro de control de l¡nea 8250             */
const LineStat1= 0x3fe; /* Line status RS232				  */
const rci1     = 0x3f9; /* Registro de activaci¢n de interrupciones 8250 */
const relin1   = 0x3fd; /* Registro de estado de l¡nea 8250              */
const rdl1     = 0x3f8; /* Registro divisor 8250 LSB                     */
const rdm1     = 0x3f9; /* Registro divisor 8250 MSB                     */
const com2     = 0x2f8; /* entrada datos com2 */
const rccm2    = 0x2fd;
const rcm2     = 0x2fc;
const rrb2     = 0x2f8; /* Registro de recepci¢n 8250                    */
const rat2     = 0x2f8; /* Registro de almac‚n de transmisi¢n 8250       */
const rcl2     = 0x2fb; /* Registro de control de l¡nea 8250             */
const LineStat2= 0x2fe; /* Line status RS232				  */
const rci2     = 0x2f9; /* Registro de activaci¢n de interrupciones 8250 */
const relin2   = 0x2fd; /* Registro de estado de l¡nea 8250              */
const rdl2     = 0x2f8; /* Registro divisor 8250 LSB                     */
const rdm2     = 0x2f9; /* Registro divisor 8250 MSB                     */

void interrupt (*ViejIntCOM1)(...);   //  vieja interrupci¢n com1:
void interrupt (*ViejIntCOM2)(...);   //  vieja interrupci¢n com2:
void interrupt (*ViejIntTIMER)(...);  //  vieja interrupci¢n Timer:
void interrupt IntCom1 (...);
void interrupt IntCom2 (...);

void InitComs(void);
void restaura ();
void DoKbhit(void);
void PrintIndi();
void menu (void);
char BuffSerie[11];

char RxMesg[]={'*','*','*','*','*','*','*','*','*','*','*'};

unsigned int inbyte(unsigned int port);
int MsgRx=0;
char FinRx=0;

union	REGS ent,sal;

void main()
{
	clrscr();
	InitComs();
	int n,car;
	for (;;)
	{
		while (FinRx==0);
		{
			while (!kbhit() && FinRx==0);

			if (kbhit())
			{
				car=getch();
				if (car==' ')
				{
					 menu();
					 clrscr();
				}
				if (car=='q')
				{
					restaura();
					exit(0);
				}
			}
		}
		switch (BuffSerie[3])
		{
			case DES:
				if (RxMesg[0]=='*')
				{
					printf ("Recibido DES ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case MEC:
				if (RxMesg[1]=='*')
				{
					FinRx=0;
					printf ("MECANICO %s \n",BuffSerie+7);
				}
			break;
			case OCU:
				if (RxMesg[2]=='*')
				{
					printf ("Recibido OCU ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case PAR:
				if (RxMesg[3]=='*')
				{
					printf ("Recibido ATU ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case FIN:
				if (RxMesg[4]=='*')
				{
					printf ("Recibido FIN ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case EME:
				if (RxMesg[5]=='*')
				{
					printf ("Recibido EME  ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case VEU:
				if (RxMesg[6]=='*')
				{
					printf ("Recibido VEU ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case TAN:
				if (RxMesg[7]=='*')
				{
					printf ("Recibido TAN ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case LLI:
				if (RxMesg[8]=='*')
				{
					printf ("Recibido LLI ");
					PrintIndi();
					FinRx=0;
				}
			break;
			case SQL_ON:
				if (RxMesg[9]=='*')
				{
					printf ("Recibido SQL ON \n");
					FinRx=0;
				}
			break;
			case SQL_OFF:
				if (RxMesg[10]=='*')
				{
					printf ("Recibido SQL OFF \n");
					FinRx=0;
				}
			break;
			case ERR:
				printf ("Recibido ERR \n");
				FinRx=0;
			break;
			default:
				printf ("Mensaje no reconocido ... \n");
				FinRx=0;
			break;
		}
	}
}

void menu(void)
{
	char *tec;
	int n;
	int selec;
	int fin=0;


	while (fin==0)
	{
		clrscr();
		
		printf ("            MENU \n\n");
		printf ("           1.- DES  \n");
		printf ("           2.- MEC  \n");
		printf ("           3.- OCU  \n");
		printf ("           4.- PAR  \n");
		printf ("           5.- FIN  \n");
		printf ("           6.- EME  \n");
		printf ("           7.- VEU  \n");
		printf ("           8.- TAN  \n");
		printf ("           9.- LLI  \n");
		printf ("          10.- SQL ON  \n");
		printf ("          11.- SQL OFF \n");
		printf ("          12.- Salir de este menu \n");

		gotoxy (1,20);
		printf ("Entre opcion :");
		for (n=0;n<12;n++)
		{
			gotoxy (8,n+3);
			printf ("%c",RxMesg[n]);
		}
		tec=new char [4];
		gotoxy (16,20);

		gets(tec);
		selec=atoi (tec);

		if(selec==12)
			fin=1;
		else
		{
			selec--;
			if (RxMesg[selec]=='*')
				RxMesg[selec]=' ';
			else
				RxMesg[selec]='*';

			delete (tec);
		}
	}


}
void PrintIndi()
{
	unsigned int flota=(char)BuffSerie[0];
	unsigned int indiH =(char)BuffSerie[1];
	unsigned int indiL =(char)BuffSerie[2];

//	itoa (flota,aflota,3);
//	itoa (indi,aindi,3);

	indiL=indiL & 0x00ff;
	indiH=indiH<<8;
	indiH=indiH | indiL;
	printf (" FLOTA %u  INDIC. %u  \n",flota,indiH);
}

void InitComs(void)  // Inicializa los COMS
{
	unsigned char byte;

	ent.x.ax = 0xe3;        /* 9600,n,8,1*/
	ent.x.dx = 1;           /* com2 */
	int86(0x14,&ent,&sal);  /* total (OPEN "COM2:9600,N,8,1") */


	outp (rci2,0x00);		 /* Desactiva interrupciones */
	outp (rcm2,0x03);      /* Desactiva RTS */

	ViejIntCOM2=getvect (11);   /* Obtiene vector int. antigua */
	setvect(11,IntCom2);   	    /* Redefine vector int.Nueva */

	outp (rcm2, tdl | lpe | out2);

	byte = inp (cop1);
	byte &= irq3;
	outp (cop1, byte);

	outp (rci2,0x01);		 // Activa interrupciones: recepci¢
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

void restaura ()
{
	setvect (11,ViejIntCOM2);
}

// -------------------- INTERRUPCIONES ---------------------------------------

void interrupt IntCom2 (...)
{
  FILE *file;
  unsigned int ch=0,numBloques=0,length=0,trobat=0,i;
  static int ptrRx=-2;
  static int lineStatAnt=0;
  static int FinBuffer=4;

  outp (rci2,0x0);         /* desactiva interrupciones de la UART */
  ch=inbyte(com2);

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



  outp (rcm2, tdl | lpe | out2);
  outp (cop2,0x20);  /* Indica Fin de interrupt */
  outp (rci2,1);      /*ACTIVA INTERRUPCIONES UART */
}

void interrupt IntCom1 (...)
{
  outp (rcm1, tdl | lpe | out2);
  outp (cop1,0x20);  /* Indica Fin de interrupt */
  outp (rci1,1);      /*ACTIVA INTERRUPCIONES UART */
}


