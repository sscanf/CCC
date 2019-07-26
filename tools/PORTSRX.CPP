

#include "d:defscgc.h"

void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/
void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/
void interrupt (*ViejIntTIMER)(...);   /*vieja interrupci¢n Timer:*/


union	REGS ent,sal;

unsigned int bufConfig[ACT_CH][ACT_OPER];

extern unsigned int consoles,canals,obrir;


extern unsigned int  comndAct;
extern ptrCmndInfo listCmnd,listCanals;
extern unsigned int lastCmnd; // apuntar… a l'£ltim element de la llista
extern ptrOperInfo listOpers;

extern ptrRadioInfo listIndics;
extern lastIndic;

// DEFINICIONS

void initPorts(void)
{
	unsigned char byte;

	clrscr();
	/* INICIALITZA PORTS */

	if ((IniSio())==NOTOK)
	  error("Error en la inicialitzaci¢ dels ports");

	/* INICIALITZA COMS */

	ent.x.ax = 0xe3;        /* 4800=c3,9600=e3,n,8,1*/
	ent.x.dx = 1;           /* com2 */
	int86(0x14,&ent,&sal);  /* total (OPEN "COM2:9600,N,8,1") */

	ent.x.ax = 0xe3;        /* 9600,n,8,1*/
	ent.x.dx = 0;           /* com1 */
	int86(0x14,&ent,&sal);  /* total (OPEN "COM1:9600,N,8,1") */


	outp (rci1,0x00);		 /* Desactiva interrupciones */
	outp (rci2,0x00);		 /* Desactiva interrupciones */

	outp (rcm1,0x03);      /* Desactiva RTS */
	outp (rcm2,0x03);      /* Desactiva RTS */

	ViejIntCOM2=getvect (11);   /* Obtiene vector int. antigua */
	setvect(11,cojebuff2); 		/* Redefine vector int.Nueva */

	ViejIntCOM1=getvect (12);   /* Obtiene vector int. antigua */
	setvect(12,cojebuff1); 		/* Redefine vector int.Nueva */

	ViejIntTIMER=getvect (0x1c);   /* Obtiene vector int. antigua */
	setvect(0x1c,newTimer); 		/* Redefine vector int.Nueva */


  outp (rcm1, tdl | lpe | out2);
  outp (rcm2, tdl | lpe | out2);

  byte = inp (cop1);
  byte &= irq3;
  outp (cop1, byte);

  outp (rci1,0x09);		 // Activa interrupciones: modem i recepci¢
  outp (rci2,0x01);		 /* Activa interrupciones */

}

