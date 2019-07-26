/*void Tx2MasterClk(unsigned int comanda,unsigned int canal,unsigned int oper,unsigned int dades)
{
 //
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



	unsigned int selec,chIn,chOut;

	disable();
	outp (rci2,0);              desactiva interrupciones 8259

	outp (rcm2,inp(rcm2) | 0x02);

	sacabyte(com2,0xec);
	sacabyte(com2,0xa1);

	switch (comanda)
	{
	  case CONFIG:
	  {
		 chIn=canal;
		 chOut=oper;

		 chIn=chIn&0x4f;
		 chOut=chOut<<2;
		 chOut=chOut|0xc2; /* 11 chOut <10 comanda de config> */
		 sacabyte(com2,chOut);
		 sacabyte(com2,chIn);
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
		  S'especifica el canal de sortida i el Master Clk ens donarÖ
		  informaci¢ del d'entrada.
		  La variable canal, en aquest cas, pot referir-se a operador o canal de rÖdio.
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
		 else				      chIn=0x00; // bit 3='0'
		 sacabyte(com2,chIn);
		 break;
	  default:;
  }

  outp (rci2,1);              /*activa interrupciones 8259 */
  enable();
}

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

//	ViejIntTIMER=getvect (0x1c);   /* Obtiene vector int. antigua */
//	setvect(0x1c,newTimer); 		/* Redefine vector int.Nueva */


  outp (rcm1, tdl | lpe | out2);
  outp (rcm2, tdl | lpe | out2);

  byte = inp (cop1);
  byte &= irq3;
  outp (cop1, byte);

  outp (rci1,0x09);		 // Activa interrupciones: modem i recepci¢
  outp (rci2,0x01);		 /* Activa interrupciones */





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

//	ViejIntTIMER=getvect (0x1c);   /* Obtiene vector int. antigua */
//	setvect(0x1c,newTimer); 		/* Redefine vector int.Nueva */


  outp (rcm1, tdl | lpe | out2);
  outp (rcm2, tdl | lpe | out2);

  byte = inp (cop1);
  byte &= irq3;
  outp (cop1, byte);

  outp (rci1,0x09);		 // Activa interrupciones: modem i recepci¢
  outp (rci2,0x01);		 /* Activa interrupciones */

void Tx2VAX(unsigned int comanda,unsigned int indicatiu,unsigned int numbytes,char *missatge)
{
 char dest[8]="",dest2[8],expedient[8],numServei[8];
 unsigned int i=0,j=0,trobat=0;


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
				 switch(listExec[0].proces)
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

void iniConfigMstClk(unsigned int selec,unsigned int canal,unsigned int oper)
{
 unsigned int j=0,chIn,chOut,res=0,trobaRx=0,selec2,chTemp;

		 if (selec==1 || selec==3) /* Rx o Rx/Tx */
		 {
			chIn=canal;
			chOut=oper;
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
		  chIn=oper;
		  chOut=canal;
		 }


		 if (selec==4) /* Ni Tx ni Rx */
		 {
			for (j=0;j<canals && trobaRx==0;j++)
			{
			 selec2=bufConfig[j][oper-0x0a];
			 if (selec2==1 || selec2==3)
			 {
				chOut=canal; // es vol desconnectar el canal
				trobaRx=1;   // l'operador es troba com a sortida d'algun altre canal
			 }
			 else chOut=oper; // desconnexi¢ operador si no escolta cap canal
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

		 if (lastCmnd<MAX_NUM_COMND) lastCmnd++;
		 else lastCmnd=0;

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

void interrupt IntPort2(void)
{
/* El missatge que s'espera rebre Çs: 0xec 0xa1 numProc comanda numbytes <info de numbytes de llargÖria>*/
	int carac;
	unsigned int k,q,j,i;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_OPER];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;

	carac=sio_getch(PORT2);
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
		 gestRxIntOpers(PORT2,bufRxHeader,bufRxData);

		 /* Inicialitza els parÖmetres de recepci¢*/

		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_OPER);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT2,0);
	}// tanca if FiRx
  }//tanca carac
}

void interrupt cojebuff2(...)
{
  unsigned int ch=0;
  static int ptrRx=-2;

  outp (rci2,0x0);

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
		listExec[0].proces=ch;
		listExec[0].estat=FIESPERA;
		ptrRx=-2;
		break;
  }// del switch

  outp (rcm2, tdl | lpe | out2);
  outp (cop2,0x20);  /* Indica Fin de interrupt */
  outp (rci2,0x01);      /*ACTIVA INTERRUPCIONES UART */
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


