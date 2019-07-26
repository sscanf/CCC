#include "c:defspan.h"
extern void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/

extern unsigned int bufRxHeader[HEADER];
extern char bufRxDades[MAX_BUFFER_RX];

unsigned long numSerie=3456;
float potencia=134.6,roe=145;
unsigned int canals=ACT_CH,consoles=ACT_OPER;
unsigned int mesCan=0,mesOper=0,ini=0,error=0;

unsigned int numChAct=1,numOperAct=1;
unsigned int posCh=0,posOper=0,nTChPan=1,nTOperPan=1;

unsigned int posLine=0,iniLine=0,posWrite=1;

int tecnic=0;

ptrRxInfo listRxCmnd;
unsigned int lastCmnd=0,comndAct=0;

unsigned int configRx=0,RxEstat=0,Nit=0,prefixesRx=0,trucCanalFin=0;

char **listError;
unsigned int CGCOn=OFF;
unsigned int inici=ON;

char traficBuffer[MAX_NUMLINES][MAX_LENGTHLINE];
unsigned int lastSourcePTT=CANALS;
char lastPTT[LONG_INDIC+2]="";

ptrEstatData estatData;
ptrRepData repData;
ptrEqMobData eqMobData;
ptrCGCData ConfigCGCBuf[MAX_CH_BUF][MAX_OPER_BUF];
ptrIndicsData llistaIndics;
ptrIndicsData llistaEmers;
ptrFinVeuData veuData;
ptrFinInData inData;
ptrTrucGrupData trucData;
ptrTrucGrupData2 trucData2;
ptrTrucCanalData trucCanalData;
ptrDefGrupData defData;
ptrAnulGrupData anulData;
ptrRepData1 repData1;
ptrIndics stringIndics;
ptrIndics stringEmers;

char *stringTrucsGrup[MAX_GRUP];
char *stringRep[MAX_REP];
char *stringTrucCanal[MAX_NUM_CANAL];

char serv2Af[LONG_INDIC]="001";
char emer2Af[LONG_INDIC]="001";

int numServeis=0,numEmer=MAX_EMER,numGrups=0,numReps=0;

unsigned int numFin=1;

ptrObrirData obrirData;


extern TScrollBar *vScrollBar;

TListBox2 *indicsBox;
TListWindow *serveisListBox;
TListBox3 *emersBox;
TListWindow2 *emersListBox;
TListBox5 *trucGrupBox;
TDialog *trucGrupDialog;
TListBox6 *trucCanalBox;
TListWindow3 *trucCanalDialog;
TDialog *configRepDialog=NULL;
TDialog *eqMobDialog=NULL;
TTraficWindow *traficWin=NULL;
unsigned int iniLineAnt=0;
char lineAnt[MAX_LENGTHLINE]=" Comanda  Flota  Indicatiu  Origen  Altres";

void initDefData(void);

void TMyApp::newDialog(int proc)
{
 TDialog *pd1;
 char dest[LONG_INDIC+2]="001";
 char tempo[LONG_GRUP];
 ushort control;
 unsigned int trobat=1,i;
 TEvent event2;

 switch(proc)
 {
  case cmObrir:
	pd1=new TDialog(TRect(3,6,29,16),"CGC respon per...");
	if (pd1)
	{
	 pd1=dialogObrir(pd1);
	 pd1->selectNext((Boolean)0);
	 pd1->setData(obrirData);
	 control=deskTop->execView(pd1);
	 if (control!=cmCancel)
	 {
		pd1->getData(obrirData);
		TEvent event;
		event.what=evCommand;
		event.message.command=cmCanviObrir;
		putEvent(event);
	 }
	}
  break;
  case cmFinVeu:
	pd1=new TDialog(TRect(3,6,34,15),"Trucada Individual");
	if (pd1)
	{
	 pd1=dialogVeu(pd1);
	 pd1->selectNext((Boolean)0);
	 pd1->setData(veuData);
	 control=deskTop->execView(pd1);
	 if (control!=cmCancel)
	 {
		pd1->getData(veuData);
		Tx2CGC(cmFinVeu,strlen(veuData->indicatiu),0,veuData->indicatiu);
	 }
	}
  break;
  case cmTrucGrup:
	pd1=new TDialog(TRect(3,6,33,20),"Trucada Grup");
	trucGrupDialog=pd1;
	if (pd1)
	{
	 pd1=dialogTrucGrup(pd1);

	 if (numGrups==0)
	 {
	  stringTrucsGrup[0]=new char[LONG_GRUP];
	  strcpy(stringTrucsGrup[0],"1");
	  trucData->focus=0;
	  trucData2->focus=0;
	 }

	 strcpy(trucData->grup,stringTrucsGrup[trucData->focus]);


	 pd1->selectNext((Boolean)0);
	 pd1->setData(trucData);
	 control=deskTop->execView(pd1);
	 pd1->getData(trucData);
	 Tx2CGC(cmFinTrucGrup,strlen(stringTrucsGrup[trucData->focus]),0,stringTrucsGrup[trucData->focus]);
	 extreuTrucGrup();
	}
	break;
  case cmDefGrup:
	char dest[LONG_GRUP+8+(LONG_INDIC*8)]="";
	pd1=new TDialog(TRect(3,4,33,20),"Definici¢ Grup");
	if (pd1)
	{
	 pd1=dialogDefGrup(pd1);
	 pd1->selectNext((Boolean)0);
	 pd1->setData(defData);
	 control=deskTop->execView(pd1);
	 if (control!=cmCancel)
	 {
		pd1->getData(defData);
		if (defData->grup[0]=='\x0' && defData->grup[1]>='0' && defData->grup[1]<='9') defData->grup[0]='0';
		if (defData->grup[1]=='\x0' && defData->grup[0]>='0' && defData->grup[0]<='9') {defData->grup[1]=defData->grup[0]; defData->grup[0]='0';}
		if ((defData->grup[0]<'0' || defData->grup[0]>'9') || (defData->grup[1]<'0' || defData->grup[1]>'9') || atoi(defData->grup)==0)
		{
		 messageBox("\03Error: nombre de grup incorrecte.",mfOKButton|mfError);
		 event2.what=evCommand;
		 event2.message.command=cmDefGrup;
		 putEvent(event2);
		}
		else
		{
		 strcat(dest,defData->grup);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind1);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind2);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind3);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind4);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind5);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind6);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind7);
		 strcat(dest,"\n");
		 strcat(dest,defData->ind8);
		 strcat(dest,"\n");
		 Tx2CGC(cmDefGrup,strlen(dest),0,dest);
		}
	 }
	}
	break;
  case cmEstat:
	pd1=new TDialog(TRect(3,3,29,15),"Estat Xarxa");
	if (pd1)
	{
	 pd1=dialogEstat(pd1);
	 llegeixEstatData();
	 pd1->setData(estatData);
	 control=deskTop->execView(pd1);
	}

	break;
  case cmRep1:
	pd1=new TDialog(TRect(3,6,40,20),"Config. Repetidors");
	if (pd1)
	{
	 pd1=dialogRep(pd1);

	 if (numReps==0)
	 {
	  stringRep[0]=new char[LONG_REP];
	  strcpy(stringRep[0],"COLLSEROLA");
	  repData1->focus=0;
	 }

	 strcpy(repData1->rep,stringRep[repData1->focus]);

	 pd1->selectNext((Boolean)0);
	 pd1->setData(repData1);
	 control=deskTop->execView(pd1);
	 if (control!=cmCancel)
	 {
		pd1->getData(repData1);
		strcpy(repData->repetidor,stringRep[repData1->focus]);
		TEvent event;
		event.what=evCommand;
		event.message.command=cmRep;
		putEvent(event);
	 }
	}
	break;

  default:break;
  }
 destroy(pd1);
 if (proc==cmTrucGrup)  trucGrupBox=new TListBox5(TRect(3,5,16,12),1,0);
}

void TMyApp::gestioTrucCanal(void)
{
}

void TMyApp::llegeixEstatData(void)
{

 char dest4[(ACT_OPER*4)+4]="";
 char dest5[3]="";
 unsigned int i,j,length=0,estat=0,oper;

 strcpy(dest4,listRxCmnd[comndAct].info);
 length=strlen(dest4);

 for(;length!=0;)
 {
  for(j=0;j<2 && dest4[j]!='\n';j++)
	dest5[j]=dest4[j];
  dest5[j]='\0';
  oper=atoi(dest5);
  memmove(dest4,dest4+strlen(dest5)+1,sizeof(char)*(length-strlen(dest5)-1));
  dest4[length-strlen(dest5)-1]='\0';
  if (dest4[0]=='1') // ACTIU
	  estat=1;
  else //NOACTIU
	  estat=0;
  if (strlen(dest4)>1)
  {
	memmove(dest4,dest4+1,sizeof(char)*(strlen(dest4)-1));
	dest4[strlen(dest4)-1]='\0';
  }
  else dest4[0]='\0';
  switch(oper)
  {
	 case 0:estatData->info1=estat;break;
	 case 1:estatData->info2=estat;break;
	 case 2:estatData->info3=estat;break;
	 case 3:estatData->info4=estat;break;
	 case 4:estatData->info5=estat;break;
	 case 99:estatData->infoVAX=estat;break;
	 default:;
  }
  length=strlen(dest4);
  strcpy(dest5,"");
 }
}

char *TMyApp::entradaDades(char *tempo)
{
	TDialog *pd1;

	pd1=new TDialog(TRect(0,17,15,23),"Indic");
	if (pd1)
	{
	 if (tempo[strlen(tempo)-1]!='*')
		strcpy(inData->indicatiu,tempo);
	 else
	 {
		strncpy(inData->indicatiu,tempo,(strlen(tempo)-2));
	 }
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TInputLine(TRect(3,1,13,2),LONG_INDIC));
	 pd1->insert(new TButton(TRect(3,3,13,5),"~O~k",cmOK,bfDefault));
	 pd1->selectNext((Boolean)0);
	 pd1->setData(inData);
	 ushort control=deskTop->execView(pd1);
	 if (control!=cmCancel)
		pd1->getData(inData);
	}
	destroy(pd1);
	return (inData->indicatiu);
}

TDialog *TMyApp::dialogVeu(TDialog *pd1)
{
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TInputLine(TRect(13,2,27,3),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(2,2,12,3),"Indica~t~iu",0));
	 pd1->insert(new TButton(TRect(4,5,14,7),"~O~k",cmOK,bfDefault));
	 pd1->insert(new TButton(TRect(16,5,26,7),"~C~ancel",cmCancel,bfNormal));
	 return pd1;
}

