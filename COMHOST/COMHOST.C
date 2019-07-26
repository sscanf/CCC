#ifdef __cplusplus
	 #define __CPPARGS ...
#else
	 #define __CPPARGS
#endif

#include "g:\projects\dmqdos20\types.h"
#include "g:\projects\dmqdos20\p_entry.h"
#include "g:\projects\dmqdos20\p_proces.h"
#include "g:\projects\dmqdos20\p_typecl.h"
#include "g:\projects\dmqdos20\p_return.h"
#include "g:\projects\dmqdos20\p_symbol.h"
#include "g:\projects\dmqdos20\p_group.h"

#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_BUFFER_VAX 422
#define HEADER_VAX     3
#define MAX_LONGMSG    10
#define MAX_NUMMSG     20

const unsigned int long TIMEOUT = 100000;

const bard    = 0x080; /* Registro de acceso al registro divisor 8250   */
const tdl     = 0x001;
const lpe     = 0x002;
const out2    = 0x008;
const cop1    = 0x021;
const cop2    = 0x020;

const irq4    = 0xef; // bit 4: 0 -> com1, bit 3: 0 -> com2

const com1     = 0x3f8; /* entrada datos com1 */
const rccm1    = 0x3fd;
const rcm1     = 0x3fc;
const rrb1     = 0x3f8; /* Registro de recepci¢n 8250                    */
const rat1     = 0x3f8; /* Registro de almac‚n de transmisi¢n 8250       */
const rcl1     = 0x3fb; /* Registro de control de l¡nea 8250             */
const LineStat1= 0x3fe; /* Line status RS232            */
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
const LineStat2= 0x2fe; /* Line status RS232            */
const rci2     = 0x2f9; /* Registro de activaci¢n de interrupciones 8250 */
const relin2   = 0x2fd; /* Registro de estado de l¡nea 8250              */
const rdl2     = 0x2f8; /* Registro divisor 8250 LSB                     */
const rdm2     = 0x2f9; /* Registro divisor 8250 MSB                     */

char bufferRx[HEADER_VAX+MAX_BUFFER_VAX+2]="";

void interrupt (*ViejIntCOM1)(__CPPARGS);   /*vieja interrupci¢n com1:*/

union REGS ent,sal;


unsigned int nBytes=0,ptrRx=0,numMsg=0,txMsg=0;


char *DMQ_Q_ASIGNA="2";
char *DMQ_Q_CAMB_EST="1";
Pams_Address target;
Pams_Address source;
long int sts_value;
long int time_out;
long int proc_num_in;
Pams_Address proc_num_out;
short int size;
short int oclass,otype;
short int iclass,itype;
short int msg_size;
char ibuffer[423];       // buffer que ser… transmŠs al CGC
char delivery,priority,rcv_priority;
long int put_timeout;
struct PSB psb;
char uma;
Pams_Address resp_que;
long int sel_addr,attach_mode,q_type,q_name_len;
char q_name[31];
short buffer_length=422;
char msgQueue[MAX_NUMMSG][MAX_LONGMSG];


// PROTOTIPUS

void initComs(void);
void Tx2CGC(unsigned int numbytes,char *missatge);
void sacabyte(unsigned int port,unsigned int byte);
unsigned int inbyte(unsigned int port);
void interrupt cojebuff1();
void TxMsg(void);
void RxMsg(void);
void Ascii2Hex(char *missatge);


