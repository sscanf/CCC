#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <mem.h>
#include <string.h>

const bard	  = 0x080;
const tdl 	  = 0x001;


const lpe	  = 0x002;
const out2	  = 0x008;
const cop1	  = 0x021;
const cop2    = 0x020;
const irq4    = 0x010;

const	com1    = 0x2f8; /* entrada datos com1 */
const	rcm     = 0x2fd;
const	rcm1    = 0x2fc;
const	rrb     = 0x2f8; /* Registro de recepci¢n 8250                    */
const	rat     = 0x2f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl     = 0x2fb; /* Registro de control de l¡nea 8250             */
const	LineStat= 0x2fe; /* Line status RS232				  */
const	rci     = 0x2f9; /* Registro de activaci¢n de interrupciones 8250 */
const	rel     = 0x2fd; /* Registro de estado de l¡nea 8250              */
const	rdl     = 0x2f8; /* Registro divisor 8250 LSB                     */
const	rdm     = 0x2f9; /* Registro divisor 8250 MSB                     */

union REGS ent,sal;

char file1[]={"0000001\n11\n1111\n1234\nOscar Casamitjana Vazquez\n123456789012345\nB-1754-FM\nSeat panda\n1234\nEn el maletero\nPepito de los palotes\nC/.Granvia\nMalgrat de mar\n123\n000001\n\0"};
char file2[]={"0000002\n22\n2222\n1234\nJuan Perez\n---\nB-1111-AA\nSeat 132\n1234\nEn el maletero\nPepito de los palotes\nC/.Granvia\nMalgrat de mar\n123\n000002\n\0"};
char file3[]={"0000003\n33\n3333\n1234\nFernando Alvarez\n123456789012345\nB-2222-BB\nOPEL CORSA\n1234\nEn el maletero\nPepito de los palotes\nC/.Granvia\nMalgrat de mar\n123\n999999\n\0"};

char fileMOD[]={"MODIFIC\n++\nMODI\nMODI\nMODI\nMOD\nMODIFI\n\0"};

char *PuntFich=file1;
char *ptr;


char st0[]={0xec,0xa1,0x02,0x1};

//            sincro.     SYNT    0   1   2    3  4  5
char ahyc   []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x54,0x00,0x30};
char ack    []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x20,0x00,0x20};
char rqcASI []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x5c,0x00,0x30};
char rqcMOD []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x5c,0x00,0x32};
char head   []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x70,0x20,0x01};


char rqqANU   []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x58,0x00,0x29};
char rqqVEU   []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x58,0x00,0x2A};
char rqqFIVEU []={0xaa,0xaa,0x3b,0x28,0x81,0x00,0x08,0x58,0x00,0x2B};

char slot1[48];


void (interrupt far *ViejIntCOM1)();
void far interrupt cojebuff();
void TranEntra(unsigned int nbytes,char *BufTx);
void sacabyte(unsigned char byte);
void TranSlot(unsigned int nbytes,char *BufTx);
void TxHEAD (char *PuntFich,int FileLen);
char RxData();