int IniSio(void)
{

	int n;

	n=(sio_reset());
	if (n<=0)
	 return (-1);


	// ABRE LOS 8 PORTS

	for (n=3;n<11;n++)
	{

	  // ABRE EL PORT
	  sio_close(n);
	  if ((sio_open (n))!=0) return(-1);

	  // PROGRAMA EL PORT PARA 9600,N,8,1
	  if ((sio_ioctl (n,B9600,BIT_8|STOP_1|P_NONE))!=0)
			 return(-1);

	  if ((sio_overlap (n ,0))!=0) return(-1);
	  if ((sio_flowctrl (n ,0))!=0) return(-1);
	  sio_flush(n,2);

	}

	  // GENERA UNA INTERRUPCION EN CADA PORT PARA CADA BYTE RECIBIDO

	  if ((sio_cnt_irq (3 ,IntPort3 ,1)!=0)) return(-1);  // Rutina puerto 3
	  if ((sio_modem_irq (3 ,IntActiu3)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(3);

	  if ((sio_cnt_irq (4 ,IntPort4 ,1)!=0)) return(-1);  // Rutina puerto 4
	  sio_enableTx(4);

	  if ((sio_cnt_irq (5 ,IntPort5 ,1)!=0)) return(-1);  // Rutina puerto 5
	  sio_enableTx(5);

	  if ((sio_cnt_irq (6 ,IntPort6 ,1)!=0)) return(-1);  // Rutina puerto 6
	  if ((sio_modem_irq (6 ,IntFitxer)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(6); // pel CTS de les plaques ST

	return (0);
}


//---------------------------- INTERRUPCIONES -------------------------------


void colocaCmnd(unsigned int posRefAct,unsigned int *header,char *info,unsigned int origen)
{
	unsigned int comanda=header[0],tempo=0;

	tempo=header[2]&0x00ff;
	tempo=tempo<<8;
	tempo=tempo|header[1];
	posaTasca(posRefAct,info,comanda,tempo,origen);
}

void colocaCmnd2(unsigned int posRefAct,unsigned int *header,char *info,unsigned int origen)
{
	unsigned int comanda=header[3],tempo=0,length;
	char expedient[8]="",servei[3]="";
	int posicio=-1;


	tempo=header[1]&0x00ff;
	tempo=header[1]<<8; // indicatiu high
	tempo=tempo|header[2]; // indicatiu low

	if ((comanda==MSSERVEIS || comanda==MSEMER) && obrir==CODIS)
	{
		itoa(tempo,info,10);
		strcpy(info,ompleZeros(info,3));
	}

	if (comanda==ERR)
	{
	//  Si error en ASI o MOD o ANU,
	//  en .proces la funci¢ en la que tenim l'error
	//  en .origen l'indicatiu
	//  en .info el num d'expedient i servei.
	//
	//  Si el canal estava ocupat en PROGINDIC i retorna error
	//  en els 2 ultims bytes de listCanals.info trobarem l'operador que estava
	//  programant per tal d'enviar-li el missatge d'error.
	//  Ho posarem en el camp .proces de la tasca, gestioErr ja sap a qui ho ha d'enviar.
	//  En .origen l'indicatiu.

	 origen=tempo; // .origen
	 posicio=posicioIndicatiu(tempo);

	 if (posicio!=-1 && (listCanals[PORT6].comnd==ASI || listCanals[PORT6].comnd==MOD
		  || listCanals[PORT6].comnd==ANU))
	 {
	  tempo=listIndics[posicio].lastFunc; // .proces

	  ltoa(listIndics[posicio].expedient,expedient,10);
	  if (strlen(expedient)<7) strcpy(expedient,ompleZeros(expedient,7));

	  strcpy(info,expedient);

	  itoa(listIndics[posicio].servei,servei,10);
	  if (strlen(servei)<2) strcpy(servei,ompleZeros(servei,2));

	  strcat(info,servei);
	 }

	 if (listCanals[PORT6].comnd==PROGINDIC && posicio!=-1)
	 // error de programaci¢ d'indicatiu
	 // posem en .proces l'operador al que ha de retornar
	 {
		length=strlen(listCanals[PORT6].info);
		strcpy(info,listCanals[PORT6].info+length-2);
		tempo=atoi(info); // .proces
	 }
	}

	posaTasca(posRefAct,info,comanda,tempo,origen);
}

void interrupt IntPort3(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(3);

	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 2: // num de proces
		 case 1:
		 case 0: /* comanda identifica el tipus de missatge*/
				bufRxHeader[ptrRx++]=carac;
				break;
		 case 3:/* num de bytes */
				numbytes=carac;
				if (numbytes==0) FiRx=1;
				ptrRx++;
				break;
		 case -2: /* FRAME es 0xec 0xa1*/
				if (carac==0xec) ptrRx++;
				break;
		 case -1:
				if(carac==0xa1) ptrRx++;
				else ptrRx=-2;
				break;
		 default:
				 if (ptrRx<(numbytes+HEADER_OPER) && (ptrRx<(HEADER_OPER+MAX_BUFFER_RX)))
				 {
					bufRxData[ptrRx-HEADER_OPER]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_OPER))   FiRx=1;
				 break;
	  }/* del switch ptrRX*/


	  if (FiRx==1) // FI RECEPCIO
	  {
		 if (bufRxHeader[0]==INI_CONFIG)  // omple el buffer de configuraci¢
		 {
			  for(k=0,j=0;k<consoles && (j<MAX_BUFFER_RX);k++)
				for(q=0;q<canals && (j<MAX_BUFFER_RX);q++,j++)
				 bufConfig[q][k]=bufRxData[j];
		 }

		 /* COL.LOCA COMANDA PER SER EXECUTADA */
		 if (bufRxHeader[0]==INI_CONFIG || bufRxHeader[0]==PETVEUE) /* MAX PRIORITAT: col.loca la comanda despr‚s de comndAct */
			colocaCmnd(0,bufRxHeader,bufRxData,PORT3);
		 else /* al final de la cua de comandes*/
		  if (bufRxHeader[0]!=RES)
			 colocaCmnd(1,bufRxHeader,bufRxData,PORT3);
		 /* Inicialitza els par…metres de recepci¢*/

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(3,0);
	}// tanca if FiRx
  }//tanca carac
}

void interrupt IntPort4(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(4);

	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 2: // num de proces
		 case 1:
		 case 0: /* comanda identifica el tipus de missatge*/
				bufRxHeader[ptrRx++]=carac;
				break;
		 case 3:/* num de bytes */
				numbytes=carac;
				if (numbytes==0) FiRx=1;
				ptrRx++;
				break;
		 case -2: /* FRAME es 0xec 0xa1*/
				if (carac==0xec) ptrRx++;
				break;
		 case -1:
				if(carac==0xa1) ptrRx++;
				else ptrRx=-2;
				break;
		 default:
				 if (ptrRx<(numbytes+HEADER_OPER) && (ptrRx<(HEADER_OPER+MAX_BUFFER_RX)))
				 {
					bufRxData[ptrRx-HEADER_OPER]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_OPER))   FiRx=1;
				 break;
	  }/* del switch ptrRX*/


	  if (FiRx==1) // FI RECEPCIO
	  {
		 if (bufRxHeader[0]==INI_CONFIG)  // omple el buffer de configuraci¢
		 {
			  for(k=0,j=0;k<consoles && (j<MAX_BUFFER_RX);k++)
				for(q=0;q<canals && (j<MAX_BUFFER_RX);q++,j++)
				 bufConfig[q][k]=bufRxData[j];
		 }

		 // COL.LOCA COMANDA PER SER EXECUTADA
		 if (bufRxHeader[0]==INI_CONFIG || bufRxHeader[0]==PETVEUE)
		 // MAX PRIORITAT: col.loca la comanda despr‚s de comndAct
			colocaCmnd(0,bufRxHeader,bufRxData,PORT4);
		 else
		 // al final de la cua de comandes
		  if (bufRxHeader[0]!=RES)
			colocaCmnd(1,bufRxHeader,bufRxData,PORT4);

		 // Inicialitza els par…metres de recepci¢

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(4,0);

	}// tanca if FiRx
  }//tanca carac
}

