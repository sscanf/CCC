
// l'indicatiu de la central ‚s <1111 1111 1111 1> <13 uns>

#include "defscgc.h"
#include <signal.h>

int putejar=0;
char **listError;
unsigned int consoles=ACT_OPER,canals=ACT_CH;

unsigned int  comndAct;
ptrCmndInfo listCmnd;
ptrCmndInfo listExec;

unsigned int lastExec,lastCmnd; // apuntar… a la £ltima comanda
ptrCmndInfo listCanals;

ptrRadioInfo listIndics;
unsigned int lastIndic=0;

extern  void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/
extern void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/
//extern void interrupt (*ViejIntTIMER)(...);   /*vieja interrupci¢n Timer:*/

void ListIndics (void);
//void creaTaulaMsg(void);
void ListColas (void);
void ErrorCritico(int);

char prueba[512];
int TablaPredefs[7] = {6482,6504,6526,6562,6584,3202,0};

ptrOperInfo listOpers;

//ptrMsgInfo taulaMsg;
int RxFich=0;
char RxDest[MAX_BLOQUE+1];
int DebugMode=0;

int BrkOn=0;

char BuffMsg[1000];
int ptrBuff=0;
unsigned int llegeixCmnd(void)
{
 return (listExec[0].comnd);
}


void tstComndAct(void)
{
 if (listExec[0].estat==FIEXEC)
	correAmunt();
}


void execsys(void)
{
 switch (llegeixCmnd())
 {
  case RES:
									gestioCarrega();
									break;
  case ERR:
									gestioErr();
									break;

  case ERRHOST: 				gestioErrHost();
									break;
  case INI_CONFIG:			gestioIniConfig();
									break;
  case PETVEUE:
  case PETVEUS:
									gestioPetVeu();
									break;

  case TRUCINDIV: 			gestioTrucIndiv();
									break;
  case TRUC_SOCI:
  case MSEMER:
  case MSSERVEIS:
  case VEU:
									gestioTxOpers();
									break;
  case SQLON:
									gestioSql();
									break;
  case SQLOFF:
									gestioSqlOff();
									break;
  case DES:
  case OCU:
  case PAR:
  case OBE:
  case LLI:
  case FIN:
  case TAN:
									gestioVAX();
									break;
  case ANU:
  case ASI:
  case PROGINDIC:
  case MOD:
									gestioTx2Mobil();
									break;

  case TRUCSOCI:
									gestioTrucSoci();
									break;
  case ANULVEU:
									gestioAnulVeu();
									break;
  case NIT:						gestioNit();
									break;
  case ESTATPTT:
									gestioTotPTT();
									break;
  case OBRIR:
									gestioObrir();
									break;

  case TRUCGRUP:
  case DEFGRUP:
  case ANULGRUP:  			gestioGrup();
									break;
  case FINTRUCGRUP:
									gestioFiTrucGrup();
									break;

  case RXPREFIXES: 			gestioTxPrefixes();
									break;

  case TST:
									gestioTst();
									break;
  case ESTAT:
									gestioEstat();
									break;
  case RXCONFIG:
									gestioRxConfig();
									break;

  case OPERNIT:				gestioOperNit();
									break;

  case SEND_PROG:
									gestioSEND_PROG();
									break;
  case CLEARED:
									gestioCLEARED();
									break;


  default:listExec[0].estat=FIEXEC;break;
 }
 tstComndAct();
}


void presenta(void)
{
// printf("CGCSys... <prem F per acabar>");
	FILE *f;

	int n;

	if ((f=fopen ("cgc.scr","rt"))!=NULL)
	{
		for (n=0;n<4000;n++)
			pokeb (0xb000,n,fgetc (f));
//			pokeb (0xb800,n,fgetc (f));

		fclose (f);
	}
	else
		printf ("No puedo abrir el fichero\n");

	gotoxy (1,1);
	printf ("CGC411T Electr¢nica Barcelona S.L.");

}

