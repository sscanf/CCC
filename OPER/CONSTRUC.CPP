
#include "c:defspan.h"
/*__________ CONSTRUCTORS ________________*/
extern ptrRxInfo listRxCmnd;
extern unsigned int lastCmnd,comndAct;

extern int tecnic;
extern unsigned int maxLineLength;
extern unsigned int CGCOn;
extern unsigned int inici;
extern ptrObrirData obrirData;
extern TTraficWindow *traficWin;
extern char traficBuffer[MAX_NUMLINES][MAX_LENGTHLINE];
extern unsigned int posLine,iniLine,trucCanalFin;
TScrollBar *vScrollBar;

extern ptrTrucCanalData trucCanalData;

TDStaticText::TDStaticText(const TRect& bounds,const char *aText):
  TStaticText(bounds,aText)
{}
TListBox2::TListBox2(const TRect& bounds,ushort aNumCols,TScrollBar *aScrollBar):
 TListBox(bounds,aNumCols,aScrollBar)
{}

TListBox3::TListBox3(const TRect& bounds,ushort aNumCols,TScrollBar *aScrollBar):
 TListBox(bounds,aNumCols,aScrollBar)
{}


TListBox4::TListBox4(const TRect& bounds,ushort aNumCols,TScrollBar *aScrollBar):
 TListBox(bounds,aNumCols,aScrollBar)
{}

TListBox5::TListBox5(const TRect& bounds,ushort aNumCols,TScrollBar *aScrollBar):
 TListBox(bounds,aNumCols,aScrollBar)
{}

TListBox6::TListBox6(const TRect& bounds,ushort aNumCols,TScrollBar *aScrollBar):
 TListBox(bounds,aNumCols,aScrollBar)
{}

configCGCButton::configCGCButton(const TRect& bounds,const char *aTitle,ushort aCommand,
	ushort aFlags):TButton(bounds,aTitle,aCommand,aFlags)
{}

TRepButton::TRepButton(const TRect& bounds,const char *aTitle,ushort aCommand,
	ushort aFlags):TButton(bounds,aTitle,aCommand,aFlags)
{}

void TMyApp::tstCGCOn(void)
{
 unsigned long time=0;
 TDialog *pd1;
 TButton *pd2;
 TRect r6(0,0,40,13);

 outp(rci2,0x01); // activa int de recepci¢n


 Tx2CGC(cmTst,0,0,"");

 delay(500);

 r6.move(20,5);
 pd1=new TDialog(r6,"Information");
 if (pd1)
 {
	 pd1->insert(new TDStaticText(TRect(2,2,38,3),"\03Comprovant que CGC es trobi en"));
	 pd1->insert(new TDStaticText(TRect(2,4,38,5),"\03funcionament ..."));

	 deskTop->insert(pd1);
 }


 for(;CGCOn==OFF && time<TIMEOUT;time++);
 if (CGCOn==OFF)
 {
	pd1->insert(new TDStaticText(TRect(2,6,38,7),"\03El CGC est… parat."));
	pd1->insert(new TDStaticText(TRect(2,8,38,9),"\03Premi return per sortir."));
	pd2=new TButton(TRect(14,10,24,12),"~O~k",cmOK,bfDefault);
	pd1->insert(pd2);
	while(!(kbhit()));
	lliberaMemo();
	restauraInt();
	exit(1);
 }
 else
	delay(700);

 destroy(pd1);
 inici=OFF;
 CGCOn=OFF;
 outp(rci2,0x00); // desactiva int de recepci¢n

}

