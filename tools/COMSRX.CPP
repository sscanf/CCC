
#include "d:defscgc.h"


extern unsigned int bufConfig[ACT_CH][ACT_OPER];

extern unsigned int consoles,canals;

extern unsigned int  comndAct;
extern ptrCmndInfo listCmnd,listCanals;
extern unsigned int lastCmnd; // apuntarÖ a l'£ltim element de la llista

extern ptrOperInfo listOpers;

unsigned int ackMstClk=0;
unsigned int numAckMstClk=0;


extern ptrRadioInfo listIndics;
extern unsigned int lastIndic;

extern int comHost;

void Tx2MasterClk(unsigned int comanda,unsigned int canal,unsigned int oper,unsigned int dades)
{
 /*
	 FORMAT MISSATGES:


	 0xec   Frame
	 0xa1

	 1 1 <Ch sortida, 4 bits> <comanda, 2 bits>

	 comanda : 00 Es pretän obtenir informaci¢ del canal associat al
					  Ch sortida, 40h=No hi ha connexi¢

				  10 Ordre de connexi¢ del Ch sortida amb el canal indicat en el
					  segÅent byte

				  01 Lectura de dades: subt¢, squelch
				  11 Transmissi¢ de dades: activa PTT, relÇs, ...

	 0 <ODE> 0 0 <Ch entrada, 4 bits>

	 ODE: 40h, ordre de desconnexi¢ del canal especificat en el byte anterior (Ch sortida).
			No es mira es Ch entrada.

 */

	unsigned int selec,chIn,chOut;

	disable();
	outp (rci2,0);              /*desactiva interrupciones 8259 */

	outp (rcm2,inp(rcm2) | 0x02);

	sacabyte(com2,0xec);
	sacabyte(com2,0xa1);

	switch (comanda)
	{
	  case CONFIG:
	  {
		 chIn=canal;
		 chOut=oper+0x0a;
		 chIn=chIn&0x0f;
		 chOut=chOut<<2;
		 chOut=chOut|0xc2; /* 11 chOut <10 comanda de config> */
		 sacabyte(com2,chOut);
		 sacabyte(com2,chIn);
		 break;
	  }
	  case INI_CONFIG:
	  {
		 selec=bufConfig[canal][oper];
		 iniConfigMstClk(selec,canal,oper);
		 break;
	  }
	  case RXESTAT:
	  /*
		  S'especifica el canal de sortida i el Master Clk ens donarÖ
		  informaci¢ del d'entrada.
	  */
		 chOut=canal;
		 chOut=chOut<<2;
		 chOut=chOut|0xc0; /* 11 chOut <00 comanda de lectura connexi¢> */
		 sacabyte(com2,chOut);
		 break;
	  case RXSUBTO:
		 chOut=canal;
		 chOut=chOut<<2;
		 chOut=chOut|0xc1; /* 11 chOut <01 comanda de lectura dades> */
		 sacabyte(com2,chOut);
		 break;
	  case TXDADES:
		 chOut=canal;
		 chOut=chOut<<2;
		 chOut=chOut|0xc3; /* 11 chOut <11 comanda de transmissi¢ de dades> */
		 sacabyte(com2,chOut);
		 if (dades==PTTON)	chIn=0x04; // bit 3='1'
		 else				   chIn=0x00; // bit 3='0'
		 sacabyte(com2,chIn);
		 break;
	  default:;
  }

  outp (rci2,1);              /*activa interrupciones 8259 */
  enable();

}

void iniConfigMstClk(unsigned int selec,unsigned int canal,unsigned int oper)
{
 unsigned int j=0,chIn,chOut,res=0,trobaRx=0,selec2,chTemp;

		 if (selec==1 || selec==3) /* Rx o Rx/Tx */
		 {
			chIn=canal;
			chOut=oper+0x0a;
		 }

		 if (selec==3) /*En cas Rx/Tx envia la configuraci¢ de Rx i el frame per la de Tx */
		 {
			chOut=chOut<<2;
			chOut=chOut|0xc2; /* 11 chOut <10 comanda de config> */
			chIn=chIn&0x0f;

			sacabyte(com2,chOut);
			sacabyte(com2,chIn);
			sacabyte(com2,0xec);
			sacabyte(com2,0xa1);
		 }


		 if (selec==2 || selec==3) /* Tx o Rx/Tx */
		 {
		  chIn=oper+0x0a;
		  chOut=canal;
		 }


		 if (selec==4) /* Ni Tx ni Rx */
		 {
			for (j=0;j<canals && trobaRx==0;j++)
			{
			 selec2=bufConfig[j][oper];
			 if (selec2==1 || selec2==3)
			 {
				chOut=canal; // es vol desconnectar el canal
				trobaRx=1;   // l'operador es troba com a sortida d'algun altre canal
			 }
			 else chOut=oper+0x0a; // desconnexi¢ operador si no escolta cap canal
			}

			for (j=0;j<consoles && res==0;j++)
			{
			 selec2=bufConfig[canal][j];
			 if ((selec2==2||selec2==3) && trobaRx==1) // es volia desconnectar canal perï estÖ
													// en Tx respecte un altre operador
			  res=1; 							// no fa res.
			}

			if (selec==4 && res==0)
				chIn=0x40;

			if (res==0 && trobaRx==0) // es tracta de res tant per canal com operador
			{                         // si trobaRx=0 chOut=oper, per tant,
											  // forcem el procÇs per canal
			 chTemp=canal;
			 chTemp=chTemp<<2;
			 chTemp=chTemp|0xc2;
			 sacabyte(com2,chTemp);
			 sacabyte(com2,0x40);
			 sacabyte(com2,0xec);
			 sacabyte(com2,0xa1);
			}

		 }

		 /* S'envia la configuraci¢ Rx, Tx o Tx en Rx/Tx */
		 if (res==0)
		 {
			 chOut=chOut<<2;
			 chOut=chOut|0xc2; /* 11 chOut <10 comanda de config> */
			 chIn=chIn&0x00ff;
			 sacabyte(com2,chOut);
			 sacabyte(com2,chIn);
		 }
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

void interrupt cojebuff1(...)
{
  static int ptrRx=0,lineStatAnt=0;
  static char bufRxHeader1[HEADER_VAX1+1]="";
  static char bufRxHeader2[HEADER_VAX2+1]="";
  static char bufRxData[MAX_BUFFER_VAX+1]="";
  unsigned int ch=0,FiRx=0,numBloques=0,length=0,trobat=0,i;
  static unsigned int numbytes=0,comanda=RES;
  int canalASI=-1,posicio=-1;



  outp (rci1,0x0);         /* desactiva interrupciones de la UART */
  // mirem la linea de DSR per veure si estÖ actiu

  ch=inp(LineStat1);

  if (((lineStatAnt^ch)&0x02)==0x02)
  { //  canvi d'estat DSR
	lineStatAnt=ch;
	if (comHost==OK) comHost=NOTOK;
	else comHost=OK;
  }
  else // int provocada per altres causes diferentes a canvi d'estat de DSR
  {
	  ch=inbyte(com1);

	  switch(ptrRx)
	  {
		 // tipus de missatge
		 case 0:if (ch=='A' || ch=='M' || ch=='N')
					  bufRxHeader1[ptrRx++]=ch;
				  else  ptrRx=0;
				  break;
		 case 1:if (ch=='S' || ch=='N' || ch=='O' || ch=='I')
					  bufRxHeader1[ptrRx++]=ch;
				  else ptrRx=0;
				  break;
		 case 2:
				  bufRxHeader1[ptrRx++]=ch;
				  bufRxHeader1[ptrRx]='\0';
				  if (strcmp(bufRxHeader1,"ASI")==0) //ASI
				  {
						  comanda=ASI;
						  numbytes=MAX_BUFFER_VAX-2;
				  }
				  else
					if (strcmp(bufRxHeader1,"ANU")==0) // ANU
					{
						  comanda=ANU;
						  numbytes=MAX_ANU;
					}
					else
						if (strcmp(bufRxHeader1,"MOD")==0) // MOD
						{
						  comanda=MOD;
						  numbytes=MAX_FITMOD;
						}
						else
						 if (strcmp(bufRxHeader1,"NIT")==0) // NIT INI o FIN
						 {
							comanda=NIT;
							numbytes=0;
						 }
						 else
							if (strcmp(bufRxHeader1,"ERR")==0) // ERRHOST
							{
							  comanda=ERRHOST;
							  numbytes=0;
							}
							else
							{
							  comanda=RES;
							  numbytes=0;
							  ptrRx=0;
							}

				  break;
		 case 3: // num de vehicle en ASCII
				  bufRxHeader2[ptrRx-HEADER_VAX1]=ch;
				  ptrRx++;
				  break;
		 case 4:
				  bufRxHeader2[ptrRx-HEADER_VAX1]=ch;
				  ptrRx++;
				  break;
		 case 5:
				  bufRxHeader2[ptrRx-HEADER_VAX1]=ch;
				  ptrRx++;
				  bufRxHeader2[ptrRx-HEADER_VAX1]='\0';
				  if (comanda==NIT || comanda==ERRHOST) FiRx=1;
				  break;

		 default:
				  if (((ptrRx-HEADER_VAX)<numbytes) && ((ptrRx-HEADER_VAX)<(MAX_BUFFER_VAX-2)))
				  {
						bufRxData[ptrRx-HEADER_VAX]=ch;
						ptrRx++;
				  }

				  if ((ptrRx-HEADER_VAX)>=numbytes || comanda==ERR)
				     FiRx=1;
				  break;
	  }/* del switch ptrRx*/


	  if (FiRx==1) // FI RECEPCIO
	  {
		 canalASI=canalAssociat(atoi(bufRxHeader2));

		 // comprova que indicatiu donat d'alta sino error al VAX
		 if ((comanda!=RES && canalASI!=-1) ||  comanda==ERRHOST || comanda==NIT)
		 {
			// indicatiu donat d'alta
			// en origen tindrem el num d'indicatiu al que va dirigit
			listCmnd[lastCmnd].comnd=comanda;
			listCmnd[lastCmnd].origen=atoi(bufRxHeader2);
			listCmnd[lastCmnd].proces=0;
			if (comanda!=ASI && comanda!=NIT)
			 strcpy(listCmnd[lastCmnd].info,bufRxData);
			else
			 if (comanda==NIT)
			  strcpy(listCmnd[lastCmnd].info,bufRxHeader2);
			 else
			 {
			  strcpy(listCmnd[lastCmnd].info,"");
			  posicio=posicioIndicatiu(listCmnd[lastCmnd].origen);
			  strcpy(listIndics[posicio].fitxer,bufRxData);
			 }
			listCmnd[lastCmnd].estat=NOTRUN;

		 }
		 else
		 {
			if (comanda==ASI || comanda==MOD || comanda==ANU)
			{
			  listCmnd[lastCmnd].comnd=ERR;
			  listCmnd[lastCmnd].proces=comanda; // funci¢ en la que hi ha l'error
			  listCmnd[lastCmnd].origen=atoi(bufRxHeader2);// indicatiu
			  listCmnd[lastCmnd].estat=NOTRUN;
			  getExpServei(bufRxData);
			}
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

		 /* inicialitzacio per recepci¢*/
		 ptrRx=0;
		 strcpy(bufRxHeader1,"");
		 strcpy(bufRxHeader2,"");
		 strcpy(bufRxData,"");
		 comanda=RES;
		 numbytes=0;
	  }
  }

  outp (rcm1, tdl | lpe | out2);
  outp (cop2,0x20);   /* Indica Fin de interrupt */
  outp (rci1,0x09);      /* ACTIVA INTERRUPCIONES UART */
								 // Modem Line Status i recepci¢
}



void getExpServei(char *tempo)
{
 unsigned int i,j,k;
 char expedient[8]="",servei[3]="";

 for(i=0;i<7 && tempo[i]!='\n';i++)
	expedient[i]=tempo[i];
 expedient[i]='\0';

 if (i!=7)
 {
	 strcpy(expedient,ompleZeros(expedient,7));
	 i++;
 }

 strcpy(listCmnd[lastCmnd].info,expedient);


 for(j=i,k=0;k<2 && tempo[j]!='\n';j++,k++)
	 servei[k]=tempo[j];

 servei[k]='\0';

 if (k!=2) strcpy(servei,ompleZeros(servei,2));

 strcat(listCmnd[lastCmnd].info,servei);
}


void interrupt cojebuff2(...)
{
  unsigned int ch;
  static int ptrRx=-2;

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
		listCmnd[comndAct].proces=ch;
		listCmnd[comndAct].estat=FIESPERA;
		ptrRx=-2;
		break;
  }// del switch

  outp (rcm2, tdl | lpe | out2);
  outp (cop2,0x20);  /* Indica Fin de interrupt */
  outp (rci2,1);      /*ACTIVA INTERRUPCIONES UART */
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


void Tx2VAX(unsigned int comanda,unsigned int indicatiu,unsigned int numbytes,char *missatge)
{
 char dest[8]="",dest2[8],expedient[8],numServei[8];
 unsigned int i=0,j=0,trobat=0;

 disable();
 outp (rci2,0);              /*desactiva interrupciones 8259 */

 outp (rcm2,inp(rcm2) | 0x02);

 switch(comanda)
 {
  case FIN:strcpy(dest,"FIN");break;
  case DES:strcpy(dest,"DES");break;
  case OCU:strcpy(dest,"REP");break;
  case PAR:strcpy(dest,"PAR");break;
  case TAN:strcpy(dest,"TAN");break;
  case OBE:strcpy(dest,"OBE");break;
  case LLI:strcpy(dest,"FIN");break;
  case ERR:strcpy(dest,"ERR");break;
  case NIT:strcpy(dest,"NIT");break;
  default:;
 }

 for(i=0;i<3;i++)
  sacabyte(com1,dest[i]); // comanda en ASCII Ex. OBE---> 'O' 'B' 'E'


 switch(comanda)
 {
  case FIN:
  case DES:
  case OCU:
  case PAR:
  case TAN:
  case OBE:
  case LLI:
			  itoa(indicatiu,dest2,10);
			  strcpy(dest2,ompleZeros(dest2,3));

			  for(i=0;i<3;i++)
				  sacabyte(com1,dest2[i]);

			  for(i=0;i<numbytes;i++)
				sacabyte(com1,missatge[i]);
			  break;
  case ERR:
				 switch(listCmnd[comndAct].proces)
				 {
				  case ASI:strcpy(dest,"ASI");break;
				  case MOD:strcpy(dest,"MOD");break;
				  case ANU:strcpy(dest,"ANU");break;
				  default:;
				 }
				 for(i=0;i<3;i++)
				  sacabyte(com1,dest[i]); // comanda en ASCII Ex. OBE---> 'O' 'B' 'E'

				 itoa(indicatiu,dest2,10);
				 strcpy(dest2,ompleZeros(dest2,3));

				 for(i=0;i<3;i++)
					sacabyte(com1,dest2[i]);

				 for(i=0;i<numbytes;i++)
					sacabyte(com1,missatge[i]);

			 break;
  default:break;
 }


 outp(rci2,1);              /*activa interrupciones 8259 */
 enable();

}


void interrupt newTimer(...)
{
 unsigned int i;
 for (i=0;i<consoles;i++)
 {
  if (listOpers[i].estat==ESPERA && listOpers[i].compta<20 && listOpers[i].ctrlPTT==PTTOFF)
	 listOpers[i].compta++;
  else
  {
	  listOpers[i].estat=FIESPERA;
	  listOpers[i].compta=0;
  }
 }

}
