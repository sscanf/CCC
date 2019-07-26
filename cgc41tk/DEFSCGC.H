#ifndef __DEFSCGC
#define __DEFSCGC

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <math.h>
#include <dos.h>
#include <alloc.h>
#include <mem.h>
#include <malloc.h>

#include <c128.h>

#define NumBase 602 //2852;

// inclou els fitxers referents a les plaques MOXA
#include "process.h"
// num de canals i operadors de la xarxa
#define ACT_CH 5
#define ACT_OPER 4

#define HEADER 2

// La llongitud dels cap‡als que s'espera rebre depŠn de la font, ja
// sigui la r…dio, el vax o l'operador.
#define HEADER_RADIO 5
#define HEADER_OPER 5
#define HEADER_VAX1 3
#define HEADER_VAX2 3
#define HEADER_VAX 6

// Diferents estats de les tasques
enum {NOTRUN=0,FIEXEC,ESPERA,FIESPERA,NOACTIU};

#define MAX_TABLA 6
// Dimensi¢ dels camps d'informaci¢ de diverses estructures.
#define MAX_BUFFER_RX 60 // Si (ACT_CH*ACT_OPER) es grande MAX_BUFFER_RX=*
								 // o MAX_BLOQUE o MAX_GRUP*LONG_GRUP.S'agafa el m‚s gran.
								 // o LONG_GRUP+8+(LONG_INDIC*8)
#define MAX_LONG_INFO 48 // MAX_BLOQUE es el string m‚s llarg pot hever en listCmnd
#define MAX_BUFFER_VAX 418 // per la recepcio del VAX i per indics, on es guarda fitxer
#define MAX_BLOQUE 48

#define MAX_GRUP 20 // m…xim nombre de grups definits
#define LONG_GRUP 3 // Els grups queden definits amb 2 bytes
#define LONG_INDIC 4 // Els indicatius queden definits amb 4 bytes


// Nombre m…xim de tasques en la llista
#define MAX_NUM_COMND  70
#define MAX_NUM_EXEC   40
// Nombre m…xim de canals de r…dio
#define MAX_NUM_RADIO 30

// num de reTx de fitxers
#define NUM_RETX 3
// num de missatges
#define MAX_LONGMSG 5
#define NUM_MISSATGES 30
// -Comanda--Flota--Indicatiu--Origen--Altres------ .... MAX_LENGTHLINE=80
#define MAX_MSGTRAFIC 160

// canals r…dio
enum {PORT0=0,PORT1,PORT2,PORT3,PORT4,PORT5,PORT6,PORT7,PORT8,PORT9,PORT10,PORT11,PORT12,PORT13,PORT14,PORT15,PORT16};

enum {RX=1,TX,TX_RX,Res};
enum {PTTOFF=0,PTTON};
enum {SILENCIADOR=0,CODIS};

const unsigned long TIMEOUT = 10000;

const OK				=0;
const NOTOK       =-1;

/* COMANDES */

const RES			=238;

/* OPERADOR -> CGC */

const INI_CONFIG		=227;
const TRUCINDIV	   =222; // trucada individual
const PETVEUE		   =219;
const PETVEUS		   =217; // perm¡s de veu
const ANULVEU        =218; // anul.la perm¡s de veu
const ESTATPTT			=220; // han polsat PTT, ON o OFF
const OBRIR				=240;
const FINTRUCGRUP    =241;

const TRUCGRUP		=233;
const DEFGRUP		=234;
const ANULGRUP		=235;

const RXPREFIXES  =199;
const TST			=242;
const ESTAT       =243;

const RXCONFIG    =245;
const TRUCSOCI		=198;

/* CGC -> OPERADOR */

const MSSERVEIS   =229;
const MSEMER      =231;
const ERROR       =210;
const MODFSERVEIS =230;
const cSEND_PROG  =253;
const cCLEARED		=254;

// tamb‚ ANULVEU per fer desaparŠixer una Incid que estigui en diferents
// operadors.
const MODFEMERS   =232;
const EXTREMERS   =221;
const EXTRSERVEIS =239;
const ININITOPER  =247;
const OPERNIT     =248;
const OPERSQLON   =249;
const OPERSQLOFF  =252;
const MSTRAFIC    =251;



// tamb‚ TRUCGRUP		=233;
// DEFGRUP		=234;
// ANULGRUP		=235;
// RXPREFIXES  =241;



/* COMANDES MASTER CLOCK */

/* CGC -> MASTER CLOCK */

const RXESTAT		=11;
const RXSUBTO     =12;
const TXDADES     =13;
const CONFIG		=14; // Programaci¢ d'una £nica connexi¢.