void interrupt IntPort5(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(5);

	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 2: // num de proces
		 case 1:
		 case 0: /* comanda identifica el tipus de missatge*/
				bufRxHeader[ptrRx++]=carac;
				break;
		 case 3:/* num de bytes */
				numbytes=carac;
				if (numbytes==0) FiRx=1;
				ptrRx++;
				break;
		 case -2: /* FRAME es 0xec 0xa1*/
				if (carac==0xec) ptrRx++;
				break;
		 case -1:
				if(carac==0xa1) ptrRx++;
				else ptrRx=-2;
				break;
		 default:
				 if (ptrRx<(numbytes+HEADER_OPER) && (ptrRx<(HEADER_OPER+MAX_BUFFER_RX)))
				 {
					bufRxData[ptrRx-HEADER_OPER]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_OPER))   FiRx=1;
				 break;
	  }/* del switch ptrRX*/


	  if (FiRx==1) // FI RECEPCIO
	  {
		 if (bufRxHeader[0]==INI_CONFIG)  // omple el buffer de configuraci¢
		 {
			  for(k=0,j=0;k<consoles && (j<MAX_BUFFER_RX);k++)
				for(q=0;q<canals && (j<MAX_BUFFER_RX);q++,j++)
				 bufConfig[q][k]=bufRxData[j];
		 }

		 /* COL.LOCA COMANDA PER SER EXECUTADA */
		 if (bufRxHeader[0]==INI_CONFIG || bufRxHeader[0]==PETVEUE) /* MAX PRIORITAT: col.loca la comanda despr‚s de comndAct */
			colocaCmnd(0,bufRxHeader,bufRxData,PORT5);
		 else /* al final de la cua de comandes*/
		  if (bufRxHeader[0]!=RES) colocaCmnd(1,bufRxHeader,bufRxData,PORT5);
		 /* Inicialitza els par…metres de recepci¢*/

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(5,0);
	}// tanca if FiRx
  }//tanca carac
}