TDialog *TMyApp::dialogEstat(TDialog *pd1)
{
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TCheckBoxes(TRect(3,2,23,3),
					 new TSItem("Op.1  Actiu",0)));
	 pd1->insert(new TCheckBoxes(TRect(3,3,23,4),
					 new TSItem("Op.2  Actiu",0)));
	 pd1->insert(new TCheckBoxes(TRect(3,4,23,5),
					 new TSItem("Op.3  Actiu",0)));
	 pd1->insert(new TCheckBoxes(TRect(3,5,23,6),
					 new TSItem("Op.4  Actiu",0)));
	 pd1->insert(new TCheckBoxes(TRect(3,6,23,7),
					 new TSItem("Op.5  Actiu",0)));
	 pd1->insert(new TCheckBoxes(TRect(3,7,23,8),
					 new TSItem("Com.Host Actiu", 0 )));
	 pd1->insert(new TButton(TRect(7,9,17,11),"~O~k",cmOK,bfDefault));
	 return pd1;
}

TDialog *TMyApp::dialogObrir(TDialog *pd1)
{
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TLabel(TRect(3,2,17,3),"Obertura per:",0));
	 pd1->insert(new TRadioButtons(TRect(4,4,22,6),
					 new TSItem("~S~ilenciador",
					 new TSItem("~C~odis", 0 ))));
	 pd1->insert(new TButton(TRect(8,7,18,9),"~O~k",cmOK,bfDefault));
	 return pd1;
}

TDialog *TMyApp::dialogTrucGrup(TDialog *pd1)
{

	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TInputLine(TRect(9,2,16,3),LONG_GRUP));
	 pd1->insert(new TLabel(TRect(2,2,8,3),"~G~rup",0));
	 pd1->insert(new TLabel(TRect(3,4,16,5),"Ja definits:",0));
	 pd1->insert(trucGrupBox);
	 pd1->insert(new TButton(TRect(18,10,28,12),"~F~i",cmOK,bfDefault));
	 return pd1;
}


TDialog *TMyApp::dialogRep(TDialog *pd1)
{
	 TListBox *repBox;
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TInputLine(TRect(7,2,22,3),LONG_REP));
	 pd1->insert(new TLabel(TRect(2,2,6,3),"~R~ep",0));
	 pd1->insert(new TLabel(TRect(3,4,16,5),"Repetidors",0));
	 repBox=new TListBox4(TRect(3,5,LONG_REP+3,12),1,0);
	 pd1->insert(repBox);
	 pd1->insert(new TButton(TRect(24,6,34,8),"~O~k",cmOK,bfDefault));
	 pd1->insert(new TButton(TRect(24,10,34,12),"~C~ancel",cmCancel,bfNormal));

	 return pd1;
}

TDialog *TMyApp::dialogDefGrup(TDialog *pd1)
{
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TInputLine(TRect(9,2,16,3),LONG_GRUP));
	 pd1->insert(new TLabel(TRect(2,2,8,3),"~G~rup",0));
	 pd1->insert(new TLabel(TRect(2,4,16,5),"Indicatius:",0));
	 pd1->insert(new TLabel(TRect(3,5,6,6),"~1~.",0));
	 pd1->insert(new TInputLine(TRect(7,5,16,6),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,6,6,7),"~2~.",0));
	 pd1->insert(new TInputLine(TRect(7,6,16,7),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,7,6,8),"~3~.",0));
	 pd1->insert(new TInputLine(TRect(7,7,16,8),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,8,6,9),"~4~.",0));
	 pd1->insert(new TInputLine(TRect(7,8,16,9),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,9,6,10),"~5~.",0));
	 pd1->insert(new TInputLine(TRect(7,9,16,10),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,10,6,11),"~6~.",0));
	 pd1->insert(new TInputLine(TRect(7,10,16,11),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,11,6,12),"~7~.",0));
	 pd1->insert(new TInputLine(TRect(7,11,16,12),LONG_INDIC));
	 pd1->insert(new TLabel(TRect(3,12,6,13),"~8~.",0));
	 pd1->insert(new TInputLine(TRect(7,12,16,13),LONG_INDIC));
	 pd1->insert(new TButton(TRect(18,6,28,8),"~O~k",cmOK,bfDefault));
	 pd1->insert(new TButton(TRect(18,10,28,12),"~C~ancel",cmCancel,bfNormal));

	 return pd1;
}

void TMyApp::gestioVeu(TEvent &event)
{
 char tempo[LONG_INDIC+2];
 TView *tempo2;
 unsigned int serveis=2,numProc;


 tempo2=deskTop->first();
 if (tempo2==serveisListBox)
 {
	serveis=1;
	if (numServeis==0)
	{
	 messageBox("\03No hi ha cap IncidŠncia.\n\03Premi Alt-2 per activar S.O.S.\n\03o Alt-3 pels canals.",mfOKButton|mfInformation);
	 return;
	}
	else
	{
	  serveisListBox->getData(llistaIndics);
	  strcpy(tempo,stringIndics[llistaIndics->focus].indicatiu);
	  numProc=stringIndics[llistaIndics->focus].numProc;
	}
 }
 else
 {
	if (tempo2==emersListBox)
	{
	  serveis=0;
	  if (numEmer==MAX_EMER)
	  {
		messageBox("\03No hi ha cap EmergŠncia.\n\03Premi Alt-1 per activar Incid.\n\03o Alt-3 pels canals.",mfOKButton|mfInformation);
		return;
	  }
	  else
	  {
			 emersListBox->getData(llistaEmers);
			 strcpy(tempo,stringEmers[numEmer+llistaEmers->focus+1].indicatiu);
			 numProc=stringEmers[numEmer+llistaEmers->focus+1].numProc;
	  }
	}
	else
	 if (tecnic==0)
	 {
		 messageBox("\03Premi Alt-1 o Alt-2 per activar S.0.S. o IncidŠncies.",mfOKButton|mfError);
		 return;
	 }
	 else
	  if (tecnic==1)
	  {
		 messageBox("\03Premi Alt-I o Alt-E per crear finestres S.0.S. o IncidŠncies.",mfOKButton|mfError);
		 return;
	  }

 }

 if (serveis==1)
 {
  // finestra de serveis activa
  if ((tempo[strlen(tempo)-1])=='*')
  {
	  tempo[strlen(tempo)-2]='\0';
	  Tx2CGC(cmAnulVeu,strlen(tempo),numProc,tempo);
	  extreuServeis(0);
	  lastSourcePTT=CANALS;
  }
  else
	  Tx2CGC(cmVeu,strlen(tempo),numProc,tempo); // demana veu a CGC i aquest retorna comanda de modificar serveis
 }
 else
  if (serveis==0)
  // finestra d'emergŠncies activa
  {
	  if ((tempo[strlen(tempo)-1])=='*')
	  {
		 tempo[strlen(tempo)-2]='\0';
		 Tx2CGC(cmAnulVeu,strlen(tempo),numProc,tempo);
		 extreuEmers(0);
	  }
	  else
		  Tx2CGC(cmVeuE,strlen(tempo),numProc,tempo);
  }
}


void TMyApp::gestioVeu2()
{
 char tempo[LONG_INDIC+9];
 TView *tempo2;
 unsigned int serveis=2,numProc,focus=0;


  tempo2=deskTop->first();

  if (tempo2==serveisListBox)
  {
	serveis=1;
	if (listRxCmnd[comndAct].numProc==0 && lastSourcePTT!=INCID && lastSourcePTT!=RES)
	// ens trobem a la finestra Incid n'hi hagin o no, hi ha una
	// desconnexi¢ de PTT per servir
	{
	  Tx2CGC(cmPTT,strlen(lastPTT),listRxCmnd[comndAct].numProc,lastPTT);
	  strcpy(lastPTT,"");
	  lastSourcePTT=RES;
	  return;
	}
	else
	{
	 if (numServeis==0 && (*obrirData).info==1 && listRxCmnd[comndAct].numProc==1) // per codis
	 {
	  messageBox("\03No hi ha cap IncidŠncia.\n\03Premi Alt-2 per activar S.O.S.\n\03 o Alt-3 pels canals.",mfOKButton|mfInformation);
	  return;
	 }
	 else
	 {
	 // Si ens trobem en la finestra d'Incid, hi han Incid per servir i polsem PTT
	 // es gestionar… si t‚ perm¡s de veu sin¢ donar… un missatge informatiu.


	  if (numServeis>0) // per codis o silenciador
	  {
		serveisListBox->getData(llistaIndics);
		strcpy(tempo,stringIndics[llistaIndics->focus].indicatiu);
		numProc=stringIndics[llistaIndics->focus].numProc;
	  }
	  else // per silenciador i cap servei
	  {
		if ((*obrirData).info==0 && listRxCmnd[comndAct].numProc==1 && trucCanalFin==0)
		{
		 messageBox("\03No hi ha cap IncidŠncia.\n\03Premi Alt-2 per activar S.O.S.\n\03 o Alt-3 pels canals.",mfOKButton|mfInformation);
		 return;
		}
		else
		 if (trucCanalFin==1 && listRxCmnd[comndAct].numProc==1)
		 {
		  serveis=3;
		  trucCanalBox->getData(trucCanalData);
		  strcpy(tempo,stringTrucCanal[trucCanalData->focus]);
		 }
	  }// tanca per silenciador i cap servei
	 }// tanca si ens trobem a Incid
	}
  }// tanca si incid activa
  else
  {
	if (tempo2==emersListBox)
	{
	  serveis=0;
	  if (listRxCmnd[comndAct].numProc==0 && lastSourcePTT!=SOS && lastSourcePTT!=RES)
	  // ens trobem a la finestra Incid n'hi hagin o no i hi ha una
	  // desconnexi¢ de PTT per servir
	  {
		Tx2CGC(cmPTT,strlen(lastPTT),listRxCmnd[comndAct].numProc,lastPTT);
		strcpy(lastPTT,"");
		lastSourcePTT=RES;
		return;
	  }
	  else
	  {
		if (numEmer==MAX_EMER && (*obrirData).info==1 && listRxCmnd[comndAct].numProc==1) // per codis
		{
		 messageBox("\03No hi ha cap EmergŠncia.\n\03Premi Alt-1 per activar Inc.\n\03 o Alt-3 pels canals.",mfOKButton|mfInformation);
		 return;
		}
		else
		{
		 if (numEmer<MAX_EMER) // per codis o silenciador
		 {
		  emersListBox->getData(llistaEmers);
		  strcpy(tempo,stringEmers[numEmer+llistaEmers->focus+1].indicatiu);
		  numProc=stringEmers[numEmer+llistaEmers->focus+1].numProc;
		 }
		 else // per silenciador i cap emer
		 {
		  if ((*obrirData).info==0 && listRxCmnd[comndAct].numProc==1 && trucCanalFin==0)
		  {
			messageBox("\03No hi ha cap EmergŠncia.\n\03Premi Alt-1 per activar Inc.\n\03 o Alt-3 pels canals.",mfOKButton|mfInformation);
			return;
		  }
		  else
			if (trucCanalFin==1 && listRxCmnd[comndAct].numProc==1)
			{
			 serveis=3;
			 trucCanalBox->getData(trucCanalData);
			 strcpy(tempo,stringTrucCanal[trucCanalData->focus]);
			}
		 }// tanca per silenciador i cap emer
		}// tanca numemer<MAX_EMER
	  }
	 }// tanca if emersListbox
	 else
	 {
	  if (tempo2==trucCanalDialog || tempo2==traficWin)
	  {
		if (listRxCmnd[comndAct].numProc==0 && lastSourcePTT!=CANALS && lastSourcePTT!=RES)
		// ens trobem a la finestra Incid n'hi hagin o no i hi ha una
		// desconnexi¢ de PTT per servir
		{
		 Tx2CGC(cmPTT,strlen(lastPTT),listRxCmnd[comndAct].numProc,lastPTT);
		 strcpy(lastPTT,"");
		 lastSourcePTT=RES;
		 return;
		}
		else
		{
		 serveis=3;
		 trucCanalBox->getData(trucCanalData);
		 strcpy(tempo,stringTrucCanal[trucCanalData->focus]);
		}
	  }
	  else
	  {
		if (listRxCmnd[comndAct].numProc==1)
		{
		  if (tecnic==0)
		  {
			 messageBox("\03Premi Alt-1 o Alt-2 per activar S.0.S. o IncidŠncies.",mfOKButton|mfError);
			 return;
		  }
		  else
		  if (tecnic==1)
		  {
			 messageBox("\03Premi Alt-I o Alt-E per crear finestres S.0.S. o IncidŠncies.",mfOKButton|mfError);
			 return;
		  }
		 }
		 else
		 {
		  if (listRxCmnd[comndAct].numProc==0 && lastSourcePTT!=RES)
		  {
			Tx2CGC(cmPTT,strlen(lastPTT),listRxCmnd[comndAct].numProc,lastPTT);
			strcpy(lastPTT,"");
			lastSourcePTT=RES;
		  }
		  return;
		 }
		}
	 }
  }// tanca si != incid activa


	if (serveis==1)
	// finestra de serveis activa
	{

	if ((tempo[strlen(tempo)-1])=='*')
	 {

	  tempo[strlen(tempo)-2]='\0';

	  Tx2CGC(cmPTT,strlen(tempo),listRxCmnd[comndAct].numProc,tempo);
	  if (listRxCmnd[comndAct].numProc==1)
	  {
		  lastSourcePTT=INCID;
		  strcpy(lastPTT,tempo);
	  }
	  else
	  {
		  lastSourcePTT=RES;
		  strcpy(lastPTT,"");
	  }

	 }
	 else
		if (listRxCmnd[comndAct].numProc==1)
		{
		 messageBox("\03L'indicatiu seleccionat no t‚\n\03perm¡s de veu.",mfOKButton|mfError);
		 return;
		}

	}
	else
	 if (serveis==0)
	 {
	 // finestra d'emergŠncies activa
	  if ((tempo[strlen(tempo)-1])=='*')
	  {
		 tempo[strlen(tempo)-2]='\0';
		 Tx2CGC(cmPTT,strlen(tempo),listRxCmnd[comndAct].numProc,tempo);
		 if (listRxCmnd[comndAct].numProc==1)
		 {
		  lastSourcePTT=SOS;
		  strcpy(lastPTT,tempo);
		 }
		 else
		 {
		  lastSourcePTT=RES;
		  strcpy(lastPTT,"");
		 }
	  }
	  else
		  if (listRxCmnd[comndAct].numProc==1)
		  {
			messageBox("\03L'indicatiu seleccionat no t‚\n\03perm¡s de veu.",mfOKButton|mfError);
			return;
		  }
	 }
	 else
	 // finestra canals activa
	 {
	  if ((tempo[strlen(tempo)-1])=='*')
	  {
		// ho enviarem sempre sense asterisc
		tempo[strlen(tempo)-2]='\0';
		strcpy(stringTrucCanal[trucCanalData->focus],tempo);
		trucCanalData->items->atPut(trucCanalData->focus,stringTrucCanal[trucCanalData->focus]);
		destroy(trucCanalBox);
		trucCanalBox=new TListBox6(TRect(1,1,21,ACT_CH+1),1,0);
		trucCanalBox->setData(trucCanalData);
		trucCanalDialog->insert(trucCanalBox);
	  }

	  if (tempo[strlen(tempo)-1]=='7') strcpy(tempo," Canal 3");
	  if (tempo[strlen(tempo)-1]=='G') strcpy(tempo," Canal 4");
	  Tx2CGC(cmPTT,strlen(tempo),listRxCmnd[comndAct].numProc,tempo);

	  if (listRxCmnd[comndAct].numProc==1)
	  {
		  lastSourcePTT=CANALS;
		  strcpy(lastPTT,tempo);
	  }
	  else
	  {
		lastSourcePTT=RES;
		strcpy(lastPTT,"");
	  }
	 }
}

void TMyApp::dialogEqMob()
{
  eqMobDialog=new TDialog(TRect(3,3,51,19),"Equips M•bils");
  if (eqMobDialog)
  {
	 eqMobDialog=configEqMob(eqMobDialog);
	 eqMobDialog->selectNext((Boolean)0);
	 eqMobDialog->selectNext((Boolean)0);
	 eqMobDialog->setData(eqMobData);
	 deskTop->execView(eqMobDialog);
  }
}

TDialog *TMyApp::configEqMob(TDialog *pd1)
{

 char string[30];
 unsigned int lenSerie;

 char nSLabel[25]="Num SŠrie ";


 pd1->flags=pd1->flags&(~wfClose);
 pd1->insert(new TInputLine(TRect(17,2,31,3),LONG_INDIC));
 pd1->insert(new TLabel(TRect(3,2,17,3),"~I~ndicatiu",0));
 pd1->insert(new TRepButton(TRect(35,2,45,4),"~E~nlla‡",cmTx,bfNormal));
 pd1->insert(new TLabel(TRect(-1,4,48,5),"ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ ESTAT ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶",0));
 pd1->insert(new TInputLine(TRect(17,6,31,7),LONG_INDIC));
 pd1->insert(new TLabel(TRect(3,6,17,7),"I~n~dicatiu",0));
 pd1->insert(new TLabel(TRect(3,8,9,9),"Estat",0));
 pd1->insert(new TRadioButtons(TRect(3,9,24,10),
		 new TSItem("~P~erm¡s de Veu", 0 )));
 ultoa(numSerie,string,10);
 strcat(nSLabel,string);
 lenSerie=strlen(nSLabel);
 pd1->insert(new TLabel(TRect(3,11,lenSerie+3,12),nSLabel,0));
 pd1->insert(new TLabel(TRect(-1,13,48,14),"ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶",0));
 pd1->insert(new TRepButton(TRect(35,8,45,10),"~O~k",cmOK,bfDefault));
 pd1->insert(new TRepButton(TRect(35,11,45,13),"~C~ancel",cmCancel,bfNormal));
 return pd1;
}

void TMyApp::dialogRep2()
{
  configRepDialog=new TDialog(TRect(3,3,60,20),"Repetidor");

  if (configRepDialog)
  {
	 configRepDialog=configRep(configRepDialog);
	 configRepDialog->selectNext((Boolean)0);
	 configRepDialog->selectNext((Boolean)0);
	 configRepDialog->setData(repData);
	 deskTop->execView(configRepDialog);
  }
  destroy(configRepDialog);

}

void TRepButton::handleEvent(TEvent& event)
{
 int repTx=OFF;
 TView *tempo2;
 unsigned int tempoX=4,tempoY=15;
 static TDStaticText *tempo1=NULL;

 if (eqMobDialog!=NULL)
 {
	tempo2=eqMobDialog;
	tempoY=14;
 }
 else
  if (configRepDialog!=NULL) tempo2=configRepDialog;

 TButton::handleEvent(event);
 if (event.what==evCommand)
 {
  switch (event.message.command)
  {
	case cmTx:

//		Tx2CGC(cmTx,strlen(repData->repetidor),repData->repetidor);
//		getAnswerCGC(cmConfigRep);
		if (repTx==ON)
		{
		 if (tempo1!=NULL)
		 {
		  destroy(tempo1);
		  tempo1=NULL;
		 }
		 if (tempo2==configRepDialog) tempo2->setData(repData);
		 else tempo2->setData(eqMobData);
		}
		else
		{
		  if (tempo1==NULL)
		  {
			tempo1=new TDStaticText(TRect(tempoX,tempoY,tempoX+32,tempoY+1),"No s'ha establert comunicaci¢.");
			if (tempo2==configRepDialog) configRepDialog->insert(tempo1);
			else eqMobDialog->insert(tempo1);
		  }
		}
	break;
	case cmOK:
	 if (tempo2==configRepDialog) tempo2->getData(repData);
	 else tempo2->getData(eqMobData);
	 if (tempo1!=NULL)
	 {
	  destroy(tempo1);
	  tempo1=NULL;
	 }
	 endModal(cmOK);
	 break;
	case cmCancel:
	 if (tempo1!=NULL)
	 {
	  destroy(tempo1);
	  tempo1=NULL;
	 }
	 endModal(cmCancel);
	 break;
	default:;
  }
  clearEvent(event);
 }
}




