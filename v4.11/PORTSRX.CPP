
#include "defscgc.h"

void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/
void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/


union REGS ent,sal;

unsigned int bufConfig[ACT_CH][ACT_OPER];

extern unsigned int consoles,canals,portsOn;


extern int RxFich,DebugMode;
extern char RxDest[MAX_BLOQUE+1];
extern unsigned int  comndAct,obrir;
extern ptrCmndInfo listCmnd,listCanals,listExec;
extern unsigned int lastCmnd; // apuntar… a l'£ltim element de la llista
extern ptrOperInfo listOpers;
extern int ptrBuff;
extern char BuffMsg[1000];
extern ptrRadioInfo listIndics;
extern lastIndic;

char ErrorFichero=0;
extern char err;


// DEFINICIONS


void poolling (void)
{
	IntPort0();
	IntPort1();
	IntPort2();
	IntPort3();
	IntPort4();
	IntPort5();
	IntPort6();
	IntPort7();
	IntPort8();
	IntFitxer3();
	IntFitxer4();
	IntFitxer5();
	IntFitxer6();
}



void initPorts(void)
{
	unsigned char byte;

	/* INICIALITZA PORTS */

	if ((IniSio())==NOTOK)
	  error("Error en la inicialitzaci¢ dels ports");

}

int IniSio(void)
{
	int ret;

	int n;

	// ABRE LOS 8 PORTS

	for (n=0;n<9;n++)
	{

	  // ABRE EL PORT
	  sio_close(n);
	  ret=sio_open (n);

	  // PROGRAMA EL PORT PARA 9600,N,8,1
	  sio_ioctl (n,0xe3);

	  sio_overlap (n ,0);
	  sio_flush(n,0);
	}

	  // GENERA UNA INTERRUPCION EN CADA PORT PARA CADA BYTE RECIBIDO

/*
	  sio_cnt_irq (PORT0 ,IntPort0 ,1);  // Rutina puerto 3

	  sio_cnt_irq (PORT1 ,IntPort1 ,1);  // Rutina puerto 4

	  // Cambiar por IntPort8
	  sio_cnt_irq (PORT2 ,IntPort8 ,1);  // Rutina puerto 5

	  sio_cnt_irq (PORT3,IntPort3 ,1);  // Rutina puerto 6
	  sio_modem_irq (PORT3 ,IntFitxer3);  // Rutina puerto 6
	  sio_cnt_irq (PORT4 ,IntPort4 ,1);  // Rutina puerto 6
	  sio_modem_irq (PORT4 ,IntFitxer4);  // Rutina puerto 6

	  sio_cnt_irq (PORT5 ,IntPort5 ,1);  // Rutina puerto 6
	  sio_modem_irq (PORT5 ,IntFitxer5);  // Rutina puerto 6

	  sio_cnt_irq (PORT6 ,IntPort6,1);  // Rutina puerto 6
	  sio_modem_irq (PORT6 ,IntFitxer6);  // Rutina puerto 6

	  sio_cnt_irq (PORT7 ,IntPort7 ,1);  // Rutina puerto 6
//	  sio_modem_irq (PORT7 ,IntFitxer7); return(-1);  // Rutina puerto 6

	  sio_cnt_irq (PORT8 ,IntPort2 ,1);  // Rutina puerto 6
*/
	  return (0);
}


//---------------------------- INTERRUPCIONES -------------------------------


void colocaCmnd(unsigned int posRefAct,unsigned char *header,char *info,unsigned int origen)
{
	unsigned int comanda=header[0],tempo=0;
	unsigned int canal=header[3];

	tempo=header[2]&0x00ff;
	tempo=tempo<<8;
	tempo=tempo|header[1];
	posaTasca(posRefAct,info,comanda,tempo,canal,origen,NOTRUN);
}

