

// l'indicatiu de la central ‚s <1111 1111 1111 1> <13 uns>

#include "d:defscgc.h"

char **listError;

unsigned int consoles=ACT_OPER,canals=ACT_CH;

unsigned int  comndAct;
ptrCmndInfo listCmnd;
unsigned int lastCmnd; // apuntar… a la £ltima comanda
ptrCmndInfo listCanals;

ptrRadioInfo listIndics;
unsigned int lastIndic=0;

extern void interrupt (*ViejIntCOM1)(...);   /*vieja interrupci¢n com1:*/
extern void interrupt (*ViejIntCOM2)(...);   /*vieja interrupci¢n com2:*/
extern void interrupt (*ViejIntTIMER)(...);   /*vieja interrupci¢n Timer:*/

ptrOperInfo listOpers;




unsigned int llegeixCmnd(void)
{
 return (listCmnd[comndAct].comnd);
}


void tstComndAct(void)
{
 if (listCmnd[comndAct].estat==FIEXEC)   correAmunt();
}


void execsys(void)
{
 char dest[MAX_LONG_INFO];


 switch (llegeixCmnd())
 {
  case RES: 				break;

  case ERRHOST: gestioErrHost();
					 break;
  case INI_CONFIG:gestioIniConfig();
						break;
  case PETVEUE:
  case PETVEUS:
						gestioPetVeu();
						break;

  case TRUCINDIV: gestioTrucIndiv();
						break;
  case ANULVEU:
						gestioAnulVeu();
						break;
  case MSEMER:
  case MSSERVEIS:
  case VEU:
					  gestioTxOpers();
					  break;
  case SQLON:
				  gestioSql();
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
  case ERR:
					  gestioErr();
					  break;
  case ANU:
  case ASI:
  case PROGINDIC:
  case MOD:
				gestioTx2Mobil();
				break;
  case NIT:
			  gestioNit();
			  break;
  case ESTATPTT:
			  gestioTotPTT();
			  break;
  case OBRIR:
			  gestioObrir();
			  break;

  case TRUCGRUP:
  case DEFGRUP:
  case ANULGRUP:  gestioGrup();
						break;
  case FINTRUCGRUP:
							gestioFiTrucGrup();
							break;

  case RXPREFIXES: gestioTxPrefixes();
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

  default:break;
 }
 tstComndAct();
}


void presenta(void)
{
 clrscr();
 printf("CGCSys... <prem F per acabar>");
 if ((listError=(char **)malloc(sizeof(char *)*1))==NULL)
  error("FALTA MEM•RIA");
}

void creaCmnd(void)
{
 unsigned int i;

 if ((listCmnd=(cmndInfo *)malloc(sizeof(cmndInfo)*(MAX_NUM_COMND+1)))==NULL)
  error("FALTA MEM•RIA 1.");


 if ((listCanals=(cmndInfo *)malloc(sizeof(cmndInfo)*ACT_CH))==NULL)
  error("FALTA MEM•RIA 2.");

 if ((listIndics=(ptrRadioInfo)malloc(sizeof(radioInfo)*(MAX_NUM_RADIO)))==NULL)
  error("FALTA MEM•RIA 3.");

 if ((listOpers=(ptrOperInfo)malloc(sizeof(operInfo)*(ACT_OPER)))==NULL)
  error("FALTA MEM•RIA 4.");

 for(i=0;i<MAX_NUM_RADIO;i++)
 {
  listIndics[i].flota=0;
  listIndics[i].flotaAnt=-1;
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
 }

 for(i=0;i<ACT_OPER;i++)
 {
  listOpers[i].ctrlPTT=PTTOFF;
  listOpers[i].compta=0;
  listOpers[i].estat=NOACTIU;
  listOpers[i].canalAss=-1;
 }

 for(i=0;i<MAX_NUM_COMND+1;i++)
 {
	 listCmnd[i].comnd=RES;
	 listCmnd[i].estat=NOTRUN;
	 listCmnd[i].origen=-1;
	 listCmnd[i].proces=0;

	 if ((listCmnd[i].info=(char *)malloc(sizeof(char)*MAX_BLOQUE))==NULL)
		error("FALTA MEM•RIA 4.");


	 strcpy(listCmnd[i].info,"");
 }

 for(i=0;i<ACT_CH;i++)
 {
	 if ((listCanals[i].info=(char *)malloc(sizeof(char)*(MAX_FITXER+MAX_BLOQUE)))==NULL)
		error("FALTA MEM•RIA 5.");

	 rstCanal(i);
 }


 lastCmnd=0;
 comndAct=0;
}

void restauraInt(void)
{
	setvect(11,ViejIntCOM2); 		/* Redefine vector int.Nueva */
	setvect(12,ViejIntCOM1); 		/* Redefine vector int.Nueva */
	setvect(0x1c,ViejIntTIMER); 		/* Redefine vector int.Nueva */

}


void main()
{
 unsigned int final=0;
 initPorts();
 creaCmnd();
 presenta();
 llegeixDisc();

// gravaDisc();

//	Tx2Radio(0,ANU,1,0,"",1);

//	Tx2Radio(0,PTT,0,0,"",1);
//	Tx2Radio(0,NOPTT,0,0,"",1);


/*	Tx2Oper(PORT3,MSEMER,2,4,"0001",0);
	Tx2Oper(PORT3,MSEMER,3,4,"0002",0);
	Tx2Oper(PORT3,MSSERVEIS,4,4,"0003",0);
	Tx2Oper(PORT3,MSSERVEIS,5,4,"0004",0);
	Tx2Oper(PORT3,MSSERVEIS,6,4,"0005",0);
	Tx2Oper(PORT3,MSSERVEIS,7,4,"0006",0);
	Tx2Oper(PORT3,MSEMER,8,4,"0007",0);*/


// gravaMasterClk();
//  gestioIniConfig();
//  Tx2MasterClk(RXESTAT,0x00,0,0);
//  Tx2Radio(0,0,0,0,"",1);

 do{
  while (!(kbhit()))
	  execsys();
  if ((getche())=='F') final=1;
 }while (final==0);
 restauraInt();
 printf("GCGSys Acaba...");
}