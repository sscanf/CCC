#include "c:defspan.h"

union REGS inregs, outregs;
union	REGS ent,sal;

void interrupt cojebuff2(...);
void interrupt cojebuff1(...);


void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/
void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/


unsigned int bufRxHeader[HEADER];
char bufRxDades[MAX_BUFFER_RX];

unsigned int inbyte();

unsigned int   BufTx[ACT_CH][ACT_OPER];			/* Buffer transmisi¢n*/
extern ptrCGCData ConfigCGCBuf[MAX_CH_BUF][MAX_OPER_BUF];
extern unsigned int nTChPan,nTOperPan;
extern canals,consoles;
extern ptrRxInfo listRxCmnd;
extern unsigned int lastCmnd,comndAct;

extern ptrFinVeuData veuData;
extern unsigned int CGCOn,RxEstat;
extern ptrObrirData obrirData;
extern ptrTrucCanalData trucCanalData;



/*--------------------- TRANSMISION ----------------------------------*/

void TMyApp::Tx2CGC(unsigned int comanda,unsigned int nbytes,unsigned int numProc,char *tempo)
{
	unsigned int i,j,tempo2=0;
	ushort byte;


	disable();
	outp (rci2,0);              /*desactiva interrupciones 8259 */

	outp (rcm2,inp(rcm2) | 0x02); // RTS=1 DSR=0

	 sacabyte(0xec);
	 sacabyte(0xa1);
	 sacabyte(comanda);
	 tempo2=numProc&0x00ff;  // numProc nom‚s t‚ sentit en Incid o SOS
									 // assignades pel CGC a l'operador.

	 sacabyte(tempo2);      // num de proces low
	 sacabyte(numProc>>8); // num de proces high
	 sacabyte(nbytes);

	 switch(comanda)
	 {
	  case cmGrvConfig:
			for (i=0;(i<consoles);i++)
				for(j=0;(j<canals);j++)
				{
					byte=BufTx[j][i];
					sacabyte(byte);
				}
			break;
	  default:
			for(i=0;i<nbytes;i++)
			  sacabyte(tempo[i]);
			break;
	 }

	outp (rci2,0x09);              /*activa interrupciones 8259 */
	enable();
}


void TListBox5::Tx2CGC(unsigned int comanda,unsigned int nbytes,unsigned int ctrlPTT,char *tempo)
{
	unsigned int i,j;
	ushort byte;


	disable();
	outp (rci2,0);              /*desactiva interrupciones 8259 */

	outp (rcm2,inp(rcm2) | 0x02); // RTS=1 DSR=0

	 sacabyte(0xec);
	 sacabyte(0xa1);
	 sacabyte(comanda);
	 sacabyte(0);
	 sacabyte(ctrlPTT); // estat PTT
	 sacabyte(nbytes);
	 for(i=0;i<nbytes;i++)
			  sacabyte(tempo[i]);

	outp (rci2,0x09);              /*activa interrupciones 8259 */
	enable();
}



void TMyApp::sacabyte(unsigned int byte)
{
	unsigned char UartLibre=0;

	do{
		UartLibre=inportb (relin2) & 0x60;
	}while (UartLibre!=0x60);
	outp (com2,byte);
}

void TListBox5::sacabyte(unsigned int byte)
{
	unsigned char UartLibre=0;

	do{
		UartLibre=inportb (relin2) & 0x60;
	}while (UartLibre!=0x60);
	outp (com2,byte);
}



void TMyApp::ompleConfigTx(unsigned int numchk,unsigned int i,unsigned int j,ushort dada,
	 unsigned int topeX,unsigned int topeY,unsigned int inici)
{
 unsigned int posY,reste,nodada=0,tope=0,menor=0;
 static int posX=-1,incY=0,incX=0;

 if (inici==1) {posX=-1;incY=0;incX=0;}
 numchk--;
 posY=(unsigned int)(numchk/topeX);
 posX++;
 reste=posX%topeX;
 if (reste==0) posX=0;

 incX++;
 incY++;
 if (canals<6 && consoles<4)
 {
  menor=1;
  tope=topeX*topeY;
 }
 if (((incY>(4*topeX)) && topeX!=6) || ((incX>(6*topeY)) && topeY!=4) || (menor==1 && incX>tope)) nodada=1;
 else nodada=0;

 if (nodada==0) BufTx[(j*6)+posX][(i*4)+posY]=dada;
}