void main(void)
{
 unsigned int final=0,i;
 unsigned int long timeout=0;

 initComs();
 printf("Inicialitzaci¢ Ok.\n");
 printf("DecMsgQ ... <S sortir>\n");

 for(i=0;i<MAX_NUMMSG;i++) memset(msgQueue[i],'\0',MAX_LONGMSG);
 numMsg=0;txMsg=0;

 strcpy(q_name,DMQ_Q_ASIGNA);
 q_name_len=strlen(q_name);
 q_type=PSYM_ATTACH_PQ;
 attach_mode=PSYM_ATTACH_BY_NUMBER;
 proc_num_out.all=0;


  printf("En fase de connexi¢ DMQ_ASSIGNA.\n");
  sts_value=-1;

  pams_exit();
  for(;sts_value!=PAMS__SUCCESS && timeout<TIMEOUT;timeout++)
			sts_value=pams_attach_q(&attach_mode,&proc_num_out,
			&q_type,q_name,&q_name_len,0,0,0,0,0);

  if (sts_value!=PAMS__SUCCESS)
  {
	 printf("Error en la fase de connexi¢ %d.\n",sts_value);
	 Tx2CGC(7,"ERRHOST");
  }
  else
	  printf("Rebent cua %d.%d ...\n",proc_num_out.au.group,proc_num_out.au.queue);

 do{
  while (!(kbhit()))
  {
	 RxMsg();
	 if (msgQueue[txMsg][0]!='\0') //  Rx de CGC -> Tx a VAX
	 {
/*   strcpy(q_name,"");
	  q_name_len=0;
	  attach_mode=PSYM_ATTACH_TEMPORARY;
	  q_type=PSYM_ATTACH_PQ;
	  proc_num_out.all=0;

	  sts_value=-1;

	  pams_exit();
	  for(;sts_value!=PAMS__SUCCESS && timeout<TIMEOUT;timeout++)
		 sts_value=pams_attach_q(&attach_mode,&proc_num_out,
				  &q_type,q_name,&q_name_len,0,0,0,0,0);

	  if (sts_value!=PAMS__SUCCESS)
	  {
		printf("Error en la fase de connexi¢ %d.\n",sts_value);
		Tx2CGC(7,"ERRHOST");
	  }
	  else */
		TxMsg();
	 }
  }
  if ((getche())=='S') final=1;
 }while (final==0);
 pams_exit();

 outp(rci1,0x00);
 setvect(12,ViejIntCOM1);     /* Redefine vector int.Nueva */
 printf("DecMsgQ acaba.");
}

// DEFINICIO fFUNCIONS

void initComs(void)
{
	unsigned char byte;

	ent.x.ax = 0xe3;        /* 9600,n,8,1*/
	ent.x.dx = 0;           /* com1 */
	int86(0x14,&ent,&sal);  /* total (OPEN "COM1:9600,N,8,1") */


	outp (rci1,0x00);     /* Desactiva interrupciones */
	outp (rcm1,0x03);      /* Desactiva RTS */


	ViejIntCOM1=getvect (12);   /* Obtiene vector int. antigua */
	setvect(12,cojebuff1);     /* Redefine vector int.Nueva */

	outp (rcm1, tdl | lpe | out2);

	byte = inp (cop1);
	byte &= irq4;
	outp (cop1, byte);

	outp (rci1,0x01);     // Activa interrupciones: recepci¢
}



// COMUNICACIO AMB CGC
//  ComHost <-> CGC pel COM1


// TRANSMISSIO

void Tx2CGC(unsigned int numbytes,char *missatge)
{

 unsigned int i=0;
 int PosX;
 int PosY;
 int n;
 int cnt;

 disable();
 outp (rci1,0);              /*desactiva interrupciones 8259 */

 outp (rcm1,(inp(rcm1) | 0x02));

 if (missatge[0]=='A' && missatge[1]=='N') numbytes=17;
 else
  if (missatge[0]=='M') numbytes=34;
  else
	if (missatge[0]=='N' && missatge[1]=='I') numbytes=7;

 for(i=0;i<numbytes;i++)
  sacabyte(com1,missatge[i]);

//  Ascii2Hex (missatge);

 outp (rci1,0x01);              /*activa interrupciones 8259 */
 enable();
}