void creaCmnd(void)
{
 unsigned int i;

 if ((listCmnd=(cmndInfo *)malloc(sizeof(cmndInfo)*(MAX_NUM_COMND+1)))==NULL)
  error("FALTA MEM•RIA 0.");

 if ((listExec=(cmndInfo *)malloc(sizeof(cmndInfo)*(MAX_NUM_EXEC+1)))==NULL)
  error("FALTA MEM•RIA 2.");

 if ((listCanals=(cmndInfo *)malloc(sizeof(cmndInfo)*ACT_CH))==NULL)
  error("FALTA MEM•RIA 3.");

 if ((listIndics=(ptrRadioInfo)malloc(sizeof(radioInfo)*(MAX_NUM_RADIO)))==NULL)
  error("FALTA MEM•RIA 4.");

 if ((listOpers=(ptrOperInfo)malloc(sizeof(operInfo)*(ACT_OPER)))==NULL)
  error("FALTA MEM•RIA 5.");

 if ((listError=(char **)malloc(sizeof(char *)*1))==NULL)
  error("FALTA MEM•RIA");

 for(i=0;i<MAX_NUM_RADIO;i++)
 {
  listIndics[i].flota=0;
  listIndics[i].flotaAnt=0;
  listIndics[i].grup=0;
  listIndics[i].indicatiu=0;
  listIndics[i].permisVeu=0;
  listIndics[i].numSerie=0;
  listIndics[i].expedient=0;
  listIndics[i].servei=0;
  listIndics[i].lastFunc=RES;
  listIndics[i].canal=-1;
  if ((listIndics[i].fitxer=(char *)malloc(sizeof(char)*MAX_FITXER))==NULL)
		error("FALTA MEM•RIA 4.");

  strcpy(listIndics[i].fitxer,"");
  listIndics[i].numReTx=NUM_RETX;
 }

 for(i=0;i<ACT_OPER;i++)
 {
  listOpers[i].ctrlPTT=PTTOFF;
  listOpers[i].TxRadio=0;
  listOpers[i].estat=NOACTIU;
  listOpers[i].canalAss=-1;
 }

 for(i=0;i<MAX_NUM_COMND+1;i++)
 {
	 listCmnd[i].comnd=RES;
	 listCmnd[i].estat=NOTRUN;
	 listCmnd[i].origen=-1;
	 listCmnd[i].proces=0;

	 if ((listCmnd[i].info=(char *)malloc(sizeof(char)*418))==NULL)
		error("FALTA MEM•RIA 4.");


	 strcpy(listCmnd[i].info,"");
 }

 for(i=0;i<(MAX_NUM_EXEC+1);i++)
 {
	 listExec[i].comnd=RES;
	 listExec[i].estat=NOTRUN;
	 listExec[i].origen=-1;
	 listExec[i].proces=0;

	 if ((listExec[i].info=(char *)malloc(sizeof(char)*418))==NULL)
		error("FALTA MEM•RIA 5.");


	 strcpy(listExec[i].info,"");
 }

 for(i=0;i<ACT_CH;i++)
 {
	 listCanals[i].flag=0;
	 if ((listCanals[i].info=(char *)malloc(sizeof(char)*(MAX_FITXER+MAX_BLOQUE)))==NULL)
		error("FALTA MEM•RIA 5.");

	 rstCanal(i);
 }

/* for(i=0;i<NUM_MISSATGES;i++)
 {
	 if ((taulaMsg[i].info=(char *)malloc(sizeof(char)*(MAX_MSGTRAFIC)))==NULL)
		error("FALTA MEM•RIA 5.");
	 strcpy(taulaMsg[i].info,"");

 }*/




 lastCmnd=0;
 lastExec=0;
 comndAct=0;
 lastIndic=1;

// creaTaulaMsg();
}

void restauraInt(void)
{
  unsigned int n=PORT0;
  disable();

  for(n=PORT0;n<9;n++) sio_close(n);

  enable();

}


