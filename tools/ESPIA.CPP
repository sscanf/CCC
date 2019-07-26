#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <process.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <graphics.h>
#include <process.h>
#include <c:\racc\HEAD-C.H>


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

void interrupt (*ViejIntCOM1)(...);   //  vieja interrupci¢n com1:
void interrupt (*ViejIntCOM2)(...);   //  vieja interrupci¢n com2:
void interrupt (*ViejIntTIMER)(...);  //  vieja interrupci¢n Timer:
void interrupt IntCom1 (...);
void interrupt IntCom2 (void);
void interrupt IntTimer (...);

void InitComs(void);
void restaura ();
void TxMsgSt (char *msg,int leng);
void DoKbhit(void);
void WaitCTS(void);
void PrintIndi();

char TxMsg [] = {0xec,0xa1,1,0,1,vPTT_OFF,0,
					  0xec,0xa1,1,0,1,vPTT_ON, 0,
//               0xac,0xa1,1,0,1,0,      0,
					  0xec,0xa1,0x20,0,1,vVEU,    0,
					  0xec,0xa1,1,0,1,vFI_VEU, 0,
					  0xec,0xa1,1,0,1,vANU,    0,
					  0xec,0xa1,1,0,1,vASI,    3,
					  0xec,0xa1,1,0,1,vTEST,   0,
					  0xec,0xa1,1,0,1,vMOD,    1};

char BuffSerie[11];

char cabe    []={0xec,0xa1};
char fichero []={"0000001\n11\n1111\n1234\nOscar Casamitjana Vazquez\n123456789012345\nB-1754-FM\nSeat panda\n1234\nEn el maletero\nPepito de los palotes\nC/.Granvia\nMalgrat de mar\n123\n000001\n"};
char fichero2[]={"X860095\n19\nPART\nPART\n PRUEBA CGC NOM PRUEBA*CGC\nASDFASDF\n\n\nEXT\n   \nDSF\nPRUEBA SITUACION CGC\nBARCELONA\nCTA\n\n"};
char mod     []={"0000MOD\n22\nMODI\nMODI\nMODI\n\n"};

char reset[]={"RESET"};

char COM1   = 0;
char COM2   = 1;


char CTS=0;
int  CntTimer;

int MsgRx=0;
char FinRx=0;

union REGS ent,sal;

FILE *f;
void main()
{
	clrscr();
	int n;
	int ch;
	unsigned char AntLine;
	unsigned char line;

	char pReset=0;


	sio_open (COM2);
	sio_open (COM1);
	sio_ioctl (COM1,B9600,P_NONE|BIT_8|STOP_1);
	sio_ioctl (COM2,B9600,P_NONE|BIT_8|STOP_1);

	if ((f= fopen ("espia.dat","a"))==NULL)
	{
		printf ("No puedo abrir el fichero espia.dat");
		exit (0);
	}



	while (!kbhit())
	{

		if ((ch=sio_getch(COM2))>=0)
		{
			fprintf (f,"%x%x|",2,ch);
			printf ("%x%x|",2,ch);
		}

		if ((ch=sio_getch(COM1))>=0)
		{
			if (ch==reset[pReset]) pReset++;
			else
				pReset=0;

			if (pReset==4)
			{
				 printf ("RESET");
				 fprintf (f,"%x|",4);
				 printf ("4|");
				 pReset=0;
			}
			fprintf (f,"%x%x|",1,ch);
			printf ("%x%x|",1,ch);
		}

		line = sio_lstatus(COM2);

		if (line!=AntLine)
		{
			fprintf (f,"%x%x|",3,line);
			printf ("%x%x|",3,line);
			AntLine=line;

		}
	}

	fclose (f);
	sio_close (COM1);
	sio_close (COM2);
}