void sacabyte(unsigned int port,unsigned int byte)
{
	unsigned char UartLibre=0;
	unsigned int relin;

	switch (port)
	{
	 case 0x3f8:
		relin=relin1;
	 break;

	 case 0x2f8:
		relin=relin2;
	 break;
	}

	do{
		UartLibre=inportb (relin) & 0x60;
	}while (UartLibre!=0x60);
	outp (port,byte);
}

// RECEPCIO

void interrupt cojebuff1(__CPPARG)
{
  unsigned int ch=0,FiRxInt=0,length=0,i=0;
  char far *pp;

  outp(rci1,0x00);         /* desactiva interrupciones de la UART */

  ch=inbyte(com1);


	switch(ptrRx)
	{
		 // tipus de missatge
		 case 0:if (ch=='O' || ch=='D' || ch=='R' || ch=='P' || ch=='F'  || ch=='T' || ch=='E' || ch=='N')
					  bufferRx[ptrRx++]=ch;
				  else  ptrRx=0;
				  break;
		 case 1:if (ch=='B' || ch=='E' || ch=='A' || ch=='I' || ch=='R')
					  bufferRx[ptrRx++]=ch;
				  else ptrRx=0;
				  break;
		 case 2:

				  bufferRx[ptrRx++]=ch;
				  if (strcmp(bufferRx,"OBE")==0)
						  nBytes=6;
				  else
					if ((strcmp(bufferRx,"DES")==0) || (strcmp(bufferRx,"REP")==0)
						 || (strcmp(bufferRx,"PAR")==0) || (strcmp(bufferRx,"FIN")==0)
						 || (strcmp(bufferRx,"TAN")==0))
						  nBytes=3;
					else
					 if (strcmp(bufferRx,"NIT")==0)
					 {
						  FiRxInt=1;
						  nBytes=0;
					 }
					 else
						  if (strcmp(bufferRx,"ERR")==0)
							 nBytes=6;
						  else
						  {
							ptrRx=0;
							nBytes=0;
							ptrRx=0;
							FiRxInt=0;
							memset(bufferRx,'\0',HEADER_VAX+MAX_BUFFER_VAX+2);
						  }
				  break;
		 default:
				  if (((ptrRx-HEADER_VAX)<nBytes) && ((ptrRx-HEADER_VAX)<(MAX_BUFFER_VAX)))
				  {
						bufferRx[ptrRx]=ch;
						ptrRx++;
				  }

				  if ((ptrRx-HEADER_VAX)>=nBytes)
				  {
					  FiRxInt=1;
					  pp=(char far *)MK_FP(0xb000,0);
					  *pp=bufferRx[0];
				  }
				  break;
	}/* del switch ptrRx*/
	if (FiRxInt==1) // FI RECEPCIO
	{
		 if (strlen(bufferRx)<9)
		 {
		  for(i=strlen(bufferRx);i<9;i++) bufferRx[i]=' ';
		 }
		 bufferRx[strlen(bufferRx)]='\0';

		 // posem a la cua de Tx
		 if (numMsg==txMsg && msgQueue[txMsg][0]!='\0') numMsg++;
		 strcpy(msgQueue[numMsg],bufferRx);

		 if (numMsg<MAX_NUMMSG-1) numMsg++;
		 else numMsg=0;
		 memset(msgQueue[numMsg],'\0',MAX_LONGMSG);
		 // inicialitzaci¢ par…metres Rx
		 memset(bufferRx,'\0',HEADER_VAX+MAX_BUFFER_VAX+2);
		 nBytes=0;
		 ptrRx=0;
		 FiRxInt=0;
	}



  outp (rcm1, tdl | lpe | out2);
  outp (cop2,0x20);      /* Indica Fin de interrupt */
  outp (rci1,0x01);      /* ACTIVA INTERRUPCIONES UART */
}