void gestioTotPTT(void)
{
// Funci¢ que gestiona PTT.
  unsigned int i,permis=4,ctrlPTT=0;

  ctrlPTT=listExec[0].proces;
  // ctrlPTT valdrÖ 2 o 3 en cas de trucada de  grup per indica PTTOFF o PTTON
  // ctrlPTT valdrÖ 0 o 1 en comanda de PTT normal.
  if (ctrlPTT==3 || ctrlPTT==2 || obrir==CODIS || (strcmp(listExec[0].info,"")))
  // per codis o
  // trucada de grup o indicatiu amb sistema obert, funciona igual que per codis
	gestioPTTCodis(ctrlPTT);
  else
  // obert per silenciador i sense indicatiu, transmissi¢ per tots els canals Tx de l'operador
  {
	 for(i=0;i<ACT_CH;i++)
	 {
		  permis=bufConfig[i][listExec[0].origen];
		  if (permis==2 || permis==3)
		  {
			if ((listCanals[i].comnd==SQLON || canalOcupat(i)==NOTOK) && ctrlPTT==1)
			// Si canal ocupat per fitxer li d¢na prioritat al fitxer
			// Es a dir, connexi¢ si canal lliure o ocupat per Sql.
			{
					// demanem a MasterClk estat actual de la conexi¢
					// establim la connexi¢ si no estÖ feta
					listExec[0].estat=ESPERA;
					Tx2MasterClk(RXESTAT,i,0,0);
					tascaEspera();

					if (listExec[0].estat==FIESPERA)
					{
					  if (listExec[0].proces!=listExec[0].origen+0x0a)
					  /*
							No hi ha connexi¢ o operador es troba en un altre canal.
							Per tant, reprogramem Master Clk.
					  */
						 Tx2MasterClk(CONFIG,listExec[0].origen+0x0a,i,0); // entrada,sortida


					  // un cop transmÇs conecto PTT i desconecto operador com a sortida.
					  Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
					  conectaPTT(i);
					}
			}
			else
				// desconecto PTT  si el canal es troba connectat per PTT
				if (ctrlPTT==0 && (listCanals[i].comnd==PTT || listCanals[i].comnd==SQLON))
					 desconectaPTT(i);
		  } // de permis
	} // de for Tx
 } // de silenciador
 listExec[0].estat=FIEXEC;
}
void interrupt IntPort7(void)
{
// El missatge que s'espera rebre Çs: 0xec 0xa1 flota indicatiu<2 bytes> comanda numbytes <info de numbytes de llargÖria>
	int carac;
	unsigned int FiRx=0;
	static unsigned int bufRxHeader[HEADER_RADIO];
	static char bufRxData[MAX_BUFFER_RX]="";
	static int ptrRx=-2;
	static unsigned int numbytes=0;
	static unsigned int flag=0;

	carac=sio_getch(PORT7+3);
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
		 case -2: // FRAME es 0xec 0xa1
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
	  }// del switch ptrRX


	  if (FiRx==1) // per probar els indicatius
	  {
		 gestioRxIntRadio(PORT7,bufRxHeader,bufRxData);

		 // Inicialitza els parÖmetres de recepci¢
		 ptrRx=-2;
		 numbytes=0;carac=0;
		 memset(bufRxHeader,0,HEADER_RADIO);
		 memset(bufRxData,'\0',MAX_BUFFER_RX);
		 sio_flush(PORT7+3,0);

	  }// tanca if FiRx
	}//tanca carac
}
void Tx2Radio(unsigned int canal,unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
// En els ports 3,4,5,6,... s¢n de canals de rÖdio.
// Segons la variable canal, la comanda anirÖ dirigida a un canal o altre.
// La variable header controla la transmissi¢ de EC,A1,COMND,...
// No cal enviar-ho quan es tracta de blocs ASI o MOD.
//
 int stat=NOTOK,i=0;

// Quan enviem un missatge per rÖdio, primer tallarem la veu als operadors que es
// troben escoltant el canal; enviarem el missatge i desprÇs els
// tornarem a connectar.

 for(i=0;i<consoles;i++)
 {
  Tx2MasterClk(RXESTAT,i+0x0a,0,0);
  tascaEspera();

  if (listExec[0].estat==FIESPERA && listExec[0].proces==canal)
  // oper es troba connectat al canal
  {
	listOpers[i].canalAss=canal;
	listOpers[i].TxRadio=1;
	Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
  }
 }

 if (comanda==ANU) comanda=0x09;
 do{
  stat=Tx2PortR(canal+3,comanda,indicatiu,numbytes,missatge,header);
 }while (stat==NOTOK);

 for(i=0;i<consoles;i++)
 {
  if (listOpers[i].canalAss==canal && listOpers[i].TxRadio==1)
  {
	listOpers[i].TxRadio=0;
	Tx2MasterClk(CONFIG,listExec[0].origen+0x0a,canal,0);
  }
 }

}
