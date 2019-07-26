
#include "d:defscgc.h"

void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/
void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/
void interrupt (*ViejIntTIMER)(...);   /*vieja interrupci¢n Timer:*/


union REGS ent,sal;

unsigned int bufConfig[ACT_CH][ACT_OPER];

extern unsigned int consoles,canals,portsOn;



extern unsigned int  comndAct,obrir;
extern ptrCmndInfo listCmnd,listCanals,listExec;
extern unsigned int lastCmnd; // apuntar… a l'£ltim element de la llista
extern ptrOperInfo listOpers;

extern ptrRadioInfo listIndics;
extern lastIndic;

// DEFINICIONS

void initPorts(void)
{
	unsigned char byte;

//	ViejIntTIMER = getvect (0x1c);
//	setvect(0x1c,IntTIMER); 		/* Redefine vector int.Nueva */

	/* INICIALITZA PORTS */

	if ((IniSio())==NOTOK)
	  error("Error en la inicialitzaci¢ dels ports");

}

int IniSio(void)
{

	int n;

	n=(sio_reset());
	if (n<=0)
	 return (-1);


	// ABRE LOS 8 PORTS

	for (n=0;n<8;n++)
	{

	  // ABRE EL PORT
	  sio_close(n);
	  if ((sio_open (n))!=0) return(-1);

	  // PROGRAMA EL PORT PARA 9600,N,8,1
	  if ((sio_ioctl (n,B9600,BIT_8|STOP_1|P_NONE))!=0)
			 return(-1);

	  if ((sio_overlap (n ,0))!=0) return(-1);
	  if ((sio_flowctrl (n ,0))!=0) return(-1);
	  sio_flush(n,0);

	}

	  // GENERA UNA INTERRUPCION EN CADA PORT PARA CADA BYTE RECIBIDO

	  if ((sio_cnt_irq (PORT0 ,IntPort0 ,1)!=0)) return(-1);  // Rutina puerto 3
	  if ((sio_modem_irq (PORT0 ,IntActiu0)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(PORT0);

	  if ((sio_cnt_irq (PORT1 ,IntPort1 ,1)!=0)) return(-1);  // Rutina puerto 4
	  sio_enableTx(PORT1);

	  if ((sio_cnt_irq (PORT2 ,IntPort2 ,1)!=0)) return(-1);  // Rutina puerto 5
	  sio_enableTx(PORT2);

	  if ((sio_cnt_irq (PORT3+3 ,IntPort3 ,1)!=0)) return(-1);  // Rutina puerto 6
	  if ((sio_modem_irq (PORT3+3 ,IntFitxer3)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(PORT3+3); // pel CTS de les plaques ST

	  if ((sio_cnt_irq (PORT4+3 ,IntPort4 ,1)!=0)) return(-1);  // Rutina puerto 6
	  if ((sio_modem_irq (PORT4+3 ,IntFitxer4)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(PORT4+3); // pel CTS de les plaques ST

	  if ((sio_cnt_irq (PORT5+3 ,IntPort5 ,1)!=0)) return(-1);  // Rutina puerto 6
	  if ((sio_modem_irq (PORT5+3 ,IntFitxer5)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(PORT5+3); // pel CTS de les plaques ST

	  if ((sio_cnt_irq (PORT6+3 ,IntPort6,1)!=0)) return(-1);  // Rutina puerto 6
	  if ((sio_modem_irq (PORT6+3 ,IntFitxer6)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(PORT6+3); // pel CTS de les plaques ST

	  if ((sio_cnt_irq (PORT7+3 ,IntPort7 ,1)!=0)) return(-1);  // Rutina puerto 6
//   if ((sio_modem_irq (PORT7+3 ,IntFitxer7)!=0)) return(-1);  // Rutina puerto 6
	  sio_enableTx(PORT7+3); // pel CTS de les plaques ST

	return (0);
}


//---------------------------- INTERRUPCIONES -------------------------------


void colocaCmnd(unsigned int posRefAct,unsigned int *header,char *info,unsigned int origen)
{
	unsigned int comanda=header[0],tempo=0;

	tempo=header[2]&0x00ff;
	tempo=tempo<<8;
	tempo=tempo|header[1];
	posaTasca(posRefAct,info,comanda,tempo,origen,NOTRUN);
}

void colocaCmnd2(unsigned int posRefAct,unsigned int *header,char *info,unsigned int origen)
{
	unsigned int comanda=header[3],tempo=0,length,origen2;
	char expedient[8]="",servei[3]="";
	int posicio=-1;


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

	posaTasca(posRefAct,info,comanda,tempo,origen,NOTRUN);
}

void interrupt IntPort0(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(PORT0);
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
		 gestRxIntOpers(PORT0,bufRxHeader,bufRxData);

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT0,0);
	}// tanca if FiRx
  }//tanca carac
}

void interrupt IntPort1(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(PORT1);
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


int Tx2PortO(unsigned int operador,unsigned int comanda,unsigned int numProc,unsigned int numbytes,char *missatge)
{
  int txData,freeSpace;

  sio_enableTx(operador);

  txData=sio_oqueue(operador);
  freeSpace=sio_ofree(operador);
  if (txData>=0 && freeSpace>=numbytes+HEADER_OPER+2)
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

  sio_enableTx(canal);

  txData=sio_oqueue(canal);
  freeSpace=sio_ofree(canal);
  if (txData>=0 && freeSpace>=numbytes+HEADER+2+3)
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
			 sio_write(canal,missatge,strlen(missatge));
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
	  return OK;
  }
  else
	return NOTOK;
}


void interrupt IntPort3(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int tempo,tempo2;
	unsigned int FiRx=0;
	int posicio=-1;
	static unsigned int bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(PORT3+3);
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
	  }// del switch ptrRx


	  if (FiRx==1) // per probar els indicatius
	  {

		 gestioRxIntRadio(PORT3,bufRxHeader,bufRxData);

		 // Inicialitza els par…metres de recepci¢
		 ptrRx=-2;
		 numbytes=0;carac=0;FiRx=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT3+3,0);

	  }// tanca if FiRx
	}//tanca carac
}

void interrupt IntPort4(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int tempo,tempo2;
	unsigned int FiRx=0;
	int posicio=-1;
	static unsigned int bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(PORT4+3);
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
		 gestioRxIntRadio(PORT4,bufRxHeader,bufRxData);

		 /* Inicialitza els par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;FiRx=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT4+3,0);

	  }// tanca if FiRx
	}//tanca carac
}

void interrupt IntPort5(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(PORT5+3);
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
		 gestioRxIntRadio(PORT5,bufRxHeader,bufRxData);


		 /* Inicialitza eFls par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT5+3,0);

	  }// tanca if FiRx
	}//tanca carac
}

void interrupt IntPort6(void)
{
/* El missatge que s'espera rebre ‚s: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llarg…ria>*/
	int carac;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX];
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static unsigned long int i=0;
	char far *pp;



	carac=sio_getch(PORT6+3);
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

		 gestioRxIntRadio(PORT6,bufRxHeader,bufRxData);

		 /* Inicialitza els par…metres de recepci¢*/
		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT6+3,0);
	  }// tanca if FiRx
	}//tanca carac
}

