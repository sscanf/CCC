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

char fichero [] = {"ASI00112345671212341234Oscar Casamitjana Vazquez                         Pepe Puertas   B-1175-FM SEAT MALAGA  0001SE ENCUENTRAN EN EL MALETERO EN LA      Antonio Alvarez               Plaza Maragall No.25 al lado de la panader¡a donde est  la parada de autobus                                                                              A Madrid                                                                  01212345612345671212341234Oscar Casamitjana Vazquez                         Pepe Puertas   B-1175-FM SEAT MALAGA  0001SE ENCUENTRAN EN EL MALETERO EN LA      Antonio Alvarez               Plaza Maragall No.25 al lado de la panader¡a donde est  la parada de autobus                                                                              A Madrid                                                                  012123456"};
char *PuntFich=fichero;
char *ptr;


void (interrupt far *ViejIntCOM1)();
void TranEntra(unsigned int nbytes,char *BufTx);
void sacabyte(unsigned char byte);
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

		  printf ("         1. - Tx ASI         \n");
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

				printf ("---- Tx ASI ----\n");

				PuntFich=fichero;
				TranEntra (422,PuntFich);
			  break;

			}

		}

	}

}



/*--------------------- TRANSMISION ----------------------------------*/

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