TMyApp::TMyApp():TProgInit( TMyApp::initStatusLine,
	 TMyApp::initMenuBar,
	 TMyApp::initDeskTop)
{
 initTx();
 messageBoxRect(TRect(13,5,53,17),"\03Electr•nica Barcelona, S.L.\n \n\03Sistema Central de Coordinaci¢\n\03de Comunicacions\n \n\03Febrer, 1995\n",mfOKButton|mfInformation);

 tstCGCOn();
// inici=OFF; // aix• fora quan tstCGCOn hagi d'estar en marxa

	 if ((*obrirData).info==1 && tecnic==1) disableCommand(cmTrucCanal);
	 disableCommand(cmPTT);
	 disableCommand(cmNit);
	 disableCommand(cmTrafic);


	 if (tecnic==0)
	 {
	  enableCommand(cmFinVeu);
	  enableCommand(cmListServ);
	  disableCommand(cmCGC);
	  disableCommand(cmEqMob);
	  disableCommand(cmMCan);
	  disableCommand(cmMOper);
	  TEvent event;
	  event.what=evCommand;
	  event.message.command=cmListEmer;
	  handleEvent(event);
	  event.what=evCommand;
	  event.message.command=cmListServ;
	  handleEvent(event);
	  if ((*obrirData).info==0)
	  {
		event.what=evCommand;
		event.message.command=cmTrucCanal;
		handleEvent(event);
	  }
	 }
}

void TListBox6::handleEvent(TEvent& event)
{
 unsigned int tempo=trucCanalData->focus;
 TMyApp *tm;

 if (event.what==1)
 {
	if (trucCanalData->status==1)
	{
	 listRxCmnd[lastCmnd].comnd=cmPTT;
	 listRxCmnd[lastCmnd].numProc=0;
	 strcpy(listRxCmnd[lastCmnd].info,"");

	 if (lastCmnd<(MAX_NUM_COMND-1))
	 {
		lastCmnd++;
		listRxCmnd[lastCmnd].comnd=cmRES;
		listRxCmnd[lastCmnd].numProc=0;
		memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
	 }
	 tm->gestioVeu2();
	}
 }

 if (event.what==evKeyboard)
 {
  switch (event.message.command)
  {
	case kbDown:


		if (trucCanalData->status==1)
		{
		 listRxCmnd[lastCmnd].comnd=cmPTT;
		 listRxCmnd[lastCmnd].numProc=0;
		 strcpy(listRxCmnd[lastCmnd].info,"");

		 if (lastCmnd<(MAX_NUM_COMND-1))
		 {
			lastCmnd++;
			listRxCmnd[lastCmnd].comnd=cmRES;
			listRxCmnd[lastCmnd].numProc=0;
			memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
		 }

		 tm->gestioVeu2();
		}
		 if (tempo<ACT_CH-1) trucCanalData->focus++;
	break;

	case kbUp:
		if (trucCanalData->status==1)
		{

		 listRxCmnd[lastCmnd].comnd=cmPTT;
		 listRxCmnd[lastCmnd].numProc=0;
		 strcpy(listRxCmnd[lastCmnd].info,"");

		 if (lastCmnd<(MAX_NUM_COMND-1))
		 {
			lastCmnd++;
			listRxCmnd[lastCmnd].comnd=cmRES;
			listRxCmnd[lastCmnd].numProc=0;
			memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
		 }

		 tm->gestioVeu2();
		}

	 if (tempo>0) trucCanalData->focus--;
	break;


	case kbHome:
	case kbPgDn:
		if (trucCanalData->status==1)
		{

		 listRxCmnd[lastCmnd].comnd=cmPTT;
		 listRxCmnd[lastCmnd].numProc=0;
		 strcpy(listRxCmnd[lastCmnd].info,"");

		 if (lastCmnd<(MAX_NUM_COMND-1))
		 {
			lastCmnd++;
			listRxCmnd[lastCmnd].comnd=cmRES;
			listRxCmnd[lastCmnd].numProc=0;
			memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
		 }

		 tm->gestioVeu2();
		}

		trucCanalData->focus=0;
	 break;

	case kbEnd:
	case kbPgUp:
	if (trucCanalData->status==1)
	{

		 listRxCmnd[lastCmnd].comnd=cmPTT;
		 listRxCmnd[lastCmnd].numProc=0;
		 strcpy(listRxCmnd[lastCmnd].info,"");

		 if (lastCmnd<(MAX_NUM_COMND-1))
		 {
			lastCmnd++;
			listRxCmnd[lastCmnd].comnd=cmRES;
			listRxCmnd[lastCmnd].numProc=0;
			memset(listRxCmnd[lastCmnd].info,'\0',MAX_LONG_INFO);
		 }

		 tm->gestioVeu2();
	}
		trucCanalData->focus=ACT_CH-1;
	 break;
	default:;

  }
 }

 TListBox::handleEvent(event);
 TListBox::drawView();

}


