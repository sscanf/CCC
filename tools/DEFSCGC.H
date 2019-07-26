
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

#include "d:head-c.h"
#include "process.h"



#define ACT_CH 4
#define ACT_OPER 3

#define HEADER 2

#define HEADER_RADIO 5
#define HEADER_OPER 4
#define HEADER_VAX1 3
#define HEADER_VAX2 3
#define HEADER_VAX 6

#define NOTRUN 0
#define FIEXEC 1
#define ESPERA 2
#define FIESPERA 3
#define NOACTIU 4

#define MAX_BUFFER_RX 60 // Si (ACT_CH*ACT_OPER) es grande MAX_BUFFER_RX=*
								 // o MAX_BLOQUE o MAX_GRUP*LONG_GRUP.S'agafa el m‚s gran.
								 // o LONG_GRUP+8+(LONG_INDIC*8)

#define MAX_NUM_COMND  70
#define MAX_NUM_RADIO 30

#define MAX_LONG_INFO 48 // MAX_BLOQUE es el string m‚s llarg pot hever en listCmnd

#define LONG_INDIC 4

#define MAX_BUFFER_VAX 418 // per la recepcio del VAX i per indics, on es guarda fitxer

#define MAX_BLOQUE 48

#define MAX_GRUP 20
#define LONG_GRUP 3

#define COM1  1
#define COM2  2
#define PORT3 0x0  /* operador  0 */
#define PORT4 0x01 /* operador  1 */
#define PORT5 0x02 /* operador  2 */

#define PORT6 0x00 /* canal r…dio 0 */
#define PORT7 0x01 /* canal r…dio 1 */
#define PORT8 0x02 /* canal r…dio 2 */
#define PORT9 0x03 /* canal r…dio 3 */
#define PORT10 0x04 /* canal r…dio 4*/

#define RX     1
#define TX     2
#define TX_RX  3
#define Res    4

const PTTON = 1;
const PTTOFF =0;

const unsigned long TIMEOUT = 10000;

const OK				=0;
const NOTOK       =-1;

const SILENCIADOR=0;
const CODIS=1;


/* COMANDES */

const RES			=238;

/* OPERADOR -> CGC */

const INI_CONFIG		=227;
const TRUCINDIV	   =222; // trucada individual
const PETVEUE		   =219;
const PETVEUS		   =217; // perm¡s de veu
const ANULVEU        =218; // anul.la perm¡s de veu
const ESTATPTT			=220; // han polsat PTT
const OBRIR				=240;
const FINTRUCGRUP    =241;

const TRUCGRUP		=233;
const DEFGRUP		=234;
const ANULGRUP		=235;

const RXPREFIXES  =199;
const TST			=242;
const ESTAT       =243;

const RXCONFIG    =245;

/* CGC -> OPERADOR */

const MSSERVEIS   =229;
const MSEMER      =231;
const ERROR       =210;
const MODFSERVEIS =230;
// tamb‚ ANULVEU per fer desaparŠixer una Incid que estigui en diferents
// operadors.
const MODFEMERS   =232;
const EXTREMERS   =221;
const EXTRSERVEIS =239;

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


const SQLON		=0x11; // activitat en el canal
const SQLOFF   =0x22; // no activitat en el canal

const PROGINDIC=0x0c;

const PTT = 0x20;
const NOPTT = 0x21;

// VAX -> CGC
const ANU = 0x14;
const ASI = 0x15;
const MOD = 0x16;
const NIT = 23;
const ERRHOST = 24;



const MAX_FITXER = 416+15+1; // Segons def del RACC
const MAX_FITMOD = 26;  // Segons def del RACC
const MAX_ANU	  = 9;
const MAX_NIT	  = 3;


const	bard    = 0x080; /* Registro de acceso al registro divisor 8250   */
const	tdl     = 0x001;
const	lpe     = 0x002;
const	out2    = 0x008;
const	cop1    = 0x021;
const	cop2    = 0x020;

const	irq4	  = 0x010;
const irq3    = 0x00;/* 0xef*/

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


struct cmndInfo
{
 unsigned int comnd;
 unsigned int origen;
 char *info; /* Informaci¢ que necessita la tasca per executar-se */
 unsigned int proces;
 unsigned int estat; /* proces i estat informem de l'estat en que es troba
								l'execuci¢ de la tasca.
							*/
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
};

struct operInfo
{
 unsigned int estat;
 int canalAss;
 unsigned int ctrlPTT;
 unsigned int compta;
};

void execsys(void);
void presenta(void);
unsigned int llegeixCmnd(void);
extern void initPorts(void);
void error(char *);
void tstComndAct(void);

int posicioIndicatiu(unsigned int);


int  IniSio(void);		//Inicializa la SIO
void initPorts(void);

int Tx2Port3(unsigned int,unsigned int,unsigned int, char *);
int Tx2Port4(unsigned int,unsigned int,unsigned int, char *);
int Tx2Port5(unsigned int,unsigned int,unsigned int, char *);
int Tx2Port6(unsigned int,unsigned int,unsigned int,
				 char *,unsigned int);

int Tx2Port7(unsigned int,unsigned int,unsigned int,
				 char *,unsigned int);
int Tx2Port8(unsigned int,unsigned int,unsigned int,
				 char *,unsigned int);
int Tx2Port9(unsigned int,unsigned int,unsigned int,
				 char *,unsigned int);
int Tx2Port10(unsigned int,unsigned int,unsigned int,
				 char *,unsigned int);

void interrupt IntPort3(void);
void interrupt IntPort4(void);
void interrupt IntPort5(void);

void interrupt IntPort6(void);

void interrupt IntFitxer();
void interrupt IntActiu3();

void actListIndics(unsigned int,char *);

void sacabyte(unsigned int ,unsigned int);
void Tx2MasterClk(unsigned int ,unsigned int,unsigned int,unsigned int);
unsigned int inbyte(unsigned int);

void interrupt cojebuff1(...);
void interrupt cojebuff2(...);
void interrupt newTimer(...);

void gravaMasterClk(void);
void trucadaIndiv(unsigned int,unsigned int);
unsigned int canalPermes(void);
void retError(unsigned int,unsigned int);

void tascaEspera();

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


void gestioPTT();
void gestioErr();
unsigned int canalAssociat(unsigned int);

void rstListIndics(unsigned int);

void iniConfigMstClk(unsigned int,unsigned int,unsigned int);

void retardaTasca(void);
void gestioTxPrefixes(void);


void gestioObrir(void);

void gestioGrup(void);
int anulGrup();
int defGrup();
int trucGrup();

void gestioFiTrucGrup(void);
void gestioTotPTT(void);
void gestioTst(void);

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
					int proces,unsigned int origen);

void gestioErrHost(void);
void gestioRxConfig(void);


void baixaIndicatiu(int posicio);

#endif