TDialog *TMyApp::configRep(TDialog *pd1)
{
  char potLabel[25]="PotŠncia ";
  char roeLabel[25]="ROE ";
  char *string;
  int dec,sign;
  unsigned int lenpot,lenroe;

  pd1->flags=pd1->flags&(~wfClose);
  pd1->insert(new TInputLine(TRect(14,2,20+LONG_REP,3),LONG_REP));
  pd1->insert(new TLabel(TRect(3,2,13,3),"Re~p~etidor",0));
  pd1->insert(new TRepButton(TRect(43,2,53,4),"~E~nlla‡",cmTx,bfNormal));
  pd1->insert(new TLabel(TRect(-1,4,57,5),"ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ ESTAT ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶",0));
  pd1->insert(new TLabel(TRect(3,6,15,7),"Rel‚s",0));
  pd1->insert(new TCheckBoxes(TRect(3,7,25,10),
		new TSItem("Repetidor ~a~ctiu",
		new TSItem("Rel‚ ~2~",
		new TSItem("Rel‚ ~3~",0) ))));
  pd1->insert(new TInputLine(TRect(41,7,50,8),3));
  pd1->insert(new TLabel(TRect(31,7,40,8),"Subt• ~T~x",0));
  pd1->insert(new TInputLine(TRect(41,9,50,10),3));
  pd1->insert(new TLabel(TRect(31,9,40,10),"Subt• ~R~x",0));
  string = ecvt(potencia,7,&dec,&sign);
  strcat(potLabel,string);
  lenpot=strlen(potLabel);
  pd1->insert(new TLabel(TRect(3,11,lenpot+3,12),potLabel,0));
  string = ecvt(roe,7,&dec,&sign);
  strcat(roeLabel,string);
  lenroe=strlen(roeLabel);
  pd1->insert(new TLabel(TRect(3,12,lenroe+3,13),roeLabel,0));
  pd1->insert(new TLabel(TRect(-1,14,57,15),"ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶",0));
  pd1->insert(new TRepButton(TRect(31,12,41,14),"~O~k",cmOK,bfDefault));
  pd1->insert(new TRepButton(TRect(43,12,53,14),"~C~ancel",cmCancel,bfNormal));
  return pd1;
}

void TMyApp::dialogCGC()
{
 TDialog *pd1;
 unsigned int acaba=0;
 do{
  pd1=new TDialog(TRect(1,1,76,22),"Configuraci¢ CGC");
  if (pd1)
  {
	 pd1=configCGC(pd1);
	 pd1->selectNext((Boolean)0);
	 pd1->setData(ConfigCGCBuf[posCh][posOper]);
	 ushort control=deskTop->execView(pd1);
	 if (control!=cmCancel)
	 {
	  pd1->getData(ConfigCGCBuf[posCh][posOper]);
	  error=controlDades(ConfigCGCBuf[posCh][posOper]);
	  if (!error) {acaba=1;error=0;}
	 }
	 else {acaba=1;error=0;}
  }
  destroy(pd1);
 }while (acaba==0);
}

unsigned int TMyApp::controlDades(ptrCGCData tempo)
{
 ushort dada;

 dada=tempo->chk1;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }
 dada=tempo->chk2;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }
 dada=tempo->chk3;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk4;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk5;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk6;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk7;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk8;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk9;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk10;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk11;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk12;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk13;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk14;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk15;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk16;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk17;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk18;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk19;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk20;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk21;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk22;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk23;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 dada=tempo->chk24;
 if(dada>=5 || dada==0)
 {
  messageBox("\03Error: Dades Incoherents.",mfOKButton);
  return 1;
 }

 return 0;
}

TDialog *TMyApp::configCGC(TDialog *pd1)
{
	 char temp[MAX_NUM_CANAL*9],temp2[3];
	 unsigned int iniX2=9,posX2,iniY2=4,posY2,posX1=2,posY1=3,longTemp,i,j;

	  if (mesCan==1) {mesCan=0;numChAct+=6;}
	  if (mesOper==1) {mesOper=0;numOperAct+=4;}

	  if (ini==1) {ini=0;numChAct=1;numOperAct=1;}

	  if (numChAct>canals) numChAct=1;
	  if (numOperAct>consoles) numOperAct=1;


	 posCh=(numChAct-1)/6;
	 posOper=(numOperAct-1)/4;
	 pd1->flags=pd1->flags&(~wfClose);
	 strcpy(temp,creaString(numChAct,canals));
	 longTemp=strlen(temp)+7;
	 pd1->insert(new TDStaticText(TRect(7,2,longTemp,3),temp));

	 for(i=numOperAct,posY2=iniY2;i<=consoles && i<=(numOperAct+3);i++,posY2+=iniY2)
	 {
		strcpy(temp,creaString2(canals));
		longTemp=strlen(temp)+posX1;
		pd1->insert(new TDStaticText(TRect(posX1,posY1,longTemp,posY1+1),temp));
		strcpy(temp,"OP.");
		itoa(i,temp2,10);
		if (i<10) {temp2[1]=' ';temp2[2]='\0';}
		strncat(temp,temp2,3);
		strcat(temp," ");
		pd1->insert(new TDStaticText(TRect(posX1,posY2+1,posX1+6,posY2+2),temp));
		for(j=numChAct,posX2=8;j<=canals && j<=(numChAct+5);j++,posX2+=iniX2)
		{
		 pd1->insert(new TCheckBoxes(TRect(posX2,posY2,posX2+8,posY2+3),
		 new TSItem("~R~x",
		 new TSItem("~T~x",
		 new TSItem("R~e~s",0) ))));
		}
	 }

	 if (canals>6)    pd1->insert(new configCGCButton(TRect(62,8,72,10),"C~a~nals",cmMCan,bfNormal));
	 if (consoles>4)  pd1->insert(new configCGCButton(TRect(62,11,72,13),"O~p~er.",cmMOper,bfNormal));
	 pd1->insert(new configCGCButton(TRect(62,17,72,19),"~O~k",cmOK,bfDefault));
	 pd1->insert(new configCGCButton(TRect(62,14,72,16),"~C~ancel",cmCancel,bfNormal));

	 return pd1;
}


char *TMyApp::creaString2(unsigned int numCanal)
{
 char destination[(MAX_NUM_CANAL*9)+6];
 unsigned int i=0;

 if (numCanal>MAX_NUM_CANAL)
 {
  printf("Nombre de canals massa gran...%d",MAX_NUM_CANAL);
  exit(1);
 }
 for(i=0;i<60;i++) destination[i]='Ä';
 destination[i]='\0';
 return destination;
}

char *TMyApp::creaString(unsigned int numChAct,unsigned int numCanal)
{
 char tempo[3];
 char destination[MAX_NUM_CANAL*9];
 char *source = " CANAL ";
 unsigned int i;

 if (numCanal>MAX_NUM_CANAL)
 {
  printf("Nombre de canals massa gran...%d",MAX_NUM_CANAL);
  exit(1);
 }
 for(i=0;i<(MAX_NUM_CANAL*9);i++) destination[i]='\0';

 for (i=numChAct;i<=numCanal && i<=(numChAct+5);i++)
 {
	strncat(destination, source, 7);
	itoa(i,tempo,10);
	if (i<10) {tempo[1]=' ';tempo[2]='\0';}
	strncat(destination,tempo,3);
 }
 return (destination);
}

TMenuBar *TMyApp::initMenuBar(TRect r)
{
 r.b.y=r.a.y+1;


 switch (tecnic)
 {
  case 1:
	 return new TMenuBar(r,
	 *new TSubMenu("~F~inestres",kbAltF)+
	 *new TMenuItem("~I~ncidŠncies",cmListServ,kbAltI,hcNoContext,"Alt-I")+
	 *new TMenuItem("~E~mergŠncies",cmListEmer,kbAltE,hcNoContext,"Alt-E")+
	 *new TMenuItem("C~a~nals",cmTrucCanal,kbAltA,hcNoContext,"Alt-A")+
	 *new TSubMenu("~T~rucades",kbAltT)+
	 *new TMenuItem("~T~rucada ~I~ndividual",cmFinVeu,kbF4,hcNoContext,"F4")+
	 newLine()+
	 *new TMenuItem("~T~rucada ~G~rup",cmTrucGrup,kbF5,hcNoContext,"F5")+
	 newLine()+
	 *new TMenuItem("~D~efinici¢ ~G~rup",cmDefGrup,kbF6,hcNoContext,"F6")+
	 *new TMenuItem("~A~nul.laci¢ ~G~rup",cmAnulGrup,kbF7,hcNoContext,"F7")+
	 *new TSubMenu("~C~onfiguraci¢",kbAltC)+
	 *new TMenuItem("~O~bertura per ...",cmObrir,kbAltO,hcNoContext,"Alt-O")+
	 newLine()+
	 *new TMenuItem("C~G~C",cmCGC,kbAltG,hcNoContext,"Alt-G")+
	 *new TMenuItem("~R~epetidors",cmRep1,kbAltR,hcNoContext,"Alt-R")+
	 *new TMenuItem("Equips ~M~•bils",cmEqMob,kbAltM,hcNoContext,"Alt-M")+
	 newLine()+
	 *new TMenuItem("Gra~v~ar ...",cmGrvConfig,kbAltV,hcNoContext,"Alt-V")+
	 *new TSubMenu("Sin•~p~tic",kbAltP)+
	 *new TMenuItem("Missa~t~ges de Veu",cmMsgVeu,kbAltT,hcNoContext,"Alt-T")+
	 *new TMenuItem("Tr…~f~ic dels M•bils",cmSinTrafic,kbAltF,hcNoContext,"Alt-F")+
	 *new TMenuItem("Nom~b~re de Trucades",cmTrucades,kbAltB,hcNoContext,"Alt-B")+
	 *new TSubMenu("~X~arxa",kbAltX)+
	 *new TMenuItem("~E~stat ~X~arxa",cmEstat,kbF9,hcNoContext,"F9")
	 );
  case 0:
	 return new TMenuBar(r,
	 *new TSubMenu("~T~rucades",kbAltT)+
	 *new TMenuItem("~T~rucada ~I~ndividual",cmFinVeu,kbF4,hcNoContext,"F4")+
	 newLine()+
	 *new TMenuItem("~T~rucada ~G~rup",cmTrucGrup,kbF5,hcNoContext,"F5")+
	 newLine()+
	 *new TMenuItem("~D~efinici¢ ~G~rup",cmDefGrup,kbF6,hcNoContext,"F6")+
	 *new TMenuItem("~A~nul.laci¢ ~G~rup",cmAnulGrup,kbF7,hcNoContext,"F7")
	 );
  default:break;
 }
}

TStatusLine *TMyApp::initStatusLine(TRect r)
{


 r.a.y= r.b.y -1;
 return new TStatusLine(r,
	*new TStatusDef(0,0xffff)+
	*new TStatusItem("~Alt-N~ Nit",kbAltN,cmNit)+
	*new TStatusItem("~F8~ Tr…fic",kbF8,cmTrafic)+
	*new TStatusItem("~F1~ Veu",kbF1,cmVeu)+
	*new TStatusItem("~Tx~",kbF2,cmPTT)
	);

/* PER DEMO
	*new TStatusItem("~F2~ Rebent Inci",kbF2,cmMsServeis)+
	*new TStatusItem("~F3~ Rebent SOS",kbF3,cmMsEmer)
	);*/
}



void TMyApp::handleEvent(TEvent &event)
{
 unsigned int j,FiExec=0;
 TEvent event2;

 TApplication::handleEvent(event);
 if (event.what==evKeyboard && event.message.command==kbAltS)
 {
  event2.what=evCommand;
  event2.message.command=cmQuit;
  putEvent(event2);
 }
 if (event.what==evCommand)
 {
  switch (event.message.command)
  {

	case cmFinVeu:
	case cmDefGrup:
	case cmRep1:
	 newDialog(event.message.command);
	 // s'envia a CGC per veure si el canal ‚s correcte
	 // en cas de ser-ho, el CGC retorna l'indicatiu amb asterisc i
	 // s'afegeix a Incid.
	 // Sin¢ el CGC envia un missatge d'error.
	 break;
	case cmObrir:
	 newDialog(event.message.command);
	 Tx2CGC(cmObrir,0,(*obrirData).info,"");
	 break;
	case cmAnulGrup:
	  extreuGrup();
	  break;
	case cmTrucGrup:
	 if (prefixesRx==0) Tx2CGC(cmRxPrefixes,0,0,"");
	 else newDialog(event.message.command);
	 prefixesRx=0;
	 break;
	case cmRep:
	  dialogRep2();
	  break;
	case cmCGC:
	  //  if (configRx==1)
	  dialogCGC();
	  //  else  Tx2CGC(cmRxConfig,0,0,"");
	 configRx=0;
	 break;
	case cmEqMob:
	 dialogEqMob();
	 break;
	case cmGrvConfig:
	 convert2Tx();
	 messageBox("\03Les Dades de Configuraci¢\n\03quedaran gravades en el CGC.",mfOKButton|mfInformation);
	 Tx2CGC(cmGrvConfig,canals*consoles,0,0);
	 break;
	case cmListEmer:
	 creaEmerBox();
	 FiExec=1;			  //*** A¤adido por Oscar **//
	 break;
	case cmListServ:
	 creaListBox();
	 FiExec=1;			  //*** A¤adido por Oscar **//
	 break;
	case cmTrucCanal:
	 creaCanalBox();
	 FiExec=1;			  //*** A¤adido por Oscar **//
	 break;
	case cmMsServeis:
	 if (inici==OFF) afegeixServeis(listRxCmnd[comndAct].info,listRxCmnd[comndAct].numProc,0);
	 FiExec=1;
	 break;
	case cmMsEmer:
	 if (inici==OFF) afegeixEmers(listRxCmnd[comndAct].info,listRxCmnd[comndAct].numProc,0);
	 FiExec=1;
	 break;
	case cmVeu:
	 gestioVeu(event);
//	 FiExec=1;
	 break;
	case cmPTT:
	 if (listRxCmnd[comndAct].numProc==0)
		disableCommand(cmPTT);
	 else
		enableCommand(cmPTT);
	 gestioVeu2();
	 FiExec=1;
	 break;
	case cmModfServeis:
	 if (inici==OFF)  modifServeis(listRxCmnd[comndAct].numProc);
	 FiExec=1;
	 break;
	case cmModfEmers:
	 if (inici==OFF)  modifEmers(listRxCmnd[comndAct].numProc);
	 FiExec=1;
	 break;
	case cmExtrEmers:
	 if (inici==OFF)  extreuEmers(1);
	 FiExec=1;
	 break;
	case cmExtrServeis:
	 if (inici==OFF)  extreuServeis(1);
	 FiExec=1;
	 break;
	case cmERROR:
		 sound (600);
		 delay (100);
		 nosound();
		 messageBox((const char *)listError[atoi(listRxCmnd[comndAct].info)],mfError|mfOKButton);
		 FiExec=1;
		 break;
	case cmRxPrefixes:
		 getStringTrucsGrup();
		 FiExec=1 ;
		 break;
	case cmEstat:
		 if (RxEstat==0)
		 {
		  Tx2CGC(cmEstat,0,0,"");
		  getAnswerCGC(cmEstat);
		 }
		 else
		 {
		  newDialog(event.message.command);
		  FiExec=1;
		  RxEstat=0;
		 }
		 break;
	case cmRxConfig:
		 getCGCConfig();
		 FiExec=1;
		 break;
	case cmIniNit:
		 gestioNit();
		 FiExec=1;
		 break;
	case cmNit:
		 if (Nit==0) disableCommand(cmNit);
		 Tx2CGC(cmNit,0,Nit,"");
		 break;
	case cmSqlOn:
	case cmSqlOff:
		 modifCanals(event.message.command);
		 FiExec=1;
		 break;
	case cmCanviObrir:
		 gestioCanviObrir();
		 if (listRxCmnd[comndAct].comnd==cmCanviObrir) FiExec=1;
		 break;
	case cmTrafic:
			 //	 traficWinFunc();
		 break;
	case cmMsTrafic:
			//	 afegeixTrafic();
		 FiExec=1;
		 break;
	default:
		messageBox("\03**** ERROR 104 **** comando interno no reconocido,\n\03 pulse <RETURN> ",mfOKButton|mfInformation);
		FiExec=1;
	break;
  }
  clearEvent(event);

  /* RESET COMANDES DE CGC UN COP GESTIONADES */
  if (FiExec==1)
  {
	FiExec=0;
	for(j=comndAct;j<lastCmnd-1 && j<MAX_NUM_COMND-1;j++)
	{
	 listRxCmnd[j].comnd=listRxCmnd[j+1].comnd;
	 listRxCmnd[j].numProc=listRxCmnd[j+1].numProc;
	 strcpy(listRxCmnd[j].info,listRxCmnd[j+1].info);
	}
	if (lastCmnd>0 && lastCmnd<=(MAX_NUM_COMND-1)) lastCmnd--;

	listRxCmnd[lastCmnd].comnd=cmRES;
	listRxCmnd[lastCmnd].numProc=0;
	memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
  }
 }
}

void TMyApp::afegeixTrafic(void)
{
 unsigned int i;
 TEvent event;

 if (posWrite!=iniLineAnt)
 {
  memset(traficBuffer[posWrite],' ',MAX_LENGTHLINE);
  strcpy(traficBuffer[posWrite],listRxCmnd[comndAct].info);
 }
 else
 {
  strcpy(lineAnt,listRxCmnd[comndAct].info);
 }

 if (posWrite>=MAX_NUMLINES-1)
 // corre amunt els missatges
 {
  for(i=1;i<MAX_NUMLINES-1;i++)
  {
	if ((i+1)!=iniLineAnt)
	{
	 memset(traficBuffer[i],' ',MAX_LENGTHLINE);
	 strcpy(traficBuffer[i],traficBuffer[i+1]);
	}
	else
	{
	 strcpy(traficBuffer[i],lineAnt);
	 strcpy(lineAnt,traficBuffer[iniLineAnt+1]);
	 i=iniLineAnt;
	}
  }
  memset(traficBuffer[i],' ',MAX_LENGTHLINE);
 }

 if (posWrite<MAX_NUMLINES-1) posWrite++;

 if (vScrollBar->value<(MAX_NUMLINES-size.y))
 {
  event.what=evKeyboard;
  event.message.command=kbDown;
  putEvent(event);
 }
 else
 {
  event.what=evKeyboard;
  event.message.command=kbUp;
  putEvent(event);
  event.what=evKeyboard;
  event.message.command=kbDown;
  putEvent(event);
 }

}


void TMyApp::traficWinFunc(void)
{
 traficWin=new TTraficWindow(TRect(0,0,22,23-(ACT_CH+2)),"Tr…f.CGC",4);
 if (traficWin)
 {
  deskTop->insert(traficWin);
  disableCommand(cmTrafic);
 }

}

void TMyApp::gestioCanviObrir(void)
{
 unsigned int i;
 if (trucCanalFin==1 && (*obrirData).info==1) // canals creada i stma tancat
 {
  trucCanalFin=0;
  if (tecnic==0) enableCommand(cmTrucCanal);// a l'operador li desapareix la finestra
  else disableCommand(cmTrucCanal); // tecnic haur… de tornar activar per Sql
  for(i=0;i<ACT_CH && i<MAX_NUM_CANAL && stringTrucCanal[i]!=NULL;i++)
   delete(stringTrucCanal[i]);

  destroy(trucCanalBox);  // MIRAR SI CAL o amb la segent instruc es destrueix tot
  destroy(trucCanalDialog);
 }
 else
  if (trucCanalFin==0 && (*obrirData).info==0) // canals no creada i stma obert
  {
	  enableCommand(cmTrucCanal);
	  if (tecnic==0)
	  {
	   TEvent event;
	   event.what=evCommand;
	   event.message.command=cmTrucCanal;
		putEvent(event);
	  }
  }
  else
    if (trucCanalFin==0 && (*obrirData).info==1 && tecnic==1) disableCommand(cmTrucCanal);
}

void TMyApp::getCGCConfig(void)
{
 unsigned int i,j;
 for(i=0;i<canals;i++)
  for(j=0;j<consoles;j++)
  {

  }
}

void TMyApp::gestioNit(void)
{
// la comanda ve del CGC
 if (listRxCmnd[comndAct].numProc==1 && Nit==0)
 {
	messageBox("\03El CGC ha entrat en estat de NIT.\n\03S'espera que l'operador de nit premi Alt-N.",mfOKButton|mfInformation);
	enableCommand(cmNit);
	Nit=1;
 }
 else
  if (Nit==1 && listRxCmnd[comndAct].numProc==0)
  {
	messageBox("\03El CGC ja no es troba en estat\n\03de NIT.S'espera que l'operador de nit premi\n\03Alt-N en acabar les Incid.",mfOKButton|mfInformation);
	Nit=0;
  }
}

void TMyApp::getStringTrucsGrup(void)
{
 char dest[LONG_GRUP]="";
 unsigned int length=0,i=0,algun=0,j=0;
 TEvent event;

 length=strlen(listRxCmnd[comndAct].info);


 for(i=0,j=0;i<length;i++)
 {
	if (listRxCmnd[comndAct].info[i]!=0x0a)
	{
	 if (j<LONG_GRUP) dest[j++]=listRxCmnd[comndAct].info[i];
	 algun=1;
	}

	if (algun==1 && listRxCmnd[comndAct].info[i]==0x0a)
	{
		algun=0;
		prefixesRx=1;
		afegeixTrucGrup(dest);
		strcpy(dest,"");
		j=0;
	}
 }

 if (prefixesRx==1)
 {
		event.what=evCommand;
		event.message.command=cmTrucGrup;
		putEvent(event);
 }
}

