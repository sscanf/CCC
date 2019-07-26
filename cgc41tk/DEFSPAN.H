
#ifndef __DEFINIS_H
#define __DEFINIS_H

#define Uses_TApplication
#define Uses_TKeys
#define Uses_TRect
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TDeskTop
#define Uses_TMenuBar
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TWindow
#define Uses_TDemoWindow
#define Uses_TEvent
#define Uses_TView
#define Uses_TDrawBuffer
#define Uses_TScroller
#define Uses_TScrollBar
#define Uses_TDialog
#define Uses_TButton
#define Uses_TLabel
#define Uses_TInputLine
#define Uses_TListBox
#define Uses_TListViewer
#define Uses_TCollection
#define Uses_TStringCollection
#define Uses_TPalette
#define Uses_TWindowInit
#define Uses_TCommandSet
#define Uses_THistory
#define Uses_TCheckBoxes
#define Uses_TSItem
#define Uses_TStaticText
#define Uses_TRadioButtons
#define Uses_TListBox
#define Uses_MsgBox
#define Uses_TFrame
#define MAX_NUM_CANAL 30
#define MAX_NUM_OPER 30
#define MAX_BUFFER_TX 800

#include <tv.h>
#include <stdlib.h>
#include <strstrea.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <math.h>
#include <msgbox.h>
#include <dos.h>
#include <util.h>

enum {CANALS=0,TRAFIC,INCID,SOS,RES};


#define MAX_CH_BUF  5 /* MAX_NUM_CH/6 i si reste +1 */
#define MAX_OPER_BUF 9 /* MAX_NUM_OPER/4 i si reste +1 */

#define ACT_CH 5
#define ACT_OPER 4

#define MAX_SERVEIS 30
#define MAX_GRUP 20
#define MAX_REP 7
#define MAX_EMER 20

#define HEADER 4
#define MAX_BUFFER_RX 60  // si ‚s gran (ACT_CH*ACT_OPER) MAX_BUFFER_RX=*
								  // LONG_GRUP*MAX_GRUP
#define ON 1
#define OFF 0

#define LONG_INDIC 4
#define LONG_GRUP 3
#define LONG_CANAL 12

#define MAX_LONG_INFO 60 // indic o LONG_GRUP*MAX_GRUP

#define LONG_REP 19

#define MAX_NUM_COMND 26


#define MAX_NUMLINES 25
#define MAX_LENGTHLINE 80
const unsigned long TIMEOUT=9000000;
const cmRxPrefixes   =199;

const cmCGC				=200;
const cmEqMob			=201;
const cmMCan			=202;
const cmMOper			=203;

const cmFinGrup		=209;
const cmERROR			=210;

const cmGrpConfig		=212;
const cmMsgVeu			=214;
const cmSinTrafic		=215;
const cmTrucades		=216;
const cmVeu				=217;
const cmAnulVeu      =218;
const cmVeuE         =219;
const cmPTT				=220;

const cmExtrEmers    =221;

const cmFinVeu			=222;
const cmRep				=223;
const cmTx				=224;
const cmFinEmer		=225;
const cmFinInci		=226;
const cmGrvConfig		=227;
const cmListServ		=228;
const cmMsServeis		=229;
const cmModfServeis  =230;
const cmMsEmer			=231;
const cmModfEmers    =232;
const cmTrucGrup		=233;
const cmDefGrup		=234;
const cmAnulGrup		=235;
const cmListEmer		=236;
const cmRep1			=237;

const cmRES     		=238;

const cmExtrServeis  =239;
const cmObrir			=240;

const cmFinTrucGrup  =241;

const cmTst				=242;

const cmEstat        =243;
const cmTrafic       =244;

const cmRxConfig     =245;

const cmTrucCanal    =246;
const cmIniNit       =247;
const cmNit          =248;
const cmSqlOn        =249;
const cmCanviObrir   =250;
const cmMsTrafic     =251;
const cmSqlOff			=252;
const SEND_PROG		=253;
const CLEARED			=254;
const cmDescon		   =255;
const cmCancelC		=256;
const TRUC_SOCI		=198;
const cmIndiv			=100;
const TIMER				=101;




const	bard    = 0x080; /* Registro de acceso al registro divisor 8250   */
const	tdl     = 0x001;
const	lpe     = 0x002;
const	out2    = 0x008;
const	cop1    = 0x021;
const	cop2    = 0x020;
const	irq4	  = 0x000; /*0x10*/
const irq3    = 0x0ef;

const	com2    = 0x2f8; /* entrada datos com1 */
const	rcmm2     = 0x2fd;
const	rcm2    = 0x2fc;
const	rrb2     = 0x2f8; /* Registro de recepci¢n 8250                    */
const	rat2     = 0x2f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl2     = 0x2fb; /* Registro de control de l¡nea 8250             */
const	LineStat2= 0x2fe; /* Line status RS232				  */
const	rci2     = 0x2f9; /* Registro de activaci¢n de interrupciones 8250 */
const	relin2   = 0x2fd; /* Registro de estado de l¡nea 8250              */
const	rdl2     = 0x2f8; /* Registro divisor 8250 LSB                     */
const	rdm2     = 0x2f9; /* Registro divisor 8250 MSB                     */