main ()
{

	char byte;
	char tecla;
	int  FileLen;
	char car;


/* ------------------------ INICIALIZACION port ------------------*/

	ent.x.ax = 0xe3;         /* 0xe3 = 9600,n,8,1*/
											/* 0xc3 = 4800,n,8,1*/
	ent.x.dx = 1;            /* com2 */
	int86(0x14,&ent,&sal);   /* total (OPEN "COM2:9600,N,8,1") */

	ent.x.ax = 0xc3;         /* 0xe3 = 9600,n,8,1*/
									 /* 0xc3 = 4800,n,8,1*/
	ent.x.dx = 0;            /* com1 */
	int86(0x14,&ent,&sal);   /* total (OPEN "COM1:9600,N,8,1") */

	clrscr();

	outp (rci,0x00);		   /* Desactiva interrupciones */

	outp (rcm1, tdl | out2);  /*Pone la emisora en recepci¢n */

	byte = inp (cop1);
	byte &= irq4;
	outp (cop1, byte);

	for (;;)
	{

		  printf ("         1. - Test           \n");
		  printf ("         2. - Mensaje AHYC   \n");
		  printf ("         3. - Mensaje RQQ    \n");
		  printf ("         4. - Mensaje ACK    \n");
		  printf ("         5. - Tx ASI         \n");
		  printf ("         6. - Tx MOD         \n");
		  printf ("         7. - Tx V25-SERIE   \n");
		  printf ("         +. - Fichero mas    \n");
		  printf ("         -. - Fichero menos  \n");
		  printf ("   Espacio. - Menu         \n\n");

		tecla=0;
		while (tecla!=32)

		{
		  while (!kbhit())
		  {
			 car=RxData();
			 if (car>0)
			 {
				if (car>0x20)
				{
				  clrscr();
				  textattr (0x17);
				  window (30,1,61,25);
				  cprintf ("%c",car);

				  while (!kbhit())
				  {
					  car=RxData();
					  if (car>0) cprintf ("%c",car);
				  }
				  textattr (0x07);
				  window (1,1,80,25);
				  clrscr();
				}
			 }
		  }


		  tecla=getch();

			switch (tecla)
			{

			  case '1':

				while (!kbhit())
				{
					printf ("---- TEST ----\n");
					TranEntra (10,ahyc);
				}
			  break;

			  case '2':
				printf ("---- Tx AHYC ----\n");
				TranEntra (10,ahyc);
			  break;


			  case '3':
				printf ("---- Tx RQQ  ----\n");
				printf ("\n     1.- ANU ");
				printf ("\n     2.- VEU ");
				printf ("\n     3.- FI VEU \n");

				tecla = getch();
					if (tecla=='1') TranEntra (10,rqqANU);
					if (tecla=='1') TranEntra (10,rqqVEU);
					if (tecla=='1') TranEntra (10,rqqFIVEU);
			  break;


			  case '4':
				printf ("---- Tx ACK ----\n");
				TranEntra (10,ack);
			  break;

			  case '5':

				printf ("---- Tx ASI ----\n");
				printf ("\n     1.- Fichero 1 ");
				printf ("\n     2.- Fichero 2");
				printf ("\n     3.- Fichero 3\n");

				tecla= getch();

				if (tecla=='1')
				{
					PuntFich=file1;
					FileLen = strlen (file1);
				}
				if (tecla=='2')
				{
					PuntFich=file2;
					FileLen = strlen (file2);
				}
				if (tecla=='3')
				{
					PuntFich=file3;
					FileLen = strlen (file3);
				}

				TranEntra (10,rqcASI);
				printf ("Pulse cualquier tecla\n");
				getch();
				outportb (0x3f8,0x11);
				printf ("Pulse otra tecla\n");
				getch();

				TxHEAD (PuntFich,FileLen);
			  break;

			case '6':

				printf ("---- Tx MOD ----\n");

				TranEntra (10,rqcMOD);
				printf ("Pulse cualquier tecla\n");
				getch();
				outportb (0x3f8,0x11);
				printf ("Pulse otra tecla\n");
				getch();

				PuntFich=fileMOD;
				FileLen =strlen (fileMOD);
				TxHEAD (PuntFich,FileLen);

			break;

			  case '7':
				printf ("---- Tx V25-SERIE ----\n");

				printf ("Entre car cteres a transmitir (pulse [Esc] para finalizar\n");

				car=0;
				while (car!=27)
				{
					car= getche ();
					outportb (0x3f8,car);
				}
			  break;

			  case '+':
				outportb (0x3f8,3);
				delay (3);
				outportb (0x3f8,0x11);
			  break;

			  case '-':
				outportb (0x3f8,5);
				delay (3);
				outportb (0x3f8,0x11);
			  break;

			}

		}

	}

}



/*--------------------- TRANSMISION ----------------------------------*/

void TxHEAD (char *PuntFich,int FileLen)
{
	int block;
	char temp;

	for (block=0;block<8;block++)
	{
		memset (slot1,0,48);
		ptr = (char *) memccpy (slot1,PuntFich,0,48);
		PuntFich +=48;

		head[7]=0x70;
		head[6]=0x08;

		temp=(block+1&0x01);
		temp=temp<<7;
		head[7]=head[7]|temp;

		temp=(block+1&0x0e);
		temp=temp>>1;
		head[6]=head[6]|temp;

		TranEntra (10,head);
		TranSlot  (48,slot1);

		printf ("Transmitido bloque %d \n",block);
		if (ptr) break;
		getch();

	}
	printf ("----- Fichero transmitido -----\n");
}

void TranEntra(unsigned int nbytes,char *BufTx)
{
	int n;

	disable();
	outp (rci,0);              /*desactiva interrupciones 8259 */

	for (n=0;n<nbytes;n++)
		sacabyte(BufTx[n]);

	outp (rcm1, tdl | out2);

	outp (rci,1);              /*activa interrupciones 8259 */
	enable();
}
void TranSlot(unsigned int nbytes,char *BufTx)
{
	int i;
	int slt;
	int PosX;

	unsigned int n;

	disable();
	outp (rci,0);              /*desactiva interrupciones 8259 */

	i=0;
	slt=0;
	PosX=wherex();
	for (n=0;n<nbytes;n++)
	{
		sacabyte(BufTx[n]);
		i++;
		gotoxy (PosX,wherey());
		printf ("%02x ",BufTx[n]);

		gotoxy (PosX+30,wherey());
		PosX+=3;
		if (BufTx[n]>0x20 && BufTx[n]<0x7b) printf ("%c",BufTx[n]);
		else
		printf (".");

		if (i==6)
		{
			printf ("  slot %d \n",slt++);
			i=0;
			PosX=1;
		}

	}


	outp (rcm1, tdl | out2);

	outp (rci,1);              /*activa interrupciones 8259 */
	enable();
}


void sacabyte(unsigned char byte)
{
	unsigned char UartLibre=0;

	while (UartLibre!=0x60)
		UartLibre=inportb (rel) & 0x60;
	outp (com1,byte);
}

char RxData()
{
	int a;
	char b=-1;

	a=inportb (0x3fd);
	a = a& 0x01;
	if (a>0)
		b=(inportb (0x3f8));

	return (b);
}