void TMyApp::extreuEmers(unsigned int directe)
{
 unsigned int posFoc,j,trobat=0,treu;
 char *pp;

 if (directe==0) // extreu emer degut a anul.laci¢ de veu
 {
	 posFoc=numEmer+(llistaEmers->focus)+1;
	 treu=llistaEmers->focus;
	 trobat=1;
 }
 else // extreu emer degut a comanda de CGC
 {    // si numEmer==MAX_EMER no hi han emergŠncies trobat=0
  for(j=numEmer+1;j<=MAX_EMER && trobat==0 && stringEmers[j].indicatiu!=NULL;j++)
  {
	if ((!(strcmp(listRxCmnd[comndAct].info,stringEmers[j].indicatiu)))
		 && listRxCmnd[comndAct].numProc==stringEmers[j].numProc)
	{
	 trobat=1;
	 treu=j-numEmer-1;
	 posFoc=j;
	}
  }

 }

  for(j=posFoc;j>numEmer && trobat==1;j--)
  {
	stringEmers[j].indicatiu=stringEmers[j-1].indicatiu;
	stringEmers[j].numProc=stringEmers[j-1].numProc;
  }


 if(trobat==1 && (tecnic==0 || (tecnic==1 && !(commandEnabled(cmListEmer)))))
 {
  destroy(emersBox);
  llistaEmers->items->atFree((ccIndex)treu);
  numEmer++;
  if ((posFoc==MAX_EMER) && (numEmer<MAX_EMER) && directe==0)
	llistaEmers->focus--;
  emersBox=new TListBox3(TRect(1,1,21,19),1,0);

  emersBox->setData(llistaEmers);
  emersListBox->insert(emersBox);
 }
}

void TMyApp::modifServeis(unsigned int num)
{
  unsigned int i,trobat=0;
  for(i=0;i<numServeis && trobat==0 && stringIndics[i].indicatiu!=NULL;i++)
  {
	if (stringIndics[i].numProc==num && (!strcmp(stringIndics[i].indicatiu,listRxCmnd[comndAct].info)))
	{
	  strcat(listRxCmnd[comndAct].info," *");
	  strcpy(stringIndics[i].indicatiu,listRxCmnd[comndAct].info);
	  llistaIndics->items->atPut(i,stringIndics[i].indicatiu);
	  if (trucCanalFin==1)
	  {
		destroy(indicsBox);
		indicsBox=new TListBox2(TRect(1,1,21,19),1,0);
		indicsBox->setData(llistaIndics);
		serveisListBox->insert(indicsBox);
	  }
	  trobat=1;
	}
  }
}

void TMyApp::modifCanals(unsigned int sqlOn)
{
  unsigned int i,trobat=0,focus=0;
  char tempo[MAX_LONG_INFO]="";

  if (!(strcmp(listRxCmnd[comndAct].info," Canal 3")))
	  strcpy(listRxCmnd[comndAct].info," Canal 7");
  if (!(strcmp(listRxCmnd[comndAct].info," Canal 4")))
	  strcpy(listRxCmnd[comndAct].info," Canal G");

  for(i=0;i<ACT_CH && trobat==0 && i<MAX_NUM_CANAL && (*obrirData).info==0 && stringTrucCanal[i]!=NULL;i++)
  {
	if ((!strcmp(stringTrucCanal[i],listRxCmnd[comndAct].info)) && sqlOn==cmSqlOn)
	{
	  strcat(listRxCmnd[comndAct].info," *");
	  strcpy(stringTrucCanal[i],listRxCmnd[comndAct].info);
	  trucCanalData->items->atPut(i,stringTrucCanal[i]);
	  destroy(trucCanalBox);
	  trucCanalBox=new TListBox6(TRect(1,1,21,ACT_CH+1),1,0);
	  trucCanalBox->setData(trucCanalData);
	  trucCanalDialog->insert(trucCanalBox);
	  trobat=1;
	}
	else
	{
	 strcpy(tempo,listRxCmnd[comndAct].info);
	 strcat(tempo," *");
	 if ((!strcmp(stringTrucCanal[i],tempo)) && sqlOn==cmSqlOff)
	 {
	  tempo[strlen(tempo)-2]='\0';
	  strcpy(stringTrucCanal[i],tempo);
	  trucCanalData->items->atPut(i,stringTrucCanal[i]);
	  destroy(trucCanalBox);
	  trucCanalBox=new TListBox6(TRect(1,1,21,ACT_CH+1),1,0);
	  trucCanalBox->setData(trucCanalData);
	  trucCanalDialog->insert(trucCanalBox);
	  trobat=1;
	 }
	 strcpy(tempo,"");
	}
  }
}

void TMyApp::modifEmers(unsigned int num)
{
 unsigned int i,trobat=0;
 for(i=numEmer+1;i<=MAX_EMER && trobat==0 && stringEmers[i].indicatiu!=NULL;i++)
 {
  if (stringEmers[i].numProc==num && (!(strcmp(stringEmers[i].indicatiu,listRxCmnd[comndAct].info))))
  {
	  strcat(listRxCmnd[comndAct].info," *");
	  strcpy(stringEmers[i].indicatiu,listRxCmnd[comndAct].info);
	  llistaEmers->items->atPut(i-numEmer-1,stringEmers[i].indicatiu);
	  destroy(emersBox);
	  emersBox=new TListBox3(TRect(1,1,21,19),1,0);
	  emersBox->setData(llistaEmers);
	  emersListBox->insert(emersBox);
	  trobat=1;
  }
 }

}


void TMyApp::extreuServeis(unsigned int directe)
{
 unsigned int posFoc,i,trobat=0;

 if (directe==0)
 {
	 posFoc=llistaIndics->focus;
	 trobat=1;
 }
 else
 {
  for(i=0;i<numServeis && trobat==0;i++)
	if (listRxCmnd[comndAct].numProc==stringIndics[i].numProc
		 && (!(strcmp(listRxCmnd[comndAct].info,stringIndics[i].indicatiu))))
	{
	 trobat=1;
	 posFoc=i;
	}
 }

 for(i=posFoc;i<numServeis && trobat==1;i++)
 {
	stringIndics[i].indicatiu=stringIndics[i+1].indicatiu;
	stringIndics[i].numProc=stringIndics[i+1].numProc;
 }


// MIRAR EN CAS TECNIC, CAL DESTRUIR_ CREAR PERO NO INSERIR
 if(trobat==1 && (tecnic==0 || (tecnic==1 && !(commandEnabled(cmListServ)))))
 {
	 destroy(indicsBox);

	 llistaIndics->items->atFree(posFoc);
	 if ((posFoc==numServeis-1) && (posFoc>0) && directe==0)
		llistaIndics->focus--;
	 numServeis--;
	 indicsBox=new TListBox2(TRect(1,1,21,19),1,0);
	 indicsBox->setData(llistaIndics);
	 serveisListBox->insert(indicsBox);
 }
}

void TMyApp::extreuTrucGrup()
{
 unsigned int posFoc,i,trobat=0;

 for(i=0;i<numGrups;i++)
	 llistaIndics->items->atFree(i);

 destroy(indicsBox);
 trucGrupBox=new TListBox5(TRect(3,5,16,12),1,0);
 trucData->focus=0;
 trucData2->focus=0;
 numGrups=0;

 for(i=0;i<numGrups && stringTrucsGrup[i]!=NULL;i++)
  delete(stringTrucsGrup[i]);
}

void TMyApp::extreuGrup()
{
 TDialog *pd1;
 unsigned int control;
 TEvent event2;

 pd1=new TDialog(TRect(3,6,34,15),"Anul.laci¢ Grup");
 if (pd1)
 {
	 pd1->flags=pd1->flags&(~wfClose);
	 pd1->insert(new TInputLine(TRect(8,2,27,3),LONG_GRUP));
	 pd1->insert(new TLabel(TRect(2,2,7,3),"~G~rup",0));
	 pd1->insert(new TButton(TRect(4,5,14,7),"~O~k",cmOK,bfDefault));
	 pd1->insert(new TButton(TRect(16,5,26,7),"~C~ancel",cmCancel,bfNormal));

	 pd1->selectNext((Boolean)0);
	 pd1->setData(anulData);
	 control=deskTop->execView(pd1);
	 if (control!=cmCancel)
	 {
		pd1->getData(anulData);
		if (anulData->grup[0]=='\x0' && anulData->grup[1]>='0' && anulData->grup[1]<='9') anulData->grup[0]='0';
		if (anulData->grup[1]=='\x0' && anulData->grup[0]>='0' && anulData->grup[0]<='9') {anulData->grup[1]=anulData->grup[0]; anulData->grup[0]='0';}
		if ((anulData->grup[0]<'0' || anulData->grup[0]>'9') || (anulData->grup[1]<'0' || anulData->grup[1]>'9') || atoi(anulData->grup)==0)
		{
		 messageBox("\03Error: nombre de grup incorrecte.",mfOKButton|mfError);
		 event2.what=evCommand;
		 event2.message.command=cmAnulGrup;
		 putEvent(event2);
		}
		else
		 Tx2CGC(cmAnulGrup,strlen(anulData->grup),0,anulData->grup);
	 }
 }
 destroy(pd1);
}

void TMyApp::afegeixServeis(char *tempo,unsigned int numProc,unsigned int demo)
{
 unsigned int i,trobat=0;
 if (tempo!='\0' && (strlen(tempo)<LONG_INDIC+2))
 {
  for(i=0;i<numServeis && trobat==0 &&  stringIndics[i].indicatiu!=NULL && i<MAX_SERVEIS;i++)
  {
	if (!(strcmp(stringIndics[i].indicatiu,tempo)))
	 trobat=1;
  }

  if (numServeis<MAX_SERVEIS && trobat==0)
  {
	stringIndics[numServeis].indicatiu=new char[LONG_INDIC+2];
	if (demo==1)
		tempo=entradaDades(tempo);

	strcpy(stringIndics[numServeis].indicatiu,tempo);
	stringIndics[numServeis].numProc=numProc;

	llistaIndics->items->atInsert(numServeis,stringIndics[numServeis].indicatiu);
	numServeis++;

	if (tecnic==0 || (tecnic==1 && !(commandEnabled(cmListServ))))
	{
	 destroy(indicsBox);
	 indicsBox=new TListBox2(TRect(1,1,21,19),1,0);
	 indicsBox->setData(llistaIndics);
	 serveisListBox->insert(indicsBox);
	}

  }
  else
	if (trobat==0)
	 messageBox("\03El nombre de serveis no atesos\n\03ha arribat al m…xim.",mfOKButton|mfInformation);
 }
}


void TMyApp::afegeixTrucGrup(char *tempo)
{
 if (tempo!='\0' && strlen(tempo)<LONG_GRUP)
 {
  if (numGrups<MAX_GRUP)
  {
	stringTrucsGrup[numGrups]=new char[LONG_GRUP];
	strcpy(stringTrucsGrup[numGrups],tempo);
	trucData->items->atInsert(numGrups,stringTrucsGrup[numGrups]);
	trucData2->items->atInsert(numGrups,stringTrucsGrup[numGrups]);

	numGrups++;
  }
  else
	messageBox("\03El nombre de grups definits\n\03ha arribat al m…xim.",mfOKButton|mfInformation);
 }
}