void main()
{
 unsigned int final=0,i;
 char ch;
directvideo = 1;
 FILE *f;
 int res=0;
 struct dostime_t t;
 struct dosdate_t d;

 f=fopen("cgc.log","a");

 _dos_gettime(&t);
 _dos_getdate(&d);

 fprintf(f,"%2d:%02d:%02d.%02d ", t.hour, t.minute, t.second, t.hsecond);
 fprintf(f,"%2d/%02d/%02d\n", d.year, d.day, d.month);
 fclose (f);


 signal (SIGINT, ErrorCritico);

 creaCmnd();
 initPorts();
 clrscr();
// if (!DebugMode) presenta();
 llegeixDisc();
 //desconectaPTT(0);


/* Tx2VAX(DES,2,0,"");
 Tx2VAX(OCU,2,0,"");
 Tx2VAX(FIN,2,0,"");
 Tx2VAX(DES,2,0,"");
 Tx2VAX(OCU,2,0,"");
 Tx2VAX(FIN,2,0,"");
 Tx2VAX(OCU,2,0,"");
 Tx2VAX(OCU,2,0,"");
 Tx2VAX(OCU,2,0,"");
 Tx2VAX(DES,2,0,"");
 Tx2VAX(DES,2,0,"");
 Tx2VAX(DES,2,0,"");
 Tx2VAX(DES,2,0,"");*/

//  Tx2MasterClk(CONFIG,1,0x0a,0); // entrada, sortida
//  restauraInt();
//  exit(0);

  int n=0;
  f= fopen("cgc.ind","rb");

	while (!feof (f))
	{
	  fscanf (f,"%d",&listIndics[n].indicatiu);

	  listIndics[n].flota=1;
	  listIndics[n].flotaAnt=0;
	  listIndics[n].grup=0;
	  listIndics[n].permisVeu=0;
	  listIndics[n].numSerie=0;
	  listIndics[n].expedient=0;
	  listIndics[n].servei=0;
	  listIndics[n].lastFunc=RES;
	  listIndics[n].canal=0;
	  n++;
	}
	lastIndic=n;
  fclose (f);

 do{
	while (!(kbhit()))
	{
	  execsys();
	  gotoxy (1,1);
	  printf("\nThe stack: %u    ",_stklen);
	  printf("The stack: %u\tstack pointer: %u", stackavail(), _SP);
	  gotoxy (30,5);
	  printf ("%d",putejar);

	  if (!DebugMode) ListIndics();
	}

 ch=getch();
  if (ch=='F') final=1;

 }while (final==0);
 restauraInt();

 for(i=0;i<MAX_NUM_RADIO;i++)
  free (listIndics[i].fitxer);

 for(i=0;i<MAX_NUM_COMND+1;i++)
  free (listCmnd[i].info);

 for(i=0;i<(MAX_NUM_EXEC+1);i++)
  free (listExec[i].info);

 for(i=0;i<ACT_CH;i++)
  free (listCanals[i].info);

 free (listCmnd);
 free (listExec);
 free (listCanals);
 free (listIndics);
 free (listOpers);
 free (listError);

 printf("GCGSys Acaba...\n");


//	for (;;)
//	  execsys();

}