unsigned int TMyApp::ultimCasX(unsigned int x)
{
 unsigned int incCh=6;
 unsigned int tempo,tempo2;

 if(x==(nTChPan-1))
 {
  tempo=canals%incCh;
  if (tempo!=0)
  {
	tempo2=canals-(((unsigned int)(canals/incCh))*incCh);
   return tempo2;
  }
 }
 else return incCh;
}

unsigned int TMyApp::ultimCasY(unsigned int y)
{
 unsigned int incOper=4;
 unsigned int tempo,tempo2;

 if(y==(nTOperPan-1))
 {
  tempo=consoles%incOper;
  if (tempo!=0)
  {
   tempo2=consoles-(((unsigned int)(consoles/incOper))*incOper);
	return tempo2;
  }
 }
 else return incOper;
}

void TMyApp::convert2Tx()
{
 unsigned int i,j,topeX,topeY;
 ushort dada;

 for(i=0;i<nTOperPan;i++)  // index Y en matriu configuraci¢ pantalla
 {
  topeY=ultimCasY(i);
  for(j=0;j<nTChPan;j++)   // index X en matriu configuraci¢ pantalla
  {
    topeX=ultimCasX(j);
	 dada=ConfigCGCBuf[j][i]->chk1;
    ompleConfigTx(1,i,j,dada,topeX,topeY,1);
    dada=ConfigCGCBuf[j][i]->chk2;
    ompleConfigTx(2,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk3;
    ompleConfigTx(3,i,j,dada,topeX,topeY,0);
    dada=ConfigCGCBuf[j][i]->chk4;
    ompleConfigTx(4,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk5;
    ompleConfigTx(5,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk6;
	 ompleConfigTx(6,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk7;
	 ompleConfigTx(7,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk8;
    ompleConfigTx(8,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk9;
    ompleConfigTx(9,i,j,dada,topeX,topeY,0);
    dada=ConfigCGCBuf[j][i]->chk10;
    ompleConfigTx(10,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk11;
    ompleConfigTx(11,i,j,dada,topeX,topeY,0);
    dada=ConfigCGCBuf[j][i]->chk12;
    ompleConfigTx(12,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk13;
	 ompleConfigTx(13,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk14;
	 ompleConfigTx(14,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk15;
	 ompleConfigTx(15,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk16;
	 ompleConfigTx(16,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk17;
	 ompleConfigTx(17,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk18;
	 ompleConfigTx(18,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk19;
	 ompleConfigTx(19,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk20;
	 ompleConfigTx(20,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk21;
	 ompleConfigTx(21,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk22;
	 ompleConfigTx(22,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk23;
	 ompleConfigTx(23,i,j,dada,topeX,topeY,0);
	 dada=ConfigCGCBuf[j][i]->chk24;
	 ompleConfigTx(24,i,j,dada,topeX,topeY,0);
  }
 }
}

void TMyApp::initTx(void)
{

		  ushort byte;

		  ent.x.ax = 0xe3;        /* 9600,n,8,1*/
		  ent.x.dx = 1;           /* com2 */
		  int86(0x14,&ent,&sal);  /* total (OPEN "COM2:9600,N,8,1") */


		  disable();
		  outp (rci2,0x00);		 /* Desactiva interrupciones */

		  outp (rcm2,0x03);      /* RTS=1 i DSR=1 */

		  ViejIntCOM2=_dos_getvect (11);   /* Obtiene vector int. antigua */
		  _dos_setvect(11,cojebuff2); 		/* Redefine vector int.Nueva */


		  memset(bufRxDades,'\0',MAX_BUFFER_RX);
		  memset(bufRxHeader,0,HEADER);

		  outp (rcm2, tdl | lpe | out2);

		  byte = inp (cop1);
		  byte &= irq4;
		  outp (cop1, byte);

}

/*----------------------------- RECEPCION -----------------------------*/

/*------------------------Rutina Interrupci¢n--------------------------*/


void interrupt cojebuff2(...)
{

  static unsigned int numbytes;
  unsigned int FiRx=0,i,tempo=0;
  unsigned int long time=0;
  static long ptrRx=-2;
  static unsigned int ctrlPTT=0;
  ushort ch,stat=0,stat1=0,n;

  disable();
  outp(rci2,0x0);         /* desactiva interrupciones de la UART */
  outp(rcm2,inp(rcm2) | 0x01);      /* DSR=1 oper.ocupat  */


  stat=inp(LineStat2);

  if ((stat&0x01)==0x01)
  {
	 delay(4);
	 for (;time<80;time++) // 250
	 {
		  stat=inp(LineStat2);
//		  delay(1);
			for (n=0;n<50;n++);
		  stat1=inp(LineStat2);
		  if ((stat&0x10)!=stat1)  time=0;
	 }

	 stat=inp(LineStat2);

	 if ((stat&0x10)==0x10) // miramos el estado de CTS
		 ctrlPTT=1;
	 else
		 ctrlPTT=0;

	 trucCanalData->status = ctrlPTT;

	 listRxCmnd[lastCmnd].comnd=cmPTT;
	 listRxCmnd[lastCmnd].numProc=ctrlPTT;
	 strcpy(listRxCmnd[lastCmnd].info,"");

	 if (lastCmnd<(MAX_NUM_COMND-1))
	 {
		lastCmnd++;
		listRxCmnd[lastCmnd].comnd=cmRES;
		listRxCmnd[lastCmnd].numProc=0;
		memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
	 }
  }
  else
  {
	  ch=inbyte();
	  if (ch==0xec && ptrRx>3) ptrRx=-2;

	  switch (ptrRx)
	  {
		/* FRAME: 0xec 0xa1 */
		case -2: if (ch==0xec)
						ptrRx++;
					break;
		case -1: if (ch=0xa1)
					  ptrRx++;
					else ptrRx=-2;
					break;
		case 0:  // agafem el header
					// comanda
					bufRxHeader[ptrRx++]=ch;
					if (ch==236) ptrRx=-2;
					break;
		case 1:
		case 2:  bufRxHeader[ptrRx++]=ch; // num de proces per MsServeis o MsIncid
					break;
		case 3:  // numbytes
					numbytes=ch;
					if (numbytes==0) FiRx=1;
					ptrRx++;
					break;
		default:
				if (ptrRx<=(numbytes+HEADER) && ptrRx<(HEADER+MAX_BUFFER_RX))
				{       if (ch==0) ch='0';
					bufRxDades[ptrRx-HEADER]=ch;
				}

				ptrRx++;
				if (ptrRx>=(numbytes+HEADER))
				  FiRx=1;
	  }

	  if (FiRx==1)
	  {
		 tempo=bufRxHeader[2]<<8;
		 tempo=tempo|bufRxHeader[1];
		 if (bufRxHeader[0]==cmTst)
		 {
		  CGCOn=ON;
		  if (tempo==1) (*obrirData).info=1;  // per codi
		  else (*obrirData).info=0; // per silenciador
		  bufRxHeader[0]=cmCanviObrir;
		 }

		 if (bufRxHeader[0]==cmEstat)
		 {
		  RxEstat=1;
		  CGCOn=ON;
		 }

		 listRxCmnd[lastCmnd].comnd=bufRxHeader[0];
		 listRxCmnd[lastCmnd].numProc=tempo;
		 if (bufRxDades!='\0')
			strcpy(listRxCmnd[lastCmnd].info,bufRxDades);
		 else
			strcpy(listRxCmnd[lastCmnd].info,"");

		 if (lastCmnd<(MAX_NUM_COMND-1))
		 {
			lastCmnd++;
			listRxCmnd[lastCmnd].comnd=cmRES;
			listRxCmnd[lastCmnd].numProc=0;
			memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
		 }
		 ptrRx=-2;
		 numbytes=0;

		 memset(bufRxDades,'\0',MAX_BUFFER_RX);
		 FiRx=0;
	 }
  }

  outp (rcm2, tdl | lpe | out2);
  outp(rcm2,inp(rcm2) & 0xfe);      /* DSR=0 operador lliure  */
  outp (cop2,0x20);  /* Indica Fin de interrupt */
  outp (rci2,0x09);      /*ACTIVA INTERRUPCIONES UART */
  enable();

}



unsigned int inbyte()
{
	unsigned char tmp=0;
	unsigned long int timeout=0;
	unsigned int bt;

	while (tmp==0 && timeout<10000)
	{
		tmp = inportb (rcmm2);
		tmp &= 0x01;
		timeout++;
	}
	bt=inp (com2);
	return(bt);
}