void colocaCmnd2(unsigned int posRefAct,unsigned char *header,char *info,unsigned int origen)
{
	unsigned int comanda=header[3],tempo=0,length,origen2;
	char expedient[8]="",servei[3]="";
	int posicio=-1;
	unsigned int canal=0;


	tempo=header[1]&0x00ff;
	tempo=header[1]<<8; // indicatiu high
	tempo=tempo|header[2]; // indicatiu low

	// col.locarem el num de mec…nic en el camp .info, ja que ‚s aquest el que ha
	// de sortir en la finestra d'incidŠncies.
	// Aix¡ doncs, canviem l'indicatiu pel num de mec…nic que es troba en ASCII
	// en bufRxData

	// Activar-ho en cas que es vulgui el num d'indicatiu enlloc del num de mec…nic
	// Desactivar-ho pel num de mec…nic.
	if (comanda==MSSERVEIS || comanda==MSEMER)
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

	 origen2=tempo; // .origen l'indicatiu
	 posicio=posicioIndicatiu(tempo);


	 if (posicio!=-1 && (listCanals[origen].comnd==ASI || listCanals[origen].comnd==MOD
		  || listCanals[origen].comnd==ANU))
	 {
	  err=1;
	  tempo=listIndics[posicio].lastFunc; // .proces

	  ltoa(listIndics[posicio].expedient,expedient,10);
	  if (strlen(expedient)<7) strcpy(expedient,ompleZeros(expedient,7));

	  strcpy(info,expedient);

	  itoa(listIndics[posicio].servei,servei,10);
	  if (strlen(servei)<2) strcpy(servei,ompleZeros(servei,2));

	  strcat(info,servei);
	 }

	 if (listCanals[origen].comnd==PROGINDIC && posicio!=-1)
	 // error de programaci¢ d'indicatiu
	 // posem en .proces l'operador al que ha de retornar
	 {
		length=strlen(listCanals[origen].info);
		strcpy(info,listCanals[origen].info+length-2);
		tempo=atoi(info); // .proces
	 }
	 origen=origen2; // .origen
	}

	posaTasca(posRefAct,info,comanda,tempo,canal,origen,NOTRUN);
}

void IntPort0(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned char bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static int fram=0;

	carac=sio_getch(PORT0);

	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 3: // num de canal
		 case 2: // num de proces
		 case 1:
		 case 0: /* comanda identifica el tipus de missatge*/
				bufRxHeader[ptrRx++]=carac;
				break;
		 case 4:/* num de bytes */
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
		 gestRxIntOpers(PORT0,bufRxHeader,bufRxData);

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT0,0);
	}// tanca if FiRx
  }//tanca carac
}
void IntPort1(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int FiRx=0;
	static unsigned char bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static int fram=0;


	carac=sio_getch(PORT1);
	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 3: // num de canal
		 case 2: // num de proces
		 case 1:
		 case 0: /* comanda identifica el tipus de missatge*/
				bufRxHeader[ptrRx++]=carac;
				break;
		 case 4:/* num de bytes */
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
		 gestRxIntOpers(PORT1,bufRxHeader,bufRxData);

		 /* Inicialitza els par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT1,0);
	}// tanca if FiRx
  }//tanca carac
}


void  IntPort8(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned char bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static int fram=0;

	carac=sio_getch(PORT2);

	if (carac>=0)
	{
	  switch(ptrRx)
	  {
		 case 3: // num de canal
		 case 2: // num de proces
		 case 1:
		 case 0: /* comanda identifica el tipus de missatge*/
				bufRxHeader[ptrRx++]=carac;
				break;
		 case 4:/* num de bytes */
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
		 gestRxIntOpers(2,bufRxHeader,bufRxData);

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT2,0);
	}// tanca if FiRx
  }//tanca carac
}

int Tx2PortO(unsigned int operador,unsigned int comanda,unsigned int numProc, unsigned int numbytes,char *missatge)
 {
  int txData,freeSpace;

//  sio_enableTx(operador);

//  txData=sio_oqueue(operador);
  freeSpace=sio_ofree(operador);
//  gotoxy (1,20);
//  printf ("%d",freeSpace);
//  gotoxy (1,21);
//  printf ("%d",txData);
  if (freeSpace>=numbytes+HEADER_OPER+2)
  {
	  if (sio_lctrl(operador,C_RTS)==0)
	  {
		 sio_putch(operador,0xec);
		 sio_putch(operador,0xa1);
		 sio_putch(operador,comanda);
		 // part baixa de numProc
		 sio_putch(operador,numProc&0x00ff);
		 // part alta de numProc
		 numProc=numProc>>8;
		 sio_putch(operador,numProc);
		 sio_putch(operador,numbytes);  // numbytesHigh
		 sio_write(operador,missatge,numbytes);
	  }
	  return OK;
  }
  else
	return NOTOK;
}


int Tx2PortR(unsigned int canal,unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
/*
 missatge : 0xec 0xa1 flota indicatiu<2bytes> comanda numblocs

 en bloc: 48 bytes

*/

  int txData,freeSpace,tempo=0,flota=0,posicio=-1;
  char dest[4]="";
  int bt;

//  sio_enableTx(canal);

//  txData=sio_oqueue(canal);
  freeSpace=sio_ofree(canal);
  if (freeSpace>=numbytes+HEADER+2+3)
  {
	  if (sio_lctrl(canal,C_RTS)==0)
	  {
		sio_putch(canal,0xec);
		sio_putch(canal,0xa1);


		if (header==1)
		{
		 // busca el prefixe associat a l'indicatiu
		 if ((posicio=posicioIndicatiu(indicatiu))!=-1)
			 flota=listIndics[posicio].flota;

		 sio_putch(canal,flota);/* flota */
		 /* indicatiu 2 bytes */
		 /* tempo =  indicatiu high */
		 tempo=indicatiu&0xff00;
		 tempo=tempo>>8;
		 /* indicatiu low */
		 sio_putch(canal,tempo);
		 indicatiu=indicatiu&0x00ff;
		 sio_putch(canal,indicatiu);
		 sio_putch(canal,comanda);  /* tipus missatge */
		 sio_putch(canal,numbytes); /* en fitxer indicar… el nombre de blocs,
											=0 en qualsevol altre cas */
	  }

	  if (header==0 || comanda==MOD || comanda==PROGINDIC || comanda==ASI)
	  {
		  // info del bloc en cas de transmissi¢  de fitxer i nou prefixe e indicatiu
		  if (comanda==MOD || comanda==ASI)
		  {
			 if (sio_ofree(canal)>=strlen (missatge)) sio_write(canal,missatge,strlen(missatge));
		  }

		  else
		  { // PROGINDIC
			 // en el camp .info els tres primers bytes s¢n el prefixe
			 // despr‚s l'indicatiu
			 // i per £ltim l'origen de la comanda, la ST no s'ho mira
			 dest[0]=listExec[0].info[3];
			 dest[1]=listExec[0].info[4];
			 dest[2]=listExec[0].info[5];
			 dest[3]='\0';
			 indicatiu=atoi(dest);
			 posicio=posicioIndicatiu(indicatiu);
			 listIndics[posicio].grup=1;
			 listIndics[posicio].flotaAnt=listIndics[posicio].flota; // es guarda el prefixe antic
			 dest[0]=listExec[0].info[0];
			 dest[1]=listExec[0].info[1];
			 dest[2]=listExec[0].info[2];
			 dest[3]='\0';
			 listIndics[posicio].flota=atoi(dest);    // prefixe nou

			 // Tx
			 sio_putch(canal,listIndics[posicio].flota); // prefixe nou
			 tempo=indicatiu&0xff00;
			 tempo=tempo>>8;
			 sio_putch(canal,tempo); // indicatiu alt
			 indicatiu=indicatiu&0x00ff;
			 sio_putch(canal,indicatiu);    // indicatiu baix
			 sio_putch(canal,0);
			 sio_putch(canal,0);
			 sio_putch(canal,0);
		  }

		}
	  }

  if (!DebugMode)
  {
	  gotoxy (20,23);
	  printf ("        ");
  }
	  return OK;
  }
  else
	return NOTOK;
}