// INI_CONFIG de l'operador provoca una programaci¢ total de Master CLK


/* CGC -> ST */

const PERVEU  =0x0a; // perm¡s Veu pel m•bil
const FIVEU   =0x0b; // no perm¡s Veu pel M•bil

/* ST -> CGC */

const DES 		=1; // al VAX
const OCU		=2; // al VAX
const PAR		=3; // al VAX

const FIN		=4; // provoca l'alta en la llista dels indicatius.

const ERR      =5;

const EME		=6; // a convertir MSEMER
						 /* petici¢ d'emergŠncia provoca que s'afegeixi SOS
								a la llista de l'operador.
							*/

const VEU		=7; // a convertir MSSERVEIS
						  /* petici¢ de veu per part del m•bil provoca
								que s'afegeixi indicatiu a la llista operador.
							*/
const TAN		=8; // provoca baixa en la llista dels indicatius.

const MEC		=9; // al VAX , a m‚s, es reb el num de mec…nic i num Serie
const OBE      =9;

const LLI		=0x10; // al VAX
const SEND_PROG = 0x12;
const CLEARED	 = 0x13;

const SQLON		=0x22; // activitat en el canal
const SQLOFF   =0x11; // no activitat en el canal

const PROGINDIC=0x0c;

const PTT = 0x20;
const NOPTT = 0x21;
const TRUC_SOCI = 10;

// VAX -> CGC
const ANU = 0x14;
const ASI = 0x15;
const MOD = 0x16;
const NIT = 23;
const ERRHOST = 24;



const MAX_FITXER = 418; // Segons def del RACC
const MAX_FITMOD = 418;  // Segons def del RACC
const MAX_ANU	  = 9;
const MAX_NIT	  = 3;


const	bard    = 0x080; /* Registro de acceso al registro divisor 8250   */
const	tdl     = 0x001;
const	lpe     = 0x002;
const	out2    = 0x008;
const	cop1    = 0x021;
const	cop2    = 0x020;

const	irq4	  = 0x010;
const irq3    = 0xe7;

const	com1     = 0x3f8; /* entrada datos com1 */
const	rccm1    = 0x3fd;
const	rcm1     = 0x3fc;
const	rrb1     = 0x3f8; /* Registro de recepci¢n 8250                    */
const	rat1     = 0x3f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl1     = 0x3fb; /* Registro de control de l¡nea 8250             */
const	LineStat1= 0x3fe; /* Line status RS232				  */
const	rci1     = 0x3f9; /* Registro de activaci¢n de interrupciones 8250 */
const	relin1   = 0x3fd; /* Registro de estado de l¡nea 8250              */
const	rdl1     = 0x3f8; /* Registro divisor 8250 LSB                     */
const	rdm1     = 0x3f9; /* Registro divisor 8250 MSB                     */

const	com2     = 0x2f8; /* entrada datos com2 */
const	rccm2    = 0x2fd;
const	rcm2     = 0x2fc;
const	rrb2     = 0x2f8; /* Registro de recepci¢n 8250                    */
const	rat2     = 0x2f8; /* Registro de almac‚n de transmisi¢n 8250       */
const	rcl2     = 0x2fb; /* Registro de control de l¡nea 8250             */
const	LineStat2= 0x2fe; /* Line status RS232				  */
const	rci2     = 0x2f9; /* Registro de activaci¢n de interrupciones 8250 */
const	relin2   = 0x2fd; /* Registro de estado de l¡nea 8250              */
const	rdl2     = 0x2f8; /* Registro divisor 8250 LSB                     */
const	rdm2     = 0x2f9; /* Registro divisor 8250 MSB                     */

//typedef void (*funcExec)(void) ptrFuncExec;


typedef struct cmndInfo *ptrCmndInfo;
typedef struct radioInfo *ptrRadioInfo;
typedef struct operInfo *ptrOperInfo;
typedef struct msgInfo *ptrMsgInfo;


struct msgInfo
{
 char *info;
 unsigned int num;
};

struct cmndInfo
{
 unsigned int comnd;
 unsigned int origen;
 char *info; /* Informaci¢ que necessita la tasca per executar-se */
 unsigned int proces;
 unsigned int canal;
 unsigned int estat; // proces i estat informem de l'estat en que es troba
							//	l'execuci¢ de la tasca.
 unsigned int flag;

};

struct radioInfo
{
 unsigned int flota;
 int flotaAnt;
 unsigned int grup;
 unsigned int indicatiu; // 3 digits
 int canal;
 unsigned int lastFunc;
 long unsigned int expedient; // 7 digits
 unsigned int servei; // 2 digits
 unsigned int subto;
 char *fitxer;
 unsigned int permisVeu;
 long unsigned int numSerie;
 char numMecanic[3];
 unsigned int numReTx;
};

