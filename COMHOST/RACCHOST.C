/****************************************************************************
 *            COMHOST Copyright(c) ELECTRONICA BARCELONA S.L                *
 *                                 C./Vall d'Aran 27-29                     *
 *                                 El Prat del Llobregat                    *
 *                                 Barcelona                                *
 *                                 Telf : 370-69-55                         *
 *                                 email: ebsl@bcn.servicom.es              *
 *                                                                          *
 * Interface de comunicaciones entre DMSQ y C.G.C.                          *
 *                                                                          *
 *                                                                          *
 ****************************************************************************/

#ifdef __cplusplus
	 #define __CPPARGS ...
#else
	 #define __CPPARGS
#endif

#include "types.h"
#include "p_entry.h"
#include "p_proces.h"
#include "p_typecl.h"
#include "p_return.h"
#include "p_symbol.h"
#include "p_group.h"
#include "head-c.h"

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
char bufferRx[HEADER_VAX+MAX_BUFFER_VAX+2]="";
void interrupt (*ViejIntCOM1)(__CPPARGS);   /*vieja interrupci¢n com1:*/
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

void Tx2CGC(unsigned int numbytes,char *missatge);
void interrupt rxbuff();
void TxMsg(void);
void RxMsg(void);

void main(void)
{
 unsigned int final=0,i;
 unsigned int long timeout=0;

 sio_open (0);
 sio_ctrl (0,9600,P_NONE|BIT_8|STOP_1);
 sio_cnt_irq (0,rxbuff,1);

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
	 if (msgQueue[txMsg][0]!='\0') TxMsg();
  }
  if ((getche())=='S') final=1;
 }while (final==0);
 pams_exit();

 printf("DecMsgQ acaba.");
}

void Tx2CGC(unsigned int numbytes,char *missatge)
{

 unsigned int i=0;
 int PosX;
 int PosY;
 int n;
 int cnt;

 outp (rcm1,(inp(rcm1) | 0x02));

 if (missatge[0]=='A' && missatge[1]=='N') numbytes=17;
 else
  if (missatge[0]=='M') numbytes=34;
  else
	if (missatge[0]=='N' && missatge[1]=='I') numbytes=7;

 sio_write (0,missatge,numbytes);

}


void interrupt rxbuff(__CPPARG)
{
  unsigned int ch=0,FiRxInt=0,length=0,i=0;
  char far *pp;

  ch=sio_getch(0);

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

  rcv_priority=0;
  time_out=40;
  sel_addr=0;
  iclass=0;
  itype=0;
  sts_value=-1;

  for(;sts_value!=PAMS__SUCCESS && sts_value!=PAMS__NOMOREMSG && sts_value!=PAMS__EXHAUSTBLKS;)
		 sts_value=pams_get_msg(ibuffer,&rcv_priority,
		  &source,&iclass,&itype,&buffer_length,
		  &size,&sel_addr,&psb,0,0,0,0,0);

  if (sts_value==PAMS__SUCCESS)
  {
	Tx2CGC(strlen(ibuffer),ibuffer);
	printf( "%s",ibuffer);
  }
}