unsigned int inbyte(unsigned int port)
{
	unsigned char tmp=0;
	unsigned long int timeout=0;
	unsigned int bt,rcm;

	switch (port)
	{
	 case 0x3f8:
		rcm=rccm1;
	 break;

	 case 0x2f8:
		rcm=rccm2;
	 break;
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


void TxMsg(void)
{
 unsigned int long time=0;
 unsigned int i;

 priority=0;
 oclass=0;
 otype=0;
 delivery=0;
 put_timeout=150;
 uma=PDEL_UMA_DISC;
 target.au.group=1;
 target.au.queue=1;

 if (strlen(msgQueue[txMsg])>9)
 {
  printf("\nATENCIO: Condici¢ 3\n");
  msgQueue[txMsg][9]='\0';
 }

 msg_size=strlen(msgQueue[txMsg]);
/* printf("Enviant a la cua %d.%d\n",target.au.group,target.au.queue);*/

 printf("Missatge a DECQ: %s\n",msgQueue[txMsg]);

 sts_value=-1;

 for(;sts_value!=PAMS__SUCCESS && time<TIMEOUT;time++)
  sts_value= pams_put_msg(msgQueue[txMsg],&priority,&target,&oclass,
			&otype,&delivery,&msg_size,&put_timeout,
			&psb,&uma,&resp_que,0,0,0);


 if (sts_value!=PAMS__SUCCESS)
 {
  printf("Error enviant missatge %d\n",sts_value);
  Tx2CGC(7,"ERRHOST");
 }
 if (txMsg<MAX_NUMMSG-1) txMsg++;
 else txMsg=0;
}


void RxMsg(void)
{
  unsigned int long timeout=0;

/*  strcpy(q_name,DMQ_Q_ASIGNA);
  q_name_len=strlen(q_name);
  q_type=PSYM_ATTACH_PQ;
  attach_mode=PSYM_ATTACH_BY_NUMBER;
  proc_num_out.all=0;*/


//  printf("En fase de connexi¢ DMQ_ASSIGNA.\n");
/*  sts_value=-1;

  pams_exit();
  for(;sts_value!=PAMS__SUCCESS && timeout<TIMEOUT;timeout++)
			sts_value=pams_attach_q(&attach_mode,&proc_num_out,
			&q_type,q_name,&q_name_len,0,0,0,0,0);*/

  rcv_priority=0;
  time_out=40;
  sel_addr=0;
  iclass=0;
  itype=0;
  sts_value=-1;

/*  if (sts_value!=PAMS__SUCCESS)
  {
	 printf("Error en la fase de connexi¢ %d.\n",sts_value);
	 Tx2CGC(7,"ERRHOST");
  }
  else
  { */
//   printf("Rebent cua %d.%d ...\n",proc_num_out.au.group,proc_num_out.au.queue);

	  for(;sts_value!=PAMS__SUCCESS && sts_value!=PAMS__NOMOREMSG && sts_value!=PAMS__EXHAUSTBLKS;)
			 sts_value=pams_get_msg(ibuffer,&rcv_priority,
			  &source,&iclass,&itype,&buffer_length,
			  &size,&sel_addr,&psb,0,0,0,0,0);

	  if (sts_value==PAMS__SUCCESS)
	  {
		Tx2CGC(strlen(ibuffer),ibuffer);
		printf( "%s",ibuffer);
	  }
//   else
//    printf("Desconnexi¢ DMQ_ASSIGNA %d.\n",sts_value);
//  }
}
void Ascii2Hex(char *missatge)
{

	int cnt=0;
	int n,i;
	int PosX;
	int PosY;

	printf (" 1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  g       123456789abcdefg\n");
	printf ("-----------------------------------------------       ----------------\n");

	for (n=0;n<strlen(missatge);n+=16)
	{
		for (i=n;i<n+16;i++)
			printf ("%02x ",missatge[i]);

		printf ("      ");
		for (i=n;i<n+16;i++)
		{
			if (missatge[i]=='\n' || missatge[i]=='\a' || missatge[i]=='\0')
				printf (".");
			else
			printf ("%c",missatge[i]);
		}
	  printf ("\n");

	}
	  printf ("\n");
}