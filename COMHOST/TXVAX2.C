
#include "c:\dmqdos20\types.h"
#include "c:\dmqdos20\p_entry.h"
#include "c:\dmqdos20\p_proces.h"
#include "c:\dmqdos20\p_typecl.h"
#include "c:\dmqdos20\p_return.h"
#include "c:\dmqdos20\p_symbol.h"
#include "c:\dmqdos20\p_group.h"

#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_BUFFER_VAX 20
#define HEADER_VAX     3

const	bard    = 0x080; /* Registro de acceso al registro divisor 8250   */
const	tdl     = 0x001;
const	lpe     = 0x002;
const	out2    = 0x008;
const	cop1    = 0x021;
const	cop2    = 0x020;

const	irq4	  = 0x010;
const irq3    = 0x00;

const	com1     = 0x3f8; /* entrada datos com1 */
const	rccm1    = 0x3fd;
const	rcm1     = 0x3fc;
const	rrb1     = 0x3f8; /* Registro de recepci¢n 8250                    */
const	rat1     = 0x3f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl1     = 0x3fb; /* Registro de control de l¡nea 8250             */
const	LineStat1= 0x3fe; /* Line status RS232				  */
const	rci1     = 0x3f9; /* Registro de activaci¢n de interrupciones 8250 */
const	relin1   = 0x3fd; /* Registro de estado de l¡nea 8250              */
const	rdl1     = 0x3f8; /* Registro divisor 8250 LSB                     */
const	rdm1     = 0x3f9; /* Registro divisor 8250 MSB                     */

const	com2     = 0x2f8; /* entrada datos com2 */
const	rccm2    = 0x2fd;
const	rcm2     = 0x2fc;
const	rrb2     = 0x2f8; /* Registro de recepci¢n 8250                    */
const	rat2     = 0x2f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl2     = 0x2fb; /* Registro de control de l¡nea 8250             */
const	LineStat2= 0x2fe; /* Line status RS232				  */
const	rci2     = 0x2f9; /* Registro de activaci¢n de interrupciones 8250 */
const	relin2   = 0x2fd; /* Registro de estado de l¡nea 8250              */
const	rdl2     = 0x2f8; /* Registro divisor 8250 LSB                     */
const	rdm2     = 0x2f9; /* Registro divisor 8250 MSB                     */

char bufferRx[HEADER_VAX+MAX_BUFFER_VAX]="";

void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/

union	REGS ent,sal;

char fichero [] = {"ASI00112345671212341234Oscar Casamitjana Vazquez                         Pepe Puertas   B-1175-FM SEAT MALAGA  0001SE ENCUENTRAN EN EL MALETERO EN LA      Antonio Alvarez               Plaza Maragall No.25 al lado de la panader¡a donde est  la parada de autobus                                                                              A Madrid                                                                  012123456"};
char fichero2 [] = {"MOD001MMNNSSILLyyyyttttrrrjjjjjj"};
char fichero3 [] = {"ANU001123456712"};
char fichero4 [] = {"NITINI"};
char fichero5 [] = {"NITFIN"};

unsigned int FiRx=0;

char *PuntFich=fichero;
char *ptr;

// PROTOTIPUS

void initComs(void);
void Tx2CGC(unsigned int numbytes,char *missatge);
void Tx2CGC2(unsigned int numbytes,char *missatge);

void sacabyte(unsigned int port,unsigned int byte);
unsigned int inbyte(unsigned int port);
void interrupt cojebuff1(...);




void main(void)
{
 char tecla;

 clrscr();
 initComs();
 printf("DecMsgQ ... \n");

 for(;;)
 {
		  printf ("         1. - Tx ASI         \n");
		  printf ("         2. - Tx MOD         \n");
		  printf ("         3. - Tx ANU         \n");
		  printf ("         4. - Tx NIT INI         \n");
		  printf ("         5. - Tx NIT FIN         \n");
		  printf ("         6. - Rx            \n");
		  printf ("         7. - Salir         \n");
		  printf ("         8. - Alta M•bil    \n");
		  printf ("         9. - Menu         \n");


  while(!kbhit())
  {
		  tecla=getch();

		  switch (tecla)
		  {

			  case '1':

				printf ("---- Tx ASI ----\n");

				PuntFich=fichero;
				Tx2CGC(strlen(PuntFich),PuntFich);
			  break;
			  case '2':
				printf ("---- Tx MOD ----\n");

				PuntFich=fichero2;
				Tx2CGC(strlen(PuntFich),PuntFich);
				break;
			  case '3':
				printf ("---- Tx ANU ----\n");

				PuntFich=fichero3;
				Tx2CGC(strlen(PuntFich),PuntFich);
				break;
			  case '4':
				printf ("---- Tx NIT INI ----\n");

				PuntFich=fichero4;
				Tx2CGC(strlen(PuntFich),PuntFich);
				break;
			  case '5':
				printf ("---- Tx NIT FIN ----\n");

				PuntFich=fichero5;
				Tx2CGC(strlen(PuntFich),PuntFich);
				break;
			  case '6':
				printf("Rx ");
				printf(bufferRx);
				break;
			  case '7':
				exit(1);
			  case '8':
				printf("-----TX MEC -----");
				Tx2CGC2(5,"12356");
				break;
			  case '9':
				clrscr();
				printf ("         1. - Tx ASI         \n");
				printf ("         2. - Tx MOD         \n");
				printf ("         3. - Tx ANU         \n");
				printf ("         4. - Tx NIT INI         \n");
				printf ("         5. - Tx NIT FIN         \n");
				printf ("         6. - Rx            \n");
				printf ("         7. - Salir         \n");
				printf ("         8. - Alta M•bil   \n");
				printf ("         9. - Menu         \n");
				break;
			  default:;
			}
	}

 }
 printf("DecMsgQ acaba.");
}