TInterior::TInterior(const TRect& bounds,TScrollBar *aHScrollBar,TScrollBar *aVScrollBar):
	   TScroller(bounds,aHScrollBar,aVScrollBar)
{
 growMode=gfGrowHiX|gfGrowHiY;
 options=options|ofFramed;
 setLimit(MAX_LENGTHLINE,MAX_NUMLINES);
}



TTraficWindow::TTraficWindow(const TRect &bounds,const char *aTitle,short aNumber):
    TWindow(bounds,aTitle,aNumber),TWindowInit(TTraficWindow::initFrame)
{
 makeInterior();
 setState(sfShadow,(Boolean)0);
}

TListWindow::TListWindow(const TRect &bounds,const char *aTitle,short aNumber):
    TWindow(bounds,aTitle,aNumber),TWindowInit( TListWindow::initFrame)
{
 if (tecnic==0)
  flags=flags&(~wfClose);
 flags=flags&(~wfGrow);
}

TListWindow3::TListWindow3(const TRect &bounds,const char *aTitle,short aNumber):
	 TWindow(bounds,aTitle,aNumber),TWindowInit( TListWindow3::initFrame)
{
 if (tecnic==0)
  flags=flags&(~wfClose);
 flags=flags&(~wfGrow);
}

void TListWindow::close()
{
 TWindow::close();
 enableCommand(cmListServ);
}

void TListWindow3::close()
{
 TWindow::close();
 trucCanalFin=0;
 if (tecnic==1 && (*obrirData).info==0) enableCommand(cmTrucCanal);
}

void TTraficWindow::close()
{
 TWindow::close();
 enableCommand(cmTrafic);
}


TListWindow2::TListWindow2(const TRect &bounds,const char *aTitle,short aNumber):
   TWindow(bounds,aTitle,aNumber),TWindowInit( TListWindow2::initFrame)
{
 if(tecnic==0)
  flags=flags&(~wfClose);
 flags=flags&(~wfGrow);
}

void TListWindow2::close()
{
 TWindow::close();
 enableCommand(cmListEmer);
}

TPalette& TListWindow::getPalette() const
{
 static TPalette palette(cpNewPalette,sizeof(cpNewPalette)-1);
 return palette;
}

TPalette& TListWindow3::getPalette() const
{
 static TPalette palette(cpNewPalette,sizeof(cpNewPalette)-1);
 return palette;
}

TPalette& TTraficWindow::getPalette() const
{
 static TPalette palette(cpNewPalette,sizeof(cpNewPalette)-1);
 return palette;
}

TPalette& TListWindow2::getPalette() const
{
 static TPalette palette(cpNewPalette4,sizeof(cpNewPalette4)-1);
 return palette;
}


TPalette& TListBox2::getPalette() const
{
 static TPalette palette(cpNewPalette2,sizeof(cpNewPalette2)-1);
 return palette;
}

TPalette& TListBox6::getPalette() const
{
 static TPalette palette(cpNewPalette2,sizeof(cpNewPalette2)-1);
 return palette;
}

TPalette& TListBox3::getPalette() const
{
 static TPalette palette(cpNewPalette3,sizeof(cpNewPalette3)-1);
 return palette;
}

void TListBox2::handleEvent(TEvent& event)
{
 TListBox::handleEvent(event);
 TListBox::drawView();
}

void TListBox3::handleEvent(TEvent& event)
{

 TListBox::handleEvent(event);
 TListBox::drawView();
}

void TListBox4::handleEvent(TEvent& event)
{

 TListBox::handleEvent(event);
 TListBox::drawView();

}

void TTraficWindow::makeInterior()
{
 vScrollBar=standardScrollBar(sbVertical|sbHandleKeyboard);
 TScrollBar *hScrollBar=standardScrollBar(sbHorizontal|sbHandleKeyboard);
 TRect r=getClipRect();
 r.grow(-1,-1);
 insert(new TInterior(r,hScrollBar,vScrollBar));
}