void TMyApp::afegeixRep(char *tempo)
{
 if (tempo!='\0' && strlen(tempo)<LONG_REP)
 {
  if (numReps<MAX_REP)
  {
	stringRep[numReps]=new char[LONG_REP];
	strcpy(stringRep[numReps],tempo);
	repData1->items->atInsert(numReps,stringRep[numReps]);
	numReps++;
  }
  else
	messageBox("\03El nombre de repetidors\n\03ha arribat al m…xim.",mfOKButton|mfInformation);
 }
}

void TMyApp::afegeixEmers(char *tempo,unsigned int numProc,unsigned int demo)
{
 unsigned int i,trobat=0;
 char tempo2[MAX_LONG_INFO+2]="";

 strcpy(tempo2,tempo);
 strcat(tempo2," *");

 if (tempo!=NULL && strlen(tempo)<(LONG_INDIC+2))
 {
  for(i=MAX_EMER;i>numEmer && trobat==0 &&  stringEmers[i].indicatiu!=NULL && i>0;i--)
  {
	if ((!(strcmp(stringEmers[i].indicatiu,tempo))) || (!(strcmp(stringEmers[i].indicatiu,tempo2))))
	 trobat=1;
  }


  if (numEmer>0 && trobat==0)
  {
	if(demo==1)
	 tempo=entradaDades(tempo);

	stringEmers[numEmer].indicatiu=new char[LONG_INDIC+2];

	strcpy(stringEmers[numEmer].indicatiu,tempo);
	stringEmers[numEmer].numProc=numProc;

	llistaEmers->items->atInsert(0,stringEmers[numEmer].indicatiu);
	numEmer--;

	if ((tecnic==0) || (tecnic==1 && !(commandEnabled(cmListEmer))))
	{
	  destroy(emersBox);
	  emersBox=new TListBox3(TRect(1,1,21,19),1,0);
	  emersBox->setData(llistaEmers);
	  emersListBox->insert(emersBox);
	  sound(700);
	  delay(100);
	  nosound();
	  delay(200);
	  sound(700);
	  delay(100);
	  nosound();
	}
  }
  else
	if (trobat==0)
	 messageBox("\03El nombre d'emergŠncies no ateses\n\03ha arribat al m…xim.",mfOKButton|mfInformation);
 }
}

void TInterior::handleEvent(TEvent& event)
{
 short posVert=vScrollBar->minVal,posHorit=hScrollBar->minVal;
 static unsigned int teclat=0;

 // MOUSE
 if (event.what==evBroadcast && event.message.command==cmScrollBarChanged)
 {
  if (teclat==0)
  {
	posVert=vScrollBar->value;
	if (size.y<MAX_NUMLINES)
	{
	 strcpy(traficBuffer[iniLineAnt],lineAnt);
	 iniLine=posVert;
	 iniLineAnt=iniLine;
	 strcpy(lineAnt,traficBuffer[iniLine]);
	 strcpy(traficBuffer[iniLine]," Comanda  Flota  Indicatiu  Origen  Altres");
	}
	posHorit=hScrollBar->value;
	if (size.x<MAX_LENGTHLINE) posLine=posHorit;
  }
  else teclat=0;
 }


 TScroller::handleEvent(event);
 // TECLAT
 if (event.what==evKeyboard)
 {
  teclat=1;
  switch (event.message.command)
  {
	case kbDown:  if (iniLine<(MAX_NUMLINES-size.y))
					  {
						 strcpy(traficBuffer[iniLineAnt],lineAnt);
						 iniLine++;
						 iniLineAnt=iniLine;
						 strcpy(lineAnt,traficBuffer[iniLine]);
						 strcpy(traficBuffer[iniLine]," Comanda  Flota  Indicatiu  Origen  Altres");
					  }
	 break;
	case kbUp:    if (iniLine>0)
					  {
						 strcpy(traficBuffer[iniLineAnt],lineAnt);
						 iniLine--;
						 iniLineAnt=iniLine;
						 strcpy(lineAnt,traficBuffer[iniLine]);
						 strcpy(traficBuffer[iniLine]," Comanda  Flota  Indicatiu  Origen  Altres");
					  }
	 break;
	case kbRight: if (posLine<(MAX_LENGTHLINE-size.x)) posLine++;
	 break;
	case kbLeft:  if (posLine>0) posLine--;
	 break;
	case kbPgDn:
		 strcpy(traficBuffer[iniLineAnt],lineAnt);
		 if ((iniLine+11)<(MAX_NUMLINES-size.y)) iniLine+=11;
		 else iniLine=MAX_NUMLINES-size.y;
		 iniLineAnt=iniLine;
		 strcpy(lineAnt,traficBuffer[iniLine]);
		 strcpy(traficBuffer[iniLine]," Comanda  Flota  Indicatiu  Origen  Altres");
	 break;
	case kbPgUp:
		 strcpy(traficBuffer[iniLineAnt],lineAnt);
		 if (iniLine>=11) iniLine-=11;
		 else iniLine=0;
		 iniLineAnt=iniLine;
		 strcpy(lineAnt,traficBuffer[iniLine]);
		 strcpy(traficBuffer[iniLine]," Comanda  Flota  Indicatiu  Origen  Altres");
	 break;
	case kbHome:  posLine=0;
	 break;
	case kbEnd:   posLine=MAX_LENGTHLINE-size.x;
	 break;
	default:;
  }
 }
}


void TInterior::draw()
{
 TDrawBuffer b;
 unsigned int p,i,j,k;
 ushort color=getColor(0x0302);


 for(i=0,p=iniLine;i<size.y;i++,p++)
 {
  b.moveChar(0,' ',color,size.x);
  if (traficBuffer[p] && p<MAX_NUMLINES)
  {
	char s[MAX_LENGTHLINE];
	for (k=0,j=posLine;k<size.x && j<MAX_LENGTHLINE;j++,k++)
	 s[k]=traficBuffer[p][j];
	s[k]=EOS;
	b.moveStr(0,s,color);
  }
  writeLine(0,i,size.x,1,b);
 }
}


void TMyApp::creaListBox()
{
	TRect r6(0,0,22,20);
	r6.move(58,3);
	serveisListBox=new TListWindow(r6,"Incid.",1);
	serveisListBox->setState(sfShadow,(Boolean)0);
	indicsBox=new TListBox2(TRect(1,1,21,19),1,0);
	indicsBox->setData(llistaIndics);
	serveisListBox->insert(indicsBox);
	if (serveisListBox)
	 deskTop->insert(serveisListBox);
	disableCommand(cmListServ);
	trucGrupBox=new TListBox5(TRect(3,5,16,12),1,0);
}

void TMyApp::creaCanalBox()
{
  unsigned int i;
  char dest[LONG_CANAL]=" Canal ";
  char dest2[3]="";


  TRect r6(0,0,22,ACT_CH+2);
  r6.move(0,23-(ACT_CH+2));
  trucCanalDialog=new TListWindow3(r6,"Canals",3);
  trucCanalDialog->setState(sfShadow,(Boolean)0);
  trucCanalBox=new TListBox6(TRect(1,1,21,ACT_CH+1),1,0);

  for(i=0;i<ACT_CH && i<MAX_NUM_CANAL;i++)
  {
	stringTrucCanal[i]=new char[LONG_CANAL];
	if (i==2)
	 strcat(dest,"7");
	else
	{
	 if (i==3)
	  strcat(dest,"G");
	 else
	 {
	  itoa(i+1,dest2,10);
	  strcat(dest,dest2);
	 }
	}
	strcpy(stringTrucCanal[i],dest);
	trucCanalData->items->atInsert(i,stringTrucCanal[i]);
	strcpy(dest2,"");
	strcpy(dest," Canal ");
  }

  trucCanalData->focus=0;
  trucCanalBox->setData(trucCanalData);
  trucCanalDialog->insert(trucCanalBox);
  deskTop->insert(trucCanalDialog);
  trucCanalFin=1;
  disableCommand(cmTrucCanal);
}

void TMyApp::creaEmerBox()
{
	TRect r6(0,0,22,20);
	r6.move(36,3);
	emersListBox=new TListWindow2(r6,"S.O.S.",2);
	emersListBox->setState(sfShadow,(Boolean)0);
	emersBox=new TListBox3(TRect(1,1,21,19),1,0);
	emersBox->setData(llistaEmers);
	emersListBox->insert(emersBox);
	if (emersListBox)
	 deskTop->insert(emersListBox);
	disableCommand(cmListEmer);
}





void TListBox5::handleEvent(TEvent& event)
{
 char dest[LONG_GRUP]="";
 TListBox::handleEvent(event);
 TListBox::drawView();
 if (event.what==evCommand)
 {
  switch (event.message.command)
  {
	case cmPTT:
		if (listRxCmnd[comndAct].numProc==0) disableCommand(cmPTT);
		else enableCommand(cmPTT);
		trucGrupBox->getData(trucData2);
		strcpy(dest,stringTrucsGrup[trucData2->focus]);
		Tx2CGC(cmTrucGrup,strlen(dest),listRxCmnd[comndAct].numProc,dest);
		break;
	default:;
  }
  clearEvent(event);
 }
}


void configCGCButton::handleEvent(TEvent &event)
{

 TButton::handleEvent(event);
 if (event.what==evCommand)
 {
  switch (event.message.command)
  {
	case cmMCan:
	  mesCan=1;
	  event.what=evCommand;
	  event.message.command=cmCGC;
	  putEvent(event);
	  endModal(cmMCan);
	break;
	case cmMOper:
	 mesOper=1;
	 event.what=evCommand;
	 event.message.command=cmCGC;
	 putEvent(event);
	 endModal(cmMOper);
	 break;
	case cmOK:
	 endModal(cmOK);
	 break;
	case cmCancel:
	 mesOper=0;mesCan=0;ini=1;
	 endModal(cmCancel);
	 break;
  }
  clearEvent(event);
 }
}


void TMyApp::getAnswerCGC(int proc)
{
 unsigned long time=0;
 TDialog *pd1;
 TButton *pd2;

 TRect r6(0,0,39,12);
 r6.move(20,5);
 pd1=new TDialog(r6,"Information");

 delay(700);

 if (CGCOn==OFF && pd1)
 {
	 pd1->insert(new TDStaticText(TRect(2,2,38,3),"\03El CGC es troba gestionant"));
	 pd1->insert(new TDStaticText(TRect(2,4,38,5),"\03la seva comanda ..."));

	 deskTop->insert(pd1);
 }

 for(;CGCOn==OFF && time<(TIMEOUT*2);time++);

 if (CGCOn==OFF)
 {
	pd1->insert(new TDStaticText(TRect(2,6,38,7),"\03El CGC no respon."));
	pd2=new TButton(TRect(14,8,24,10),"~O~k",cmOK,bfDefault);
	pd1->insert(pd2);
	while(!kbhit());
 }
 else
 {
  if (proc==cmEstat) RxEstat=1;
 }
 CGCOn=OFF;
 if (pd1) destroy(pd1);
}


void TMyApp::idle()
{
 unsigned int j;
 TEvent event;

 TProgram::idle();
	/* ACTIVA INTERRUPTS QUAN TOT S'HA CREAT */
	outp(rcm2,inp(rcm2) & 0xfe);      /* DSR=0 operador lliure  */
	outp(rci2,0x09); // activa int de recepci¢n
	enable();



	if(listRxCmnd[comndAct].comnd!=cmRES)
	{
		event.what=evCommand;
		event.message.command=listRxCmnd[comndAct].comnd;
		putEvent(event);
	}

}

void password(void)
{
  char pwd[10];
  clrscr();
  printf("Password: ");
  for(int i=0;i<9 && pwd[i-1]!=13;i++)
  pwd[i]=getch();
  pwd[i-1]='\0';
  if((strcmp(pwd,"tecnic"))==0) tecnic=1;
// podem fer que tecnic estigui encriptat en disc i una funcio que canvii de password
  else tecnic=0;
  clrscr();
}


void initConfigCGC(void)
{
 unsigned int i,j,reste1=0,reste2=0;

 if (canals>MAX_NUM_CANAL || consoles>MAX_NUM_OPER)
 {
  printf("Error nombre canals o consoles, m‚s gran que max.");
  exit(1);
 }


 if (canals>6)
 {
	nTChPan=(unsigned int)(canals/6);
	reste1=canals%6;
	if (reste1!=0) nTChPan++;
 }


 if (consoles>4)
 {
	nTOperPan=((unsigned int)(consoles/4));
	reste2=consoles%4;
	if (reste2!=0) nTOperPan++;
 }


 for(i=0;i<nTChPan;i++)
 {
  for(j=0;j<nTOperPan;j++)
  {
	ConfigCGCBuf[i][j]=new ConfigCGCData[1];
	ConfigCGCBuf[i][j]->chk1=4;
	ConfigCGCBuf[i][j]->chk2=4;
	ConfigCGCBuf[i][j]->chk3=4;
	ConfigCGCBuf[i][j]->chk4=4;
	ConfigCGCBuf[i][j]->chk5=4;
	ConfigCGCBuf[i][j]->chk6=4;
	ConfigCGCBuf[i][j]->chk7=4;
	ConfigCGCBuf[i][j]->chk8=4;
	ConfigCGCBuf[i][j]->chk9=4;
	ConfigCGCBuf[i][j]->chk10=4;
	ConfigCGCBuf[i][j]->chk11=4;
	ConfigCGCBuf[i][j]->chk12=4;
	ConfigCGCBuf[i][j]->chk13=4;
	ConfigCGCBuf[i][j]->chk14=4;
	ConfigCGCBuf[i][j]->chk15=4;
	ConfigCGCBuf[i][j]->chk16=4;
	ConfigCGCBuf[i][j]->chk17=4;
	ConfigCGCBuf[i][j]->chk18=4;
	ConfigCGCBuf[i][j]->chk19=4;
	ConfigCGCBuf[i][j]->chk20=4;
	ConfigCGCBuf[i][j]->chk21=4;
	ConfigCGCBuf[i][j]->chk22=4;
	ConfigCGCBuf[i][j]->chk23=4;
	ConfigCGCBuf[i][j]->chk24=4;
  }
 }
}

void initRep()
{

 repData=new ConfigRepData[1];
 strcpy(repData->repetidor,"");
 strcpy(repData->subtoTx,"");
 strcpy(repData->subtoRx,"");
 repData->releStat=0;
 potencia=0;
 roe=0;


}
void initEqMob(void)
{
 eqMobData=new ConfigEqMobData[1];
 strcpy(eqMobData->indicatiu,"011");
 strcpy(eqMobData->indicNou,"010");
 eqMobData->estatus=3;
 numSerie=10100;
}

void initDefData()
{
 char tempo[LONG_GRUP];

 strcpy(defData->grup,itoa(numGrups+1,tempo,10));
 strcpy(defData->ind1,"");
 strcpy(defData->ind2,"");
 strcpy(defData->ind3,"");
 strcpy(defData->ind4,"");
 strcpy(defData->ind5,"");
 strcpy(defData->ind6,"");
 strcpy(defData->ind7,"");
 strcpy(defData->ind8,"");

}

void lliberaMemo(void)
{
 unsigned int i=0;
 for (i=0;i<MAX_SERVEIS;i++)
  if (stringIndics[i].indicatiu!=NULL) free(stringIndics[i].indicatiu);

 free(stringIndics);

 for (i=0;i<MAX_EMER+1;i++)
  if (stringEmers[i].indicatiu!=NULL) free(stringEmers[i].indicatiu);
 free(stringEmers);


 for(i=0;i<5;i++)
  if (listError[i]!=NULL) free(listError[i]);
 free(listError);


 for(i=0;i<MAX_NUM_COMND;i++)
  if (listRxCmnd[i].info!=NULL) free(listRxCmnd[i].info);
 free(listRxCmnd);
}

void initListIndic()
{
 unsigned int i=0;

 if((stringIndics=(ptrIndics)malloc(sizeof(indics)*MAX_SERVEIS))==NULL)
	exit(1);

 if((stringEmers=(ptrIndics)malloc(sizeof(indics)*(MAX_EMER+1)))==NULL)
	exit(1);

 for(i=0;i<MAX_EMER+1;i++)
 {
  stringEmers[i].indicatiu=NULL;
  stringEmers[i].numProc=0;
 }
 numEmer=MAX_EMER;

 for(i=0;i<MAX_SERVEIS;i++)
 {
  stringIndics[i].indicatiu=NULL;
  stringIndics[i].numProc=0;
 }

 if((listError=(char **)malloc(sizeof(char *)*5))==NULL)
	exit(1);


 listError[0]=strdup("\03Canal de transmissi¢ no permŠs a l'operador o indicatiu no donat d'alta.");
 listError[1]=strdup("\03Algun dels indicatius del grup es troba en canal no permŠs a l'operador.");
 listError[2]=strdup("\03No hi ha cap grup definit.");

 listError[3]=strdup("\03Anul.lar grup: operaci¢ no realitzada.");
 listError[4]=strdup("\03Definici¢ grup: operaci¢ no realitzada.");

 listError[5]=strdup("\03Trucada grup: operaci¢ no realitzada.");
 listError[6]=strdup("\03Grup: No s'ha pogut comunicar amb algun dels indicatius.");


 if((listRxCmnd=(ptrRxInfo)malloc(sizeof(RxInfo)*MAX_NUM_COMND))==NULL)
	exit(1);

 if((obrirData=(ptrObrirData)malloc(sizeof(ObrirInfo)))==NULL)
	exit(1);

 if((estatData=(ptrEstatData)malloc(sizeof(EstatData)))==NULL)
	exit(1);


 (*obrirData).info=1;

  if((trucCanalData=(ptrTrucCanalData)malloc(sizeof(trucGrupData2)))==NULL)
	exit(1);
  trucCanalData->items=new TCollection2((ccIndex)10,(ccIndex)10);
  trucCanalData->focus=0;


 for(i=0;i<MAX_NUM_COMND;i++)
 {
  listRxCmnd[i].comnd=cmRES;
  if((listRxCmnd[i].info=(char *)malloc(sizeof(char)*MAX_LONG_INFO))==NULL)
	exit(1);
  listRxCmnd[i].numProc=0;
  strcpy(listRxCmnd[i].info,"");
 }


 lastCmnd=0;
 comndAct=0;


 if((llistaIndics=(ptrIndicsData)malloc(sizeof(listIndicsData)))==NULL)
	exit(1);

 estatData->info1=0;
 estatData->info2=0;
 estatData->info3=0;
 estatData->info4=0;
 estatData->info5=0;
 estatData->infoVAX=0;

 llistaIndics->items=new TCollection2((ccIndex)10,(ccIndex)10);
 llistaIndics->focus=0;

 if((llistaEmers=(ptrIndicsData)malloc(sizeof(listIndicsData)))==NULL)
	exit(1);
 llistaEmers->items=new TCollection2((ccIndex)10,(ccIndex)10);
 llistaEmers->focus=0;



 veuData=new FinVeuData[1];
 strcpy(veuData->indicatiu,"001");

 inData=new FinInData[1];
 strcpy(inData->indicatiu,"001");

 trucData=new trucGrupData[1];
 trucData->items=new TCollection2((ccIndex)10,(ccIndex)10);
 trucData->focus=0;


 trucData2=new trucGrupData2[1];
 trucData2->items=new TCollection2((ccIndex)10,(ccIndex)10);
 trucData2->focus=0;


 anulData=new anulGrupData[1];
 strcpy(anulData->grup,"01");

 defData=new defGrupData[1];
 initDefData();

 repData1=new RepData[1];
 repData1->items=new TCollection2((ccIndex)10,(ccIndex)10);
 repData1->focus=0;
 strcpy(traficBuffer[0]," Comanda  Flota  Indicatiu  Origen  Altres");
 for(i=1;i<MAX_NUMLINES;i++)
  memset(traficBuffer[i],' ',MAX_LENGTHLINE);
}

void restauraInt(void)
{
  outp (rci2,0x00);      /* Desactiva interrupciones */

  _dos_setvect(11,ViejIntCOM2);
}

void main()
{

 char tempo2[LONG_REP+1];

 initListIndic();
 initConfigCGC();
 initEqMob();
 initRep();

 password();

 TMyApp myApp;



 strcpy(tempo2,"COLLSEROLA");
 myApp.afegeixRep(tempo2);

 strcpy(tempo2,"MONTAGUT");
 myApp.afegeixRep(tempo2);

 strcpy(tempo2,"TOSSA D'ALP");
 myApp.afegeixRep(tempo2);

 strcpy(tempo2,"TURO DE L'HOME");
 myApp.afegeixRep(tempo2);

 strcpy(tempo2,"TURO DE LA ROVIRA");
 myApp.afegeixRep(tempo2);

 myApp.run();

 lliberaMemo();
 restauraInt();
}