void interrupt IntPort7(void)
{
  unsigned int ch=0;
  static int ptrRx=-2;

	ch=sio_getch(PORT7+3);
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
		sio_flush(PORT7+3,0);
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

void interrupt IntFitxer3()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT3+3);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[PORT3].comnd==PROGINDIC || listCanals[PORT3].comnd==ASI
		|| listCanals[PORT3].comnd==MOD || listCanals[PORT3].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
  {
  // if (listCanals[PORT6].comnd==PROGINDIC)
  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[PORT3].info[i]!='\0';i++)
	 dest[i]=listCanals[PORT3].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[PORT3].comnd,
	  listCanals[PORT3].proces,listCanals[PORT3].origen,FIESPERA);

	if (listCanals[PORT3].proces!=0 && listCanals[PORT3].comnd==ASI)
	{
	  length=strlen(listCanals[PORT3].info);
	  memmove(listCanals[PORT3].info,(listCanals[PORT3].info)+MAX_BLOQUE,length-MAX_BLOQUE);
	  listCanals[PORT3].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[PORT3].estat=FIESPERA;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
  }
  else // canal lliure
	rstCanal(PORT3);
 }
 else // CTS ON     , canal ocupat
  listCanals[PORT3].estat=ESPERA;
}

void interrupt IntFitxer4()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT4+3);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[PORT4].comnd==PROGINDIC || listCanals[PORT4].comnd==ASI
		|| listCanals[PORT4].comnd==MOD || listCanals[PORT4].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
	  // if (listCanals[PORT6].comnd==PROGINDIC)
	  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
	  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
	  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[PORT4].info[i]!='\0';i++)
	 dest[i]=listCanals[PORT4].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[PORT4].comnd,
	  listCanals[PORT4].proces,listCanals[PORT4].origen,FIESPERA);

	if (listCanals[PORT4].proces!=0 && listCanals[PORT4].comnd==ASI)
	{
	  length=strlen(listCanals[PORT4].info);
	  memmove(listCanals[PORT4].info,(listCanals[PORT4].info)+MAX_BLOQUE,length-MAX_BLOQUE);
	  listCanals[PORT4].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[PORT4].estat=FIESPERA;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
		 }
  else // canal lliure
	rstCanal(PORT4);
 }
 else // CTS ON     , canal ocupat
  listCanals[PORT4].estat=ESPERA;
}