void ListColas (void)
{
	int n;

	for (n=0;n<16;n++)
	{
	}

}
void ListIndics(void)
{
	int n;
	int y=3;
	int y2=3;
	int sio;
	int y1;
	static int lIndic;

	if (lIndic!=lastIndic)
	{
		lIndic=lastIndic;

		for (n=0;n<14;n++)
		{
		  if (listIndics[n].indicatiu)
		  {
			  gotoxy (3,y+n);
			  printf ("%d  ",listIndics[n].indicatiu);
			  gotoxy (11,y+n);
			  printf ("%d  ",listIndics[n].flota);
			  gotoxy (18,y+n);
			  printf ("%d  ",listIndics[n].canal+1);

		  }
		  else
		  {

			  gotoxy (3,y+n);
			  printf ("   ");
			  gotoxy (11,y+n);
			  printf ("     ");
			  gotoxy (18,y+n);
			  printf ("   ");
		  }

	  }
 }
 gotoxy (4,17);
	printf ("  RADIO  ");

 gotoxy (4,19);
	printf ("Canal 1 %d ",listCanals[0].estat);
 gotoxy (4,20);
	printf ("Canal 2 %d ",listCanals[1].estat);
 gotoxy (4,21);
	printf ("Canal 7 %d ",listCanals[2].estat);
 gotoxy (4,23);
	printf ("Canal g %d ",listCanals[3].estat);

	y1=18;
/*	if (listCanals[0].estat==0)
	{
		sound (1000);
		delay (100);
		nosound();
	}*/
	gotoxy (10,19);

//	printf ("Numero serveis %d ",posicioIndicatiu(atoi(bufRxHeader2)));

	gotoxy (1,24);
	for (n=0;n<lastExec;n++)
		printf ("%d ",listExec[n].comnd);
		printf ("    ");

}
/*void creaTaulaMsg(void)
{

  unsigned int i;
  taulaMsg[0].num=ERRHOST;
  strcpy(taulaMsg[0].info,"ERRHOST");

  taulaMsg[1].num=PETVEUE;
  strcpy(taulaMsg[1].info,"PETVEUE");

  taulaMsg[2].num=PETVEUS;
  strcpy(taulaMsg[2].info,"PETVEUS");

  taulaMsg[3].num=TRUCINDIV;
  strcpy(taulaMsg[3].info,"TRUCIND");

  taulaMsg[4].num=ANULVEU;
  strcpy(taulaMsg[4].info,"ANULVEU");

  taulaMsg[5].num=MSEMER;
  strcpy(taulaMsg[5].info,"MESEMER");

  taulaMsg[6].num=MSSERVEIS;
  strcpy(taulaMsg[6].info,"MESERVE");

  taulaMsg[7].num=SQLON;
  strcpy(taulaMsg[7].info,"SQLON  ");

 taulaMsg[8].num=SQLOFF;
 strcpy(taulaMsg[8].info,"SQLOFF ");

 taulaMsg[9].num=DES;
 strcpy(taulaMsg[9].info,"DESPLA€");

 taulaMsg[10].num=PAR;
 strcpy(taulaMsg[10].info,"PARAT  ");

 taulaMsg[11].num=OBE;
 strcpy(taulaMsg[11].info,"OBERT  ");

 taulaMsg[12].num=OCU;
 strcpy(taulaMsg[12].info,"OCUPAT ");

 taulaMsg[13].num=LLI;
 strcpy(taulaMsg[13].info,"LLIURE ");

 taulaMsg[14].num=FIN;
 strcpy(taulaMsg[14].info,"FISERV ");

 taulaMsg[15].num=TAN;
 strcpy(taulaMsg[15].info,"TANCAT ");

 taulaMsg[16].num=ERR;
 strcpy(taulaMsg[16].info,"ERRVAX ");

 taulaMsg[17].num=ANU;
 strcpy(taulaMsg[17].info,"ANULAT ");

 taulaMsg[18].num=ASI;
 strcpy(taulaMsg[18].info,"ASSIGNA");

 taulaMsg[19].num=PROGINDIC;
 strcpy(taulaMsg[19].info,"CVFLOTA");

 taulaMsg[20].num=MOD;
 strcpy(taulaMsg[20].info,"MODIFIC");

 taulaMsg[21].num=NIT;
 strcpy(taulaMsg[21].info,"NIT    ");

 taulaMsg[22].num=ESTATPTT;
 strcpy(taulaMsg[22].info,"PTT-TX ");


 taulaMsg[23].num=TRUCGRUP;
 strcpy(taulaMsg[23].info,"TRUCGRP");

 taulaMsg[24].num=DEFGRUP;
 strcpy(taulaMsg[24].info,"DEFIGRP");

 taulaMsg[25].num=ANULGRUP;
 strcpy(taulaMsg[25].info,"ANULGRP");

 taulaMsg[26].num=FINTRUCGRUP;
 strcpy(taulaMsg[26].info,"FITRGRP");

 for(i=27;i<NUM_MISSATGES;i++)
 {
  taulaMsg[i].num=RES;
  strcpy(taulaMsg[i].info,"");
 }

// taulaMsg[31].num=TST;
// strcpy(taulaMsg[31].info,"TEST   ");

// taulaMsg[32].num=ESTAT;
// strcpy(taulaMsg[32].info,"ESTATX ");

// taulaMsg[33].num=RXCONFIG;
// strcpy(taulaMsg[33].info,"RXCONFI");

// taulaMsg[34].num=OPERNIT;
// strcpy(taulaMsg[34].info,"OPERNIT");
//  taulaMsg[1].num=INI_CONFIG;
//  strcpy(taulaMsg[1].info,"INICONF");

//  taulaMsg[8].num=VEU;
//  strcpy(taulaMsg[8].info,"VEU    ");

// taulaMsg[23].num=OBRIR;
// strcpy(taulaMsg[23].info,"OBRIR  ");

}
*/
void ErrorCritico (int ErrList)
{
	printf ("-------------- Error -------------");
	for (;;);
}