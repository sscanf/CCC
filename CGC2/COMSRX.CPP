              
#include "d:defscgc.h"


extern unsigned int bufConfig[ACT_CH][ACT_OPER];

extern unsigned int consoles,canals;

extern unsigned int  comndAct;
extern ptrCmndInfo listCmnd,listCanals,listExec;
extern unsigned int lastCmnd; // apuntar… a l'£ltim element de la llista

extern ptrOperInfo listOpers;

unsigned int ackMstClk=0;
unsigned int numAckMstClk=0;


extern ptrRadioInfo listIndics;
extern unsigned int lastIndic;

extern int comHost;


void Tx2MasterClk(unsigned int comanda,unsigned int canal,unsigned int oper,
						unsigned int dades)
{

  int txData,freeSpace,tempo=0,flota=0,posicio=-1;
  unsigned int numbytes,chIn,chOut,selec=4;
  char dest[4]="";


  sio_enableTx(PORT7+3);
  txData=sio_oqueue(PORT7+3);
  freeSpace=sio_ofree(PORT7+3);
  if (txData>=0 && freeSpace>=7)
  {
		sio_putch(PORT7+3,0xec);
		sio_putch(PORT7+3,0xa1);
		switch (comanda)
		{
		 case CONFIG:
		 {
		 chIn=canal;
		 chOut=oper;

		 chIn=chIn&0x4f;
		 chOut=chOut<<2;
		 chOut=chOut|0xc2; /* 11 chOut <10 comanda de config> */
//		 printf("CONFIG ChOut %x\n",chOut);
//		 printf("ChIn %x\n",chIn);

		 sio_putch(PORT7+3,chOut);
		 sio_putch(PORT7+3,chIn);
		 break;
		 }
		 case INI_CONFIG:
		 {
		 selec=bufConfig[canal][oper-0x0a];
		 iniConfigMstClk(selec,canal,oper);
		 break;
		 }
		 case RXESTAT:
	  /*
		  S'especifica el canal de sortida i el Master Clk ens donar…
		  informaci¢ del d'entrada.
		  La variable canal, en aquest cas, pot referir-se a operador o canal de r…dio.
	  */
		 chOut=canal;
		 chOut=chOut<<2;
		 chOut=chOut|0xc0; /* 11 chOut <00 comanda de lectura connexi¢> */
//		 printf("ESTAT ChOut %x\n",chOut);
		 sio_putch(PORT7+3,chOut);
		 break;
	  case RXSUBTO:
		 chOut=canal;
		 chOut=chOut<<2;
		 chOut=chOut|0xc1; /* 11 chOut <01 comanda de lectura dades> */
//		 printf("SUBTO chOut %x\n",chOut);
		 sio_putch(PORT7+3,chOut);
		 break;
	  case TXDADES:
		 chOut=canal;
		 chOut=chOut<<2;
		 chOut=chOut|0xc3; /* 11 chOut <11 comanda de transmissi¢ de dades> */
		 sio_putch(PORT7+3,chOut);
		 if (dades==PTTON)	chIn=0x04; // bit 3='1'
		 else				      chIn=0x00; // bit 3='0'
		 sio_putch(PORT7+3,chIn);
		 break;
	  default:;
  }
 }
}


void iniConfigMstClk(unsigned int selec,unsigned int canal,unsigned int oper)
{
}



void interrupt IntPort2()
{
  static int ptrRx=0,lineStatAnt=0;
  static char bufRxHeader1[HEADER_VAX1+1]="";
  static char bufRxHeader2[HEADER_VAX2+1]="";
  static char bufRxData[MAX_BUFFER_VAX+1]="";
  unsigned int ch=0,FiRx=0,numBloques=0,length=0,trobat=0,i;
  static unsigned int numbytes=0,comanda=RES;
  int canalASI=-1,posicio=-1;


  ch=sio_getch(PORT2);
  if (ch>=0)
  {

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

				  if ((ptrRx-HEADER_VAX)>=numbytes)
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
			if (comanda==NIT)
			  strcpy(bufRxData,bufRxHeader2);
			else
			  if (comanda==ASI)
			  {
				posicio=posicioIndicatiu(atoi(bufRxHeader2));
				strcpy(listIndics[posicio].fitxer,bufRxData);
				strcpy(bufRxData,"");
			  }
			posaTasca(0,bufRxData,comanda,0,atoi(bufRxHeader2),NOTRUN);
		 }
		 else
		 {
			if (comanda==ASI || comanda==MOD || comanda==ANU)
			{
			  bufRxData[9]='\0';
			  posaTasca(0,bufRxData,ERR,comanda,atoi(bufRxHeader2),NOTRUN);

			}
		 }

		 /* inicialitzacio per recepci¢*/
		 ptrRx=0;
		 memset(bufRxHeader1,'\0',HEADER_VAX1+1);
		 memset(bufRxHeader2,'\0',HEADER_VAX2+1);
		 memset(bufRxData,'\0',MAX_BUFFER_VAX+1);
		 comanda=RES;
		 numbytes=0;
		 FiRx=0;
		 sio_flush(PORT2,0);
	  }
  }
}



void getExpServei(char *tempo)
{
 unsigned int i,j,k;
 char expedient[8]="",servei[3]="";
 char tempo2[MAX_BUFFER_VAX+4]="";

 strcpy(tempo2,tempo);

 for(i=0;i<7 && tempo2[i]!='\n';i++)
	expedient[i]=tempo2[i];
 expedient[i]='\0';

 if (i!=7)
 {
	 strcpy(expedient,ompleZeros(expedient,7));
	 i++;
 }

 strcpy(tempo,expedient);


 for(j=i,k=0;k<2 && tempo2[j]!='\n';j++,k++)
	 servei[k]=tempo2[j];

 servei[k]='\0';

 if (k!=2) strcpy(servei,ompleZeros(servei,2));

 strcat(tempo,servei);
 tempo[strlen(tempo)]='\0';
}




void Tx2VAX(unsigned int comanda,unsigned int indicatiu,unsigned int numbytes,char *missatge)
{
 char dest[8]="",dest2[8],expedient[8],numServei[8];
 unsigned int i=0,j=0,trobat=0;
 int txData,freeSpace;

 sio_enableTx(PORT7+3);

 txData=sio_oqueue(PORT2);
 freeSpace=sio_ofree(PORT2);
 if (txData>=0 && freeSpace>=numbytes+12)
 {
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

	 sio_write(PORT2,dest,strlen(dest));
	 printf(dest);


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
			  sio_write(PORT2,dest2,strlen(dest2));
			  if (numbytes!=0) sio_write(PORT2,missatge,numbytes);
			  break;
	  case ERR:
				 switch(listExec[0].proces)
				 {
				  case ASI:strcpy(dest,"ASI");break;
				  case MOD:strcpy(dest,"MOD");break;
				  case ANU:strcpy(dest,"ANU");break;
				  default:strcpy(dest,"");
				 }
				 sio_write(PORT2,dest,strlen(dest));
				 printf("%s",dest);

				 itoa(indicatiu,dest2,10);
				 strcpy(dest2,ompleZeros(dest2,3));
				 sio_write(PORT2,dest2,strlen(dest2));
				 printf(dest2);
				 sio_write(PORT2,missatge,numbytes);
				 printf(missatge);
			 break;
		default:break;
  }
 }
}