void interrupt IntFitxer5()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT5+3);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[PORT5].comnd==PROGINDIC || listCanals[PORT5].comnd==ASI
		|| listCanals[PORT5].comnd==MOD || listCanals[PORT5].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
	  // if (listCanals[PORT6].comnd==PROGINDIC)
	  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
	  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
	  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[PORT5].info[i]!='\0';i++)
	 dest[i]=listCanals[PORT5].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[PORT5].comnd,
	  listCanals[PORT5].proces,listCanals[PORT5].origen,FIESPERA);

	if (listCanals[PORT5].proces!=0 && listCanals[PORT5].comnd==ASI)
	{
	  length=strlen(listCanals[PORT5].info);
	  memmove(listCanals[PORT5].info,(listCanals[PORT5].info)+MAX_BLOQUE,length-MAX_BLOQUE);
	  listCanals[PORT5].info[length-MAX_BLOQUE+1]='\0';
	}
	else
	 // sï'ha acabat la transmissi¢ incloem la tasca igualment
	 listCanals[PORT5].estat=FIESPERA;
	 // alliberem el canal quan la transmissi¢ de tot el fitxer s'ha acabat
		 }
  else // canal lliure
	rstCanal(PORT5);
 }
 else // CTS ON     , canal ocupat
  listCanals[PORT5].estat=ESPERA;
}

void interrupt IntFitxer6()
{
 int status2;
 unsigned int length,i;
 char tempo[2]="";
 char dest[MAX_BLOQUE+1]="";

 status2=sio_lstatus(PORT6+3);

 status2=status2&S_CTS;

 if (status2==0) //  CTS OFF , canal lliure
 {
  if (listCanals[PORT6].comnd==PROGINDIC || listCanals[PORT6].comnd==ASI
		|| listCanals[PORT6].comnd==MOD || listCanals[PORT6].comnd==ANU)
		// transmetent un fitxer o programant indicatiu, no deixem el canal lliure fins al final
		{
	  // if (listCanals[PORT6].comnd==PROGINDIC)
	  // en camp .info tenim al final de l'string dos bytes que m'indiquen l'origen del missatge en ASCII.
	  // Ens serveix per retornar error a l'operador en el cas que el m•bil no s'hagi pogut programar
	  // No hi ha problemes encara que ho enviem ja que ST no s'ho mira.

	for(i=0;i<MAX_BLOQUE && listCanals[PORT6].info[i]!='\0';i++)
	 dest[i]=listCanals[PORT6].info[i];
	dest[i]='\0';
	  // en .proces numblocs falten transmetre
	  // en .origen l'indicatiu
	posaTasca(0,dest,listCanals[PORT6].comnd,
	  listCanals[PORT6].proces,listCanals[PORT6].origen,FIESPERA);


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

void interrupt IntActiu0()
{
 int status2;

 status2=sio_lstatus(PORT0);
 status2=status2&S_DSR; // Data Set Ready

 if (status2!=2) //  Operador no actiu
  listOpers[PORT0].estat=NOACTIU;
}

void gestRxIntOpers(unsigned int origen,unsigned int *header,char *info)
{
	unsigned int k,q,j,i;

		 if (header[0]==INI_CONFIG)  // omple el buffer de configuraci¢
		 {
			  for(k=0,j=0;k<consoles && (j<MAX_BUFFER_RX);k++)
				for(q=0;q<canals && (j<MAX_BUFFER_RX);q++,j++)
				 bufConfig[q][k]=info[j];
		 }

		 // COL.LOCA COMANDA PER SER EXECUTADA
		 if (header[0]!=RES)
			 colocaCmnd(1,header,info,origen);

}

void gestioRxIntRadio(unsigned int origen,unsigned int *header,char *info)
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