void IntPort3(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int tempo,tempo2;
	unsigned int FiRx=0;
	int posicio=-1;
	static unsigned char bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	do {

	carac=sio_getch(PORT3);

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
				if (carac==0xec)
				 ptrRx++;
				break;
		 case -1:
				if(carac==0xa1)
					ptrRx++;
				else ptrRx=-2;
				break;

		 default:
		 if (ptrRx<(numbytes+HEADER_RADIO) && (ptrRx<(HEADER_RADIO+MAX_BUFFER_RX)))
		 {
			if (carac==0) carac='0';
				bufRxData[ptrRx-HEADER_RADIO]=(char)carac;
				ptrRx++;
			 }
				 if (ptrRx>=(numbytes+HEADER_RADIO))
				 FiRx=1;
		 break;
	  }// del switch ptrRx



	  if (FiRx==1) // per probar els indicatius
	  {

		 gestioRxIntRadio(0,bufRxHeader,bufRxData);

		 // Inicialitza els par…metres de recepci¢
		 ptrRx=-2;
		 numbytes=0;carac=0;FiRx=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,0,MAX_BUFFER_RX);
		 sio_flush(PORT3,0);

	  }// tanca if FiRx
	}//tanca carac

	}while (carac>=0);
}

void IntPort4(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int tempo,tempo2;
	unsigned int FiRx=0;
	int posicio=-1;
	static unsigned char bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static int fram=0;

	carac=sio_getch(PORT4);

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
					if (carac==0) carac='0';

					bufRxData[ptrRx-HEADER_RADIO]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_RADIO))
					 FiRx=1;
				 break;
	  }/* del switch ptrRX*/


	  if (FiRx==1) // per probar els indicatius
	  {
		 gestioRxIntRadio(1,bufRxHeader,bufRxData);

		 /* Inicialitza els par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;FiRx=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT4,0);

	  }// tanca if FiRx
	}//tanca carac
}

void IntPort5(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int FiRx=0;
	static unsigned char bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static int fram=0;

	carac=sio_getch(PORT5);

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
					if (carac==0) carac='0';
					bufRxData[ptrRx-HEADER_RADIO]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_RADIO))
					 FiRx=1;
				break;
	  }/* del switch ptrRX*/


	  if (FiRx==1) // per probar els indicatius
	  {
		 gestioRxIntRadio(2,bufRxHeader,bufRxData);


		 /* Inicialitza eFls par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT5,0);

	  }// tanca if FiRx
	}//tanca carac
}

void IntPort6(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int FiRx=0;
	static unsigned char bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static unsigned long int i=0;
	char far *pp;
	static int fram=0;



	carac=sio_getch(PORT6);
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
				if (numbytes==0)
				  FiRx=1;
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
					if (carac==0) carac='0';
					bufRxData[ptrRx-HEADER_RADIO]=(char)carac;
					ptrRx++;
				 }

				 if (ptrRx>=(numbytes+HEADER_RADIO))
					 FiRx=1;
				break;
	  }/* del switch ptrRX*/


	  if (FiRx==1) // per probar els indicatius
	  {

		 gestioRxIntRadio(3,bufRxHeader,bufRxData);

		 /* Inicialitza els par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT6,0);
	  }// tanca if FiRx
	}//tanca cara
}

void IntPort7(void)
{
  int ch=0;
  static int ptrRx=-2;

	ch=sio_getch(PORT7);
	 if (ch>=0)
	 {
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

			listExec[0].proces=ch;
			listExec[0].estat=FIESPERA;
			ptrRx=-2;
			sio_flush(PORT7,0);
		break;
	  }// del switch
	 }
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
  listIndics[i].numReTx=listIndics[i+1].numReTx;
 }
 if (lastIndic>0) lastIndic--;
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
  listIndics[i].numReTx=listIndics[i+1].numReTx;

}

void IntFitxer3()
{
 int status2;
 static char bt='a';
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT3);

	status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {

  if (listCanals[0].comnd==PROGINDIC || listCanals[0].comnd==ASI
		|| listCanals[0].comnd==MOD || listCanals[0].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
  {
  // if (listCanals[3].comnd==PROGINDIC)
  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[0].info[i]!='\0';i++)
	 dest[i]=listCanals[0].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	 posaTasca(1,dest,listCanals[0].comnd,
	  listCanals[0].proces,0,listCanals[0].origen,FIESPERA);
//		comndAct=lastCmnd-1;
/*	RxFich=1;
	memcpy (RxDest,dest,i);*/

	if (listCanals[0].proces!=0 && listCanals[0].comnd==ASI)
	{
	  length=strlen(listCanals[0].info);
	  memmove(listCanals[0].info,(listCanals[0].info)+MAX_BLOQUE,length-MAX_BLOQUE);

/*		for (int n=0;n<3;n++)
		{
		  IntPort3 ();
		  delay (1);
		  if (ErrorFichero)
			printf ("Error");
		}*/

	  listCanals[0].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	{
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[0].estat=FIESPERA;
	 listCanals[0].flag=0;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
	}
  }
  else // canal lliure
  {
		rstCanal(0);
  }
 }
 else // CTS ON     , canal ocupat

  listCanals[0].estat=ESPERA;
}

void IntFitxer4()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT4);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[1].comnd==PROGINDIC || listCanals[1].comnd==ASI
		|| listCanals[1].comnd==MOD || listCanals[1].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
	  // if (listCanals[3].comnd==PROGINDIC)
	  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
	  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
	  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[1].info[i]!='\0';i++)
	 dest[i]=listCanals[1].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[1].comnd,
	  listCanals[1].proces,0,listCanals[1].origen,FIESPERA);

	if (listCanals[1].proces!=0 && listCanals[1].comnd==ASI)
	{
	  length=strlen(listCanals[1].info);
	  memmove(listCanals[1].info,(listCanals[1].info)+MAX_BLOQUE,length-MAX_BLOQUE);
	  listCanals[1].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[1].estat=FIESPERA;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
		 }
  else // canal lliure
	rstCanal(1);
 }
 else // CTS ON     , canal ocupat
  listCanals[1].estat=ESPERA;

}