// DEFINICIO FUNCIONS

void initComs(void)
{
	unsigned char byte;

	ent.x.ax = 0xe3;        /* 9600,n,8,1*/
	ent.x.dx = 1;           /* com2 */
	int86(0x14,&ent,&sal);  /* total (OPEN "COM2:9600,N,8,1") */


	outp (rci2,0x00);		 /* Desactiva interrupciones */
	outp (rcm2,0x03);      /* Desactiva RTS */


	ViejIntCOM1=getvect (11);   /* Obtiene vector int. antigua */
	setvect(11,cojebuff1); 		/* Redefine vector int.Nueva */

	outp (rcm2, tdl | lpe | out2);

	byte = inp (cop1);
	byte &= irq4;
	outp (cop1, byte);

	outp (rci2,0x01);		 // Activa interrupciones: recepci¢
}



// COMUNICACIO AMB CGC
//  ComHost <-> CGC pel COM2


// TRANSMISSIO

void Tx2CGC(unsigned int numbytes,char *missatge)
{

 unsigned int i=0;

 disable();
 outp (rci2,0);              /*desactiva interrupciones 8259 */

 outp (rcm2,inp(rcm2) | 0x02);

 for(i=0;i<numbytes;i++)
  sacabyte(com2,missatge[i]);

 outp (rci2,0x01);              /*activa interrupciones 8259 */
 enable();
}

void Tx2CGC2(unsigned int numbytes,char *missatge)
{

 unsigned int i=0;

 disable();
 outp (rci2,0);              /*desactiva interrupciones 8259 */

 outp (rcm2,inp(rcm2) | 0x02);

 sacabyte(com2,0xec);
 sacabyte(com2,0xa1);

 sacabyte(com2,0x01);
 sacabyte(com2,0x00);
 sacabyte(com2,0x01);
 sacabyte(com2,0x09);
 sacabyte(com2,0x05);

 for(i=0;i<numbytes;i++)
  sacabyte(com2,missatge[i]);

 outp (rci2,0x01);              /*activa interrupciones 8259 */
 enable();
}


void sacabyte(unsigned int port,unsigned int byte)
{
	unsigned char UartLibre=0;
	unsigned int relin;

	switch (port)
	{
	 case com1: relin=relin1;break;
	 case com2: relin=relin2;break;
	}

	do{
		UartLibre=inportb (relin) & 0x60;
	}while (UartLibre!=0x60);
	outp (port,byte);
}

// RECEPCIO

void interrupt cojebuff1(...)
{
  static int ptrRx=0;
  static char bufRxData[MAX_BUFFER_VAX+1]="";
  static char bufRxHeader1[HEADER_VAX+1]="";
  unsigned int ch=0;
  static unsigned int numbytes=0;



  outp (rci2,0x0);         /* desactiva interrupciones de la UART */

  ch=inbyte(com2);

  switch(ptrRx)
  {
		 // tipus de missatge
		 case 0:if (ch=='O' || ch=='D' || ch=='R' || ch=='P' || ch=='F'
						|| ch=='T' || ch=='E' || ch=='N')
					  bufRxHeader1[ptrRx++]=ch;
				  else  ptrRx=0;
				  break;
		 case 1:if (ch=='B' || ch=='E' || ch=='A' || ch=='I' || ch=='R')
					  bufRxHeader1[ptrRx++]=ch;
				  else ptrRx=0;
				  break;
		 case 2:
				  bufRxHeader1[ptrRx++]=ch;
				  bufRxHeader1[ptrRx]='\0';
				  if (strcmp(bufRxHeader1,"OBE")==0)
						  numbytes=6;
				  else
					if (strcmp(bufRxHeader1,"DES")==0 || strcmp(bufRxHeader1,"REP")==0
						 || strcmp(bufRxHeader1,"PAR")==0 || strcmp(bufRxHeader1,"FIN")==0
						 || strcmp(bufRxHeader1,"TAN")==0)
						  numbytes=3;
						else
						 if (strcmp(bufRxHeader1,"NIT")==0)
						  ptrRx=0;
						 else
						  if (strcmp(bufRxHeader1,"ERR")==0)
							 numbytes=15;
						  else
							ptrRx=0;
				  break;
		 default:
				  if (((ptrRx-HEADER_VAX)<numbytes) && ((ptrRx-HEADER_VAX)<(MAX_BUFFER_VAX)))
				  {
						bufRxData[ptrRx-HEADER_VAX]=ch;
						ptrRx++;
				  }

				  if ((ptrRx-HEADER_VAX)>=numbytes)
					  FiRx=1;
				  break;
  }/* del switch ptrRx*/


  if (FiRx==1) // FI RECEPCIO
  {
		 strcpy(bufferRx,bufRxHeader1);
		 bufRxData[numbytes]='\0';
		 strcat(bufferRx,bufRxData);

		 /* inicialitzacio per recepci¢*/
		 ptrRx=0;
		 strcpy(bufRxHeader1,"");
		 strcpy(bufRxData,"");
		 numbytes=0;
  }

  outp (rcm2, tdl | lpe | out2);
  outp (cop2,0x20);   /* Indica Fin de interrupt */
  outp (rci2,0x01);      /* ACTIVA INTERRUPCIONES UART */
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