int Tx2Port3(unsigned int comanda,unsigned int numProc,unsigned int numbytes,char *missatge)
{
  int txData,freeSpace;

  sio_enableTx(3);

  txData=sio_oqueue(3);
  freeSpace=sio_ofree(3);
  if (txData>=0 && freeSpace>=numbytes+HEADER_OPER+2)
  {
	  if (sio_lctrl(3,C_RTS)==0)
	  {
		 sio_putch(3,0xec);
		 sio_putch(3,0xa1);
		 sio_putch(3,comanda);
		 // part baixa de numProc
		 sio_putch(3,numProc&0x00ff);
		 // part alta de numProc
		 numProc=numProc>>8;
		 sio_putch(3,numProc);
		 sio_putch(3,numbytes);  // numbytesHigh
		 sio_write(3,missatge,numbytes);
	  }
	  return OK;

  }
  else
	return NOTOK;
}


int Tx2Port4(unsigned int comanda,unsigned int numProc,unsigned int numbytes,char *missatge)
{
  int txData,freeSpace;

  sio_enableTx(4);

  txData=sio_oqueue(4);
  freeSpace=sio_ofree(4);
  if (txData>=0 && freeSpace>=numbytes+HEADER_OPER+2)
  {
	  if (sio_lctrl(4,C_RTS)==0)
	  {
		 sio_putch(4,0xec);
		 sio_putch(4,0xa1);
		 sio_putch(4,comanda);
		 // part baixa de numProc
		 sio_putch(4,numProc&0x00ff);
		 // part alta de numProc
		 numProc=numProc>>8;
		 sio_putch(4,numProc);
		 sio_putch(4,numbytes);  // numbytesHigh
		 sio_write(4,missatge,numbytes);
	  }
	  return OK;

  }
  else
	return NOTOK;
}

int Tx2Port5(unsigned int comanda,unsigned int numProc,unsigned int numbytes,char *missatge)
{
  int txData,freeSpace;

  sio_enableTx(5);

  txData=sio_oqueue(5);
  freeSpace=sio_ofree(5);
  if (txData>=0 && freeSpace>=numbytes+HEADER_OPER+2)
  {
	  if (sio_lctrl(5,C_RTS)==0)
	  {
		 sio_putch(5,0xec);
		 sio_putch(5,0xa1);
		 sio_putch(5,comanda);
		 // part baixa de numProc
		 sio_putch(5,numProc&0x00ff);
		 // part alta de numProc
		 numProc=numProc>>8;
		 sio_putch(5,numProc);
		 sio_putch(5,numbytes);  // numbytesHigh
		 sio_write(5,missatge,numbytes);
	  }
	  return OK;

  }
  else
	return NOTOK;
}