const	com1    = 0x3f8; /* entrada datos com1 */
const	rcmm1     = 0x3fd;
const	rcm1    = 0x3fc;
const	rrb1     = 0x3f8; /* Registro de recepci¢n 8250                    */
const	rat1     = 0x3f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl1     = 0x3fb; /* Registro de control de l¡nea 8250             */
const	LineStat1= 0x3fe; /* Line status RS232				  */
const	rci1     = 0x3f9; /* Registro de activaci¢n de interrupciones 8250 */
const	relin1   = 0x3fd; /* Registro de estado de l¡nea 8250              */
const	rdl1     = 0x3f8; /* Registro divisor 8250 LSB                     */
const	rdm1     = 0x3f9; /* Registro divisor 8250 MSB                     */



#define cpNewPalette4 "\x11\x11\x12\x13\x14\x15\x16\x17"

#define cpNewPalette "\x19\x19\x1a\x1b\x1c\x1d\x1e\x1f"

#define cpNewPalette2 "\x2\x2\x4\x4\x1" /*active inactive focused selected divider*/

#define cpNewPalette3 "\x1\x1\x8\x8\x1"

void lliberaMemo(void);
void restauraInt(void);

struct ConfigRepData
{
 char repetidor[LONG_REP];
 ushort releStat;
 char subtoTx[3];
 char subtoRx[3];
};

struct listIndicsData
{
 TCollection *items;
 short focus;
 short status;
};


struct RxInfo
{
 unsigned int comnd;
 unsigned int numProc;
 char *info;
};


struct ConfigEqMobData
{
 char indicatiu[LONG_INDIC];
 char indicNou[LONG_INDIC];
 ushort estatus;
};

struct EstatData
{
 unsigned int info1;
 unsigned int info2;
 unsigned int info3;
 unsigned int info4;
 unsigned int info5;
 unsigned int infoVAX;
};


struct FinVeuData
{
 char indicatiu[LONG_INDIC];
 char canal[3];
};


struct anulGrupData
{
 char grup[LONG_GRUP];
};

struct trucGrupData
{
 char grup[LONG_GRUP];
 TCollection *items;
 short focus;
 short status;
};


struct trucGrupData2
{
 TCollection *items;
 short focus;
 short status;
};


struct RepData
{
 char rep[LONG_REP];
 TCollection *items;
 short focus;
 short status;
};

struct FinInData
{
 char indicatiu[LONG_INDIC];
};

struct ObrirInfo
{
 ushort info;
};

struct ConfigCGCData
{
 ushort chk1;
 ushort chk2;
 ushort chk3;
 ushort chk4;
 ushort chk5;
 ushort chk6;
 ushort chk7;
 ushort chk8;
 ushort chk9;
 ushort chk10;
 ushort chk11;
 ushort chk12;
 ushort chk13;
 ushort chk14;
 ushort chk15;
 ushort chk16;
 ushort chk17;
 ushort chk18;
 ushort chk19;
 ushort chk20;
 ushort chk21;
 ushort chk22;
 ushort chk23;
 ushort chk24;
};

struct defGrupData
{
 char grup[LONG_GRUP];
 char ind1[LONG_INDIC];
 char ind2[LONG_INDIC];
 char ind3[LONG_INDIC];
 char ind4[LONG_INDIC];
 char ind5[LONG_INDIC];
 char ind6[LONG_INDIC];
 char ind7[LONG_INDIC];
 char ind8[LONG_INDIC];
};

struct indics
{
 char *indicatiu;
 unsigned int numProc;
};

typedef struct ObrirInfo      *ptrObrirData;
typedef struct EstatData      *ptrEstatData;
typedef struct ConfigCGCData  *ptrCGCData;
typedef struct ConfigRepData  *ptrRepData;
typedef struct ConfigEqMobData  *ptrEqMobData;
typedef struct listIndicsData  *ptrIndicsData;
typedef struct FinVeuData *ptrFinVeuData;
typedef struct FinInData *ptrFinInData;
typedef struct trucGrupData *ptrTrucGrupData;
typedef struct trucGrupData2 *ptrTrucGrupData2;
typedef struct trucGrupData2 *ptrTrucCanalData;

typedef struct anulGrupData *ptrAnulGrupData;
typedef struct defGrupData *ptrDefGrupData;
typedef struct RepData *ptrRepData1;

typedef struct RxInfo *ptrRxInfo;
typedef struct indics *ptrIndics;


class TIndic:public TObject
{
 public:
 const char *name;
 TIndic(char *newName);
 ~TIndic();
};

class TCollection2 : public virtual TCollection
{

public:

	 TCollection2( ccIndex aLimit, ccIndex aDelta ):TCollection(aLimit,aDelta){}
private:

	 virtual void *readItem( ipstream& ) {}
    virtual void writeItem( void *, opstream& ) {}
};


class TListBox2: public TListBox
{
 public:
  TListBox2( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar );
  virtual TPalette& getPalette() const;
  virtual void handleEvent(TEvent& event);
};