void IntFitxer5()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT5);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[2].comnd==PROGINDIC || listCanals[2].comnd==ASI
		|| listCanals[2].comnd==MOD || listCanals[2].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
	  // if (listCanals[3].comnd==PROGINDIC)
	  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
	  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
	  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[2].info[i]!='\0';i++)
	 dest[i]=listCanals[2].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[2].comnd,
	  listCanals[2].proces,0,listCanals[2].origen,FIESPERA);

	if (listCanals[2].proces!=0 && listCanals[2].comnd==ASI)
	{
	  length=strlen(listCanals[2].info);
	  memmove(listCanals[2].info,(listCanals[2].info)+MAX_BLOQUE,length-MAX_BLOQUE);
	  listCanals[2].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[2].estat=FIESPERA;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
		 }
  else // canal lliure
	rstCanal(2);
 }
 else // CTS ON     , canal ocupat
  listCanals[2].estat=ESPERA;

}

void IntFitxer6()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT6);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[3].comnd==PROGINDIC || listCanals[3].comnd==ASI
		|| listCanals[3].comnd==MOD || listCanals[3].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
	  // if (listCanals[3].comnd==PROGINDIC)
	  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
	  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
	  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[3].info[i]!='\0';i++)
	 dest[i]=listCanals[3].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[3].comnd,
	  listCanals[3].proces,0,listCanals[3].origen,FIESPERA);


	if (listCanals[3].proces!=0 && listCanals[3].comnd==ASI)
	{
	  length=strlen(listCanals[3].info);
	  memmove(listCanals[3].info,(listCanals[3].info)+MAX_BLOQUE,length-MAX_BLOQUE);
	  listCanals[3].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[3].estat=FIESPERA;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
		 }
  else // canal lliure
	rstCanal(3);
 }
 else // CTS ON     , canal ocupat
  listCanals[3].estat=ESPERA;
}