int Tx2Port6(unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
/*
 missatge : 0xec 0xa1 flota indicatiu<2bytes> comanda numblocs

 en bloc: 48 bytes

*/

  int txData,freeSpace,tempo=0,flota=0,posicio=-1;
  char dest[4]="";

  sio_enableTx(6);

  txData=sio_oqueue(6);
  freeSpace=sio_ofree(6);
  if (txData>=0 && freeSpace>=numbytes+HEADER+2+3)
  {
	  if (sio_lctrl(6,C_RTS)==0)
	  {
		sio_putch(6,0xec);
		sio_putch(6,0xa1);


		if (header==1)
		{

		 // busca el prefixe associat a l'indicatiu
		 if ((posicio=posicioIndicatiu(indicatiu))!=-1)
			 flota=listIndics[posicio].flota;

		 sio_putch(6,flota);/* flota */
		 /* indicatiu 2 bytes */
		 /* tempo =  indicatiu high */
		 tempo=indicatiu&0xff00;
		 tempo=tempo>>8;
		 /* indicatiu low */
		 sio_putch(6,tempo);
		 indicatiu=indicatiu&0x00ff;
		 sio_putch(6,indicatiu);
		 sio_putch(6,comanda);  /* tipus missatge */
		 sio_putch(6,numbytes); /* en fitxer indicar… el nombre de blocs,
											=0 en qualsevol altre cas */
	  }

	  if (header==0 || comanda==MOD || comanda==PROGINDIC || comanda==ASI)
	  {
		  // info del bloc en cas de transmissi¢	de fitxer i nou prefixe e indicatiu
		  if (comanda==MOD || comanda==ASI)
			 sio_write(6,missatge,strlen(missatge));
		  else
		  { // PROGINDIC
			 // en el camp .info els tres primers bytes s¢n el prefixe
			 // despr‚s l'indicatiu
			 // i per £ltim l'origen de la comanda, la ST no s'ho mira
			 dest[0]=listCmnd[comndAct].info[3];
			 dest[1]=listCmnd[comndAct].info[4];
			 dest[2]=listCmnd[comndAct].info[5];
			 dest[3]='\0';
			 indicatiu=atoi(dest);
			 posicio=posicioIndicatiu(indicatiu);
			 listIndics[posicio].grup=1;
			 listIndics[posicio].flotaAnt=listIndics[posicio].flota; // es guarda el prefixe antic
			 dest[0]=listCmnd[comndAct].info[0];
			 dest[1]=listCmnd[comndAct].info[1];
			 dest[2]=listCmnd[comndAct].info[2];
			 dest[3]='\0';
			 listIndics[posicio].flota=atoi(dest);    // prefixe nou

			 // Tx
			 sio_putch(6,listIndics[posicio].flota); // prefixe nou
			 tempo=indicatiu&0xff00;
			 tempo=tempo>>8;
			 sio_putch(6,tempo); // indicatiu alt
			 indicatiu=indicatiu&0x00ff;
			 sio_putch(6,indicatiu);    // indicatiu baix
			 sio_putch(6,0);
			 sio_putch(6,0);
			 sio_putch(6,0);
		  }

		}
	  }
	  return OK;
  }
  else
	return NOTOK;
}

int Tx2Port7(unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
/*
 missatge : 0xec 0xa1 flota indicatiu<2bytes> comanda numblocs

 en bloc: 48 bytes

*/
 return OK;
}

int Tx2Port8(unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
/*
 missatge : 0xec 0xa1 flota indicatiu<2bytes> comanda numblocs

 en bloc: 48 bytes

*/
 return OK;
}
int Tx2Port9(unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
/*
 missatge : 0xec 0xa1 flota indicatiu<2bytes> comanda numblocs

 en bloc: 48 bytes

*/
 return OK;
}
int Tx2Port10(unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
/*
 missatge : 0xec 0xa1 flota indicatiu<2bytes> comanda numblocs

 en bloc: 48 bytes

*/
 return OK;
}

void interrupt IntPort6(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int tempo,tempo2,tempo4;
	unsigned int FiRx=0;
	int posicio=-1;
	static unsigned int bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	int inOk=NOTOK;

	carac=sio_getch(6);

	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 3: // comanda que indica el tipus de missatge
		 case 2: // indicatiu low
		 case 1: // indicatiu high
		 case 0: // flota
				  bufRxHeader[ptrRx++]=carac;
				  break;
		 case 4:// num de bytes
				numbytes=carac;
				if (numbytes==0) FiRx=1;
				bufRxHeader[ptrRx++]=carac;
				break;
		 case -2: /* FRAME es 0xec 0xa1*/
				if (carac==0xec) ptrRx++;
				break;
		 case -1:
				if(carac==0xa1) ptrRx++;
				else ptrRx=-2;
				break;
		 default:
				 if (ptrRx<(numbytes+HEADER_RADIO) && (ptrRx<(HEADER_RADIO+MAX_BUFFER_RX)))
				 {
					bufRxData[ptrRx-HEADER_RADIO]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_RADIO))
				    FiRx=1;
				 break;
	  }/* del switch ptrRX*/


	  inOk=listCanals[PORT6].proces;
	  tempo4=listCanals[PORT6].comnd;

	  if ((tempo4==ASI || tempo4==MOD || tempo4==PROGINDIC || tempo4==ANU) && bufRxHeader[3]==SQLON)
		 inOk=OK;