class TListBox3: public TListBox
{
 public:
  TListBox3( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar );
  virtual TPalette& getPalette() const;
  virtual void handleEvent(TEvent& event);
};


class TListBox5: public TListBox
{
 public:
  TListBox5( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar );
  virtual void handleEvent(TEvent& event);
 private:
  void Tx2CGC(unsigned int,unsigned int,unsigned int,int, char *);
  void sacabyte(unsigned int);


};

class TListBox6: public TListBox
{
 public:
  TListBox6(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar );
  virtual void handleEvent(TEvent& event);
  virtual TPalette& getPalette() const;
};

class TListBox4: public TListBox
{
 public:
  TListBox4( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar );
  virtual void handleEvent(TEvent& event);
};


class TListWindow:public TWindow
{
 public:
 TListWindow(const TRect &bounds,const char *aTitle,short aNumber);
 virtual TPalette& getPalette() const;
 virtual void close();
};

class TListWindow3:public TWindow
{
 public:
 TListWindow3(const TRect &bounds,const char *aTitle,short aNumber);
 virtual TPalette& getPalette() const;
 virtual void close();
};

class  TMyApp: public TApplication
{

 public:
  TMyApp();
  static TStatusLine *initStatusLine(TRect r);
  static TMenuBar *initMenuBar(TRect r);
  void handleEvent(TEvent &event);
  void myNewWindow(int proc);
  void newDialog(int proc);
  void dialogCGC(void);
  void dialogRep2(void);
  void dialogEqMob(void);
  void gestioVeu2();
  void afegeixTrucGrup(char *);
  void afegeixRep(char *);

 private:

  void tstCGCOn(void);
  virtual void idle();
  char *creaString(unsigned int numChAct,unsigned int numCanal);
  char *creaString2(unsigned int numCanal);
  TDialog *configCGC(TDialog *);
  unsigned int controlDades(ptrCGCData);
  TDialog *configRep(TDialog *pd1);
  TDialog *configEqMob(TDialog *pd1);
  void Tx2CGC(unsigned int,unsigned int,unsigned int,int,char *);
  void sacabyte(unsigned int);
  void convert2Tx();
  void ompleConfigTx(unsigned int,unsigned int,unsigned int,ushort,
			  unsigned int,unsigned int,unsigned int);
  void initTx(void);
  unsigned int ultimCasX(unsigned int);
  unsigned int ultimCasY(unsigned int);
  void creaListBox();
  void creaEmerBox();
  void creaCanalBox();
  void gestioNit(void);

  void afegeixEmers(char *,unsigned int,unsigned int);
  void afegeixServeis(char *,unsigned int,unsigned int);

  void extreuServeis(unsigned int);
  void extreuEmers(unsigned int);
  void extreuGrup();
  void extreuTrucGrup();

  void modifServeis(unsigned int);
  void modifEmers(unsigned int);
  void modifCanals(unsigned int);

  TDialog *dialogVeu(TDialog *pd1);
  TDialog *dialogEstat(TDialog *pd1);
  TDialog *dialogObrir(TDialog *pd1);

  TDialog *dialogFinDades(TDialog *pd1);
  TDialog *dialogTrucGrup(TDialog *pd1);

  TDialog *dialogRep(TDialog *pd1);

  void gestioVeu(TEvent &event);

  void gestioTrucCanal();
  void gestioCanviObrir();

  void traficWinFunc(void);
  void afegeixTrafic(void);


  char *entradaDades(char *);
  TDialog *dialogDefGrup(TDialog *);
  void getStringTrucsGrup(void);
  void llegeixEstatData(void);

  void getCGCConfig(void);
  void getAnswerCGC(int);
  int busca(char *indiC);
};



class TTraficWindow:public TWindow
{
 public:
 TTraficWindow(const TRect &bounds,const char *aTitle,short aNumber);
 virtual TPalette& getPalette() const;
 void makeInterior();
 virtual void close();
};



class TListWindow2:public TWindow
{
 public:
 TListWindow2(const TRect &bounds,const char *aTitle,short aNumber);
 virtual TPalette& getPalette() const;
 virtual void close();
};


class configCGCButton:public TButton
{
 public:
 configCGCButton(const TRect& bounds,const char *aTitle,ushort aCommand,
   ushort aFlags);
 virtual void handleEvent(TEvent &event);
};

class TRepButton:public TButton
{
 public:
 TRepButton(const TRect& bounds,const char *aTitle,ushort aCommand,
   ushort aFlags);
 virtual void handleEvent(TEvent &event);
 void Tx2CGC(unsigned int,unsigned int,int,char *);
 void sacabyte(unsigned int);
};

class TDStaticText:public TStaticText
{
 public:
 TDStaticText(const TRect& bounds,const char *aText);
};

class TInterior:public TScroller
{
 public:
  TInterior(const TRect& bounds,TScrollBar *aHScrollBar,TScrollBar *aVScrollBar);
  virtual void draw();
  virtual void handleEvent(TEvent &event);
};

#endif