void IntActiu0()
{
 int status2;

 status2=sio_lstatus(PORT0);
 status2=status2&S_DSR; // Data Set Ready

 if (status2!=2) //  Operador no actiu
  listOpers[PORT0].estat=NOACTIU;

}

void gestRxIntOpers(unsigned int origen,unsigned char *header,char *info)
{
	unsigned int k,q,j,i;
	int posicio,tmp;
	int n=1;

		 if (header[0]==INI_CONFIG)  // omple el buffer de configuraci¢
		 {
			  for(k=0,j=0;k<consoles && (j<MAX_BUFFER_RX);k++)
				for(q=0;q<canals && (j<MAX_BUFFER_RX);q++,j++)
					 bufConfig[q][k]=info[j];
		 }

		// Si el indicativo ya existe, lo quita para despu‚s ponerlo.

		 if (header[0]==TRUCINDIV)
		 {
			tmp=atoi (info);
			posicio=posicioIndicatiu(tmp);

			if (posicio!=-1)
			// baixa en la llista d'indicatius si hi ‚s.
				 baixaIndicatiu(posicio);
		 }

		 // COL.LOCA COMANDA PER SER EXECUTADA
		 if (header[0]!=RES)
			 colocaCmnd(1,header,info,origen);
}

void gestioRxIntRadio(unsigned int origen,unsigned char *header,char *info)
{
	unsigned int tempo=0,tempo2=0;
	int posicio=-1;
	char far *pp;
	static unsigned long int i=0;
	tempo=header[1]<<8; // indicatiu high
	tempo=tempo|header[2]; // indicatiu high|low

	  // entren tots, els que coincideixin amb flota e indicatiu actualitza llista
	  // sin¢ els dono d'alta
	  // en obrir per SQL entren tots i no es mira els indicatius i flota

			posicio=posicioIndicatiu(tempo);
			if (header[3]!=SQLOFF && header[3]!=SQLON && header[3]!=TAN && posicio==-1)
			// si no hi ‚s en la llista es d¢na d'alta
			// sin¢ actualitzar
			{
				  posicio=lastIndic;
				  if (lastIndic<MAX_NUM_RADIO) lastIndic++;
			}

//			if (header[3]==ERR) ErrorFichero=1;

			if (header[3]!=SQLON && header[3]!=SQLOFF && header[3]!=TAN)
			{
				listIndics[posicio].flota=header[0];
				listIndics[posicio].indicatiu=tempo;
				listIndics[posicio].canal=origen;
				listIndics[posicio].subto=listCanals[origen].origen;
			}


			if (header[3]==TAN && posicio!=-1)
			// baixa en la llista d'indicatius si hi ‚s.
				 baixaIndicatiu(posicio);

			if (header[3]==MEC)
			{
					header[3]=OBE;

					tempo2=info[0]<<8;     // num Serie high
					tempo2=tempo2|info[1]; // num Serie low

					listIndics[posicio].numSerie=tempo2;


					memmove(info,info+2,sizeof(char)*(strlen(info)-2));
					info[strlen(info)-2]='\0';
					strcpy(listIndics[posicio].numMecanic,info);
			}

			// COL.LOCA COMANDA PER SER EXECUTADA
			if (header[3]==EME) header[3]=MSEMER;
			if (header[3]==VEU) header[3]=MSSERVEIS;

			if (obrir==SILENCIADOR || (header[3]!=SQLON && header[3]!=SQLOFF && obrir==CODIS))
			{
				  colocaCmnd2(1,header,info,origen);
				  pp=(char far *)MK_FP(0xb800,0);
				  *pp=i++;
			}
}