//	  if ((FiRx==1 && inOk==99) || ((bufRxHeader[3]==SQLON || bufRxHeader[3]==SQLOFF) && FiRx==1)) // FI RECEPCIO
	  if (FiRx==1) // per probar els indicatius
	  {
		  tempo=bufRxHeader[1]<<8; // indicatiu high
		  tempo=tempo|bufRxHeader[2]; // indicatiu high|low

		  // entren tots, els que coincideixin amb flota e indicatiu actualitza llista
		  // sin¢ els dono d'alta
		  // en obrir per SQL entren tots i no es mira els indicatius i flota

			posicio=posicioIndicatiu(tempo);

			if (bufRxHeader[3]!=SQLOFF && bufRxHeader[3]!=SQLON && posicio==-1 && obrir==CODIS)
			// alta en la llista si no hi ‚s
			// sin¢ actualitzar
			{
				  posicio=lastIndic;
				  if (lastIndic<MAX_NUM_RADIO) lastIndic++;
			}

			if (bufRxHeader[3]!=SQLON && bufRxHeader[3]!=SQLOFF && obrir==CODIS)
			{
				listIndics[posicio].flota=bufRxHeader[0];
				listIndics[posicio].indicatiu=tempo;
				listIndics[posicio].canal=PORT6;
				listIndics[posicio].subto=listCanals[PORT6].origen;
			}


			if (posicio!=-1 && bufRxHeader[3]==TAN)
				 // baixa en la llista d'indicatius si hi ‚s.
				 baixaIndicatiu(posicio);

			if (bufRxHeader[3]==MEC)
			{
					bufRxHeader[3]=OBE;

					tempo2=bufRxData[1]<<8;     // num Serie high
					tempo2=tempo2|bufRxData[2]; // num Serie low

					listIndics[posicio].numSerie=tempo2;

					memmove(bufRxData,bufRxData+2,sizeof(char)*(strlen(bufRxData)-2));
					bufRxData[strlen(bufRxData)-2]='\0';
			}



			/* COL.LOCA COMANDA PER SER EXECUTADA */
			if (bufRxHeader[3]==EME || bufRxHeader[3]==SQLON)
			// MAX PRIORITAT: col.loca la comanda despr‚s de comndAct
			{
			 if (bufRxHeader[3]==EME) bufRxHeader[3]=MSEMER;
			 colocaCmnd2(0,bufRxHeader,bufRxData,PORT6);
			}
			else /* al final de la cua de comandes*/
			{
				if (bufRxHeader[3]==VEU && listIndics[posicio].permisVeu==0 && obrir==CODIS)
				  bufRxHeader[3]=MSSERVEIS;

				if ((obrir==SILENCIADOR && bufRxHeader[3]!=SQLOFF) || (bufRxHeader[3]!=SQLON && bufRxHeader[3]!=SQLOFF && obrir==CODIS))
				  colocaCmnd2(1,bufRxHeader,bufRxData,PORT6);
				else
				 if (bufRxHeader[3]==SQLOFF && obrir==SILENCIADOR)
				  rstCanal(PORT6);
			}

		 /* Inicialitza els par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;posicio=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(6,0);

	  }// tanca if FiRx
	  else
		if (FiRx==1 && inOk==NOTOK)
		{
		 ptrRx=-2;
		 numbytes=0;carac=0;posicio=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(6,0);

		}
  }//tanca carac
}

void baixaIndicatiu(int posicio)
{
 unsigned int i=0;

 for(i=posicio;i<lastIndic;i++)
 {
  listIndics[i].flota=listIndics[i+1].flota;
  listIndics[i].flotaAnt=listIndics[i+1].flotaAnt;
  listIndics[i].grup=listIndics[i+1].grup;
  listIndics[i].indicatiu=listIndics[i+1].indicatiu;
  listIndics[i].canal=listIndics[i+1].canal;
  listIndics[i].lastFunc=listIndics[i+1].lastFunc;
  listIndics[i].expedient=listIndics[i+1].expedient;
  listIndics[i].servei=listIndics[i+1].servei;
  listIndics[i].subto=listIndics[i+1].subto;
  strcpy(listIndics[i].fitxer,listIndics[i+1].fitxer);
  listIndics[i].permisVeu=listIndics[i+1].permisVeu;
  listIndics[i].numSerie=listIndics[i+1].numSerie;
 }
}

void interrupt IntFitxer()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";

 status2=sio_lstatus(6);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
		// inclou la tasca despr‚s de comndAct
		if (listCanals[PORT6].comnd==PROGINDIC || listCanals[PORT6].comnd==ASI
			 || listCanals[PORT6].comnd==MOD || listCanals[PORT6].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
			// if (listCanals[PORT6].comnd==PROGINDIC)
			// en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
			// Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
			// No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

			if (listCmnd[comndAct].comnd!=RES)
			{
				if ((lastCmnd-comndAct)>=2)
				{
					for(i=lastCmnd;i>comndAct && i<MAX_NUM_COMND;i--)
					{
					 listCmnd[i].origen=listCmnd[i-1].origen;
					 listCmnd[i].estat=listCmnd[i-1].estat;
					 listCmnd[i].comnd=listCmnd[i-1].comnd;
					 listCmnd[i].proces=listCmnd[i-1].proces;
					 strcpy(listCmnd[i].info,listCmnd[i-1].info);
					}

					for(i=0;i<MAX_BLOQUE && listCanals[PORT6].info[i]!='\0';i++)
					 listCmnd[comndAct+1].info[i]=listCanals[PORT6].info[i];
					listCmnd[comndAct+1].info[i]='\0';
					listCmnd[comndAct+1].comnd=listCanals[PORT6].comnd;
					listCmnd[comndAct+1].proces=listCanals[PORT6].proces; // numblocs falten transmetre
					listCmnd[comndAct+1].origen=listCanals[PORT6].origen; // l'indicatiu
					listCmnd[comndAct+1].estat=FIESPERA;
				}
				else
				{
					for(i=0;i<MAX_BLOQUE && listCanals[PORT6].info[i]!='\0';i++)
					 listCmnd[lastCmnd].info[i]=listCanals[PORT6].info[i];
					listCmnd[lastCmnd].info[i]='\0';
					listCmnd[lastCmnd].comnd=listCanals[PORT6].comnd;
					listCmnd[lastCmnd].proces=listCanals[PORT6].proces; // numblocs falten transmetre
					listCmnd[lastCmnd].origen=listCanals[PORT6].origen; // l'indicatiu
					listCmnd[lastCmnd].estat=FIESPERA;
				}
			 }
			 else
			 {
				for(i=0;i<MAX_BLOQUE && listCanals[PORT6].info[i]!='\0';i++)
				 listCmnd[comndAct].info[i]=listCanals[PORT6].info[i];
				listCmnd[comndAct].info[i]='\0';
				listCmnd[comndAct].comnd=listCanals[PORT6].comnd;
				listCmnd[comndAct].proces=listCanals[PORT6].proces; // numblocs falten transmetre
				listCmnd[comndAct].origen=listCanals[PORT6].origen; // l'indicatiu
				listCmnd[comndAct].estat=FIESPERA;
			 }

			 if (lastCmnd<MAX_NUM_COMND)
			 {
				 lastCmnd++;
				 listCmnd[lastCmnd].comnd=RES;
				 strcpy(listCmnd[lastCmnd].info,"");
				 listCmnd[lastCmnd].estat=NOTRUN;
				 listCmnd[lastCmnd].origen=0;
				 listCmnd[lastCmnd].proces=0;
			 }



			if (listCanals[PORT6].proces!=0 && listCanals[PORT6].comnd==ASI)
			{
				length=strlen(listCanals[PORT6].info);
				memmove(listCanals[PORT6].info,(listCanals[PORT6].info)+MAX_BLOQUE,length-MAX_BLOQUE);
				listCanals[PORT6].info[length-MAX_BLOQUE+1]='\0';
			}
			else
				// sï'ha acabat la transmissi¢ incloem la tasca igualment
				listCanals[PORT6].estat=FIESPERA;
				// alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat

		}
		else // canal lliure
		 rstCanal(PORT6);
 }
 else // CTS ON     , canal ocupat
  listCanals[PORT6].estat=ESPERA;
}

void interrupt IntActiu3()
{
 int status2;

 status2=sio_lstatus(6);
 status2=status2&S_DSR; // Data Set Ready

 if (status2!=2) //  Operador no actiu
  listOpers[PORT3].estat=NOACTIU;
}