struct operInfo
{
 unsigned int estat;
 int canalAss;
 unsigned int ctrlPTT;
 unsigned int TxRadio; // flag indica oper connectat al canal, £til per treure veu quan Tx codis
};

void gestioTrucSoci(void);
void execsys(void);
void presenta(void);
unsigned int llegeixCmnd(void);
void initPorts(void);
void error(char *);
void tstComndAct(void);
void closeMoxa (void);
int openMoxa(void);

int posicioIndicatiu(unsigned int);


int  IniSio(void);		//Inicializa la SIO
void initPorts(void);

//  Transmissi¢ a consoles
int Tx2PortO(unsigned int,unsigned int,unsigned int,unsigned int, char *);

// Transmissi¢ a canal de r…dio
int Tx2PortR(unsigned int,unsigned int,unsigned int,unsigned int,
				 char *,unsigned int);


void gestRxIntOpers(unsigned int,unsigned char *,char *);
void gestioRxIntRadio(unsigned int,unsigned char *,char *);


void interrupt IntPort0(void);
void interrupt IntPort1(void);
void interrupt IntPort2(void);
void interrupt IntPort3(void);
void interrupt IntPort4(void);
void interrupt IntPort5(void);
void interrupt IntPort6(void);
void interrupt IntPort7(void);
void interrupt IntPort8(void);
void interrupt IntPort9(void);
void interrupt IntPort10(void);

void interrupt IntFitxer3();
void interrupt IntFitxer4();
void interrupt IntFitxer5();
void interrupt IntFitxer6();
void interrupt IntActiu0();
void interrupt IntTIMER(...);

void actListIndics(unsigned int,char *);

void sacabyte(unsigned int ,unsigned int);
void Tx2MasterClk(unsigned int ,unsigned int,unsigned int,unsigned int);
unsigned int inbyte(unsigned int);


void gravaMasterClk(void);
void trucadaIndiv(unsigned int,unsigned int);
unsigned int canalPermes(void);
void retError(unsigned int,unsigned int);

void tascaEspera(unsigned int,unsigned int,unsigned int);

void correAmunt(void);
void canviConfig(unsigned int);

void Tx2Oper(unsigned int,unsigned int,unsigned int,unsigned int,char *,unsigned int);

void gestioTxOpers(void);

unsigned int getEstatOperador(unsigned int);



void gestioPetVeu(void);
void gestioIniConfig(void);
void gestioTrucIndiv(void);
void gestioAnulVeu(void);

void gravaDisc(void);
void llegeixDisc(void);

void colocaCmnd(unsigned int,unsigned int *,char *,unsigned int);
void colocaCmnd2(unsigned int,unsigned int *,char *,unsigned int);

void gestioVAX(void);
void Tx2VAX(unsigned int,unsigned int,unsigned int,char *);

void Tx2Radio(unsigned int canal,unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header);

void  gestioTx2Mobil(void);
void gestioNit(void);

char *tractaFitxer(char *);
char *tractaFitxer2(char *fitxer);


void gestioPTTCodis(unsigned int);
void gestioErr();
void gestioSEND_PROG(void);
void gestioCLEARED(void);

unsigned int canalAssociat(unsigned int);

void rstListIndics(unsigned int);

void iniConfigMstClk(unsigned int,unsigned int,unsigned int);

void retardaTasca(void);
void gestioTxPrefixes(void);
void gestioPTTCanal(int,unsigned int);


void gestioObrir(void);

void gestioGrup(void);
int anulGrup();
int defGrup();
int trucGrup();

void gestioFiTrucGrup(void);
void gestioTotPTT(void);
void gestioTst(void);
void gestioCarrega(void);
void conectaPTT(unsigned int);
void desconectaPTT(unsigned int);

void conectaPTT2(void);
void desconectaPTT2(void);
void actNumPetVeu(void);
unsigned int getNumPetVeu(void);

void gestioEstat(void);

char *ompleZeros(char *,unsigned int);
void getExpServei(char *);
void actCanal(int canal);

void rstCanal(unsigned int canal);
void gestioSql(void);

void posaTasca(unsigned int davant,char *info,unsigned int comanda,
					unsigned int proces,unsigned int canal,unsigned int origen,unsigned int estat);

void gestioErrHost(void);
void gestioRxConfig(void);
void gestioSqlOff(void);

void baixaIndicatiu(int posicio);
void desactivaComs(void);
void activaComs(void);
void actLastExec(void);

void gestioOperNit(void);


void indicaTrafic(void);
void creaTaulaMsg(void);
int canalActiu(unsigned int);

extern int inter;

#endif
