  
#include "d:defscgc.h"

extern char **listError;

extern unsigned int bufConfig[ACT_CH][ACT_OPER];

extern unsigned int consoles,canals;

extern unsigned int  comndAct;
extern ptrCmndInfo listCmnd,listCanals;
extern unsigned int lastCmnd; // apuntar… a l'£ltim element de la llista

extern ptrRadioInfo listIndics;
extern unsigned int lastIndic;

extern ptrOperInfo listOpers;

unsigned int numPetVeu=0;

int comHost=NOTOK;

unsigned int indicAct=0,canalAct=0;

unsigned int obrir=CODIS,obrirAnt=CODIS;

unsigned int getNumPetVeu(void)
{
// numPetVeu ‚s una variable global que ens serveix per identificar el
// nombre de servei que s'envia a l'operador.
// El que ens permet ‚s anul.lar aquest servei a altres operador quan un
// operador ja li ha donat perm¡s de veu.
// Aix• passar… quan diversos operadors escoltin el mateix canal.

 return numPetVeu;
}

void actNumPetVeu(void)
{
// actualitza la variable numPetVeu.Llegir getNumPetVeu m‚s infor.
 numPetVeu++;
}


void error(char *tempo)
{
// en cas d'error de mem•ria s'executar… aquesta funci¢ i
// treur… un missatge d'error per pantalla.
 printf(tempo);
 exit(1);
}

int canalOcupat(unsigned int canal)
{
// si el camp listCanals[canal].estat es troba en ESPERA (= canal ocupat)
// la funci¢ retorna Ok, sin¢ NOTOK.

 if (listCanals[canal].estat==ESPERA) return OK; // ocupat
 else return NOTOK; // no ocupat
}



void Tx2Oper(unsigned int operador,unsigned int comanda,unsigned int numProc,
				 unsigned int numbytes,char *missatge,unsigned int ctrlPetVeu)
{

// En els ports 3,4,5 es troben els operadors.
// Segons la variable operador, la comanda anir… dirigida a un port o un altre.
// ctrlPetVeu/numProc s¢n £tils quan el que s'envia als operadors ‚s una comanda
// d'afegir serveis o emergŠncia. numProc cont‚ el nombre de servei o emergŠncia,
// i ens ser… £til per anul.lar-l'ho quan un altre operador ja li hagi donat
// perm¡s de veu.

 int stat=NOTOK;

 switch (operador)
 {
  case PORT3:do{
				  if (ctrlPetVeu==1)
				  {
					 numProc=getNumPetVeu();
					 stat=Tx2Port3(comanda,numProc,numbytes,missatge);
				  }
				  else
					 stat=Tx2Port3(comanda,numProc,numbytes,missatge);

				 }while (stat==NOTOK);
				 if (ctrlPetVeu==1) actNumPetVeu();
				 break;
  case PORT4:do{
				  if (ctrlPetVeu==1)
				  {
					 numProc=getNumPetVeu();
					 stat=Tx2Port3(comanda,numProc,numbytes,missatge);
				  }
				  else
					stat=Tx2Port4(comanda,numProc,numbytes,missatge);
				 }while (stat==NOTOK);
				 if (ctrlPetVeu==1) actNumPetVeu();
				 break;
  case PORT5:do{
				  if (ctrlPetVeu==1)
				  {
					 numProc=getNumPetVeu();
					 stat=Tx2Port3(comanda,numProc,numbytes,missatge);
				  }
				  else
					stat=Tx2Port4(comanda,numProc,numbytes,missatge);
				 }while (stat==NOTOK);
				 if (ctrlPetVeu==1) actNumPetVeu();
				 break;
  default:;
 }
}

void Tx2Radio(unsigned int canal,unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
// En els ports 6,7,8,9,... s¢n de canals de r…dio.
// Segons la variable canal, la comanda anir… dirigida a un canal o altre.
// La variable header controla la transmissi¢ de EC,A1,COMND,... No cal enviar-ho
// quan
 int stat=NOTOK;

 if (comanda==ANU) comanda=0x09;

 switch (canal)
 {
  case PORT6:do{
				 stat=Tx2Port6(comanda,indicatiu,numbytes,missatge,header);
				 }while (stat==NOTOK);
				 break;
  case PORT7:do{
				 stat=Tx2Port7(comanda,indicatiu,numbytes,missatge,header);
				 }while (stat==NOTOK);
				 break;
  case PORT8:do{
				 stat=Tx2Port8(comanda,indicatiu,numbytes,missatge,header);
				 }while (stat==NOTOK);
				 break;
  case PORT9:do{
				 stat=Tx2Port9(comanda,indicatiu,numbytes,missatge,header);
				 }while (stat==NOTOK);
				 break;
  case PORT10:do{
				 stat=Tx2Port10(comanda,indicatiu,numbytes,missatge,header);
				 }while (stat==NOTOK);
				 break;
  default:;
 }
}

unsigned int getEstatOperador(unsigned int oper)
{
// Aquesta funci¢ ens informa de l'estat de l'operador.
 // 0 oper no escolta el canal
 // 1 oper lliure
 // 2 oper ocupat

 int ch=-1;
 unsigned int estat=0,config=4;

 ch=listCmnd[comndAct].origen;
 config=bufConfig[ch][oper];
 switch (config)
 {
  case 1:
				 if (listOpers[oper].estat==ESPERA && ch!=listOpers[oper].canalAss)
				 // no li convindria un canvi de canal en un ratet
				  estat=2;
				 else estat=1;break;
  case 2:   estat=0;break;
  case 3:if (listOpers[oper].estat==ESPERA && ch!=listOpers[oper].canalAss)
				 // no li convindria un canvi de canal en un ratet
				  estat=2;
				 else  estat=1;break;
  case 4:  estat=0;break; // no est… connectat
  default:;
 }
 return (estat);
}



void gestioTxOpers(void)
/*
	Depenent del canal per on hagi vingut la petici¢ d'Incid, SOS o SQL,
	de la configuraci¢ del tŠcnic, i de si l'operador est… ocupat o si n'hi ha un
	d'alternatiu o no; es far… la selecci¢ de l'operador al que va dirigit
	el missatge i s'indicar… en proces.

	El missatge se li enviar… als operadors que escoltin el canal especificat en
	listCmnd[].origen.
*/
{
 unsigned int i=0,actiu=0,exec=0,retarda=0;
 int canal=-1;

 canal=listCmnd[comndAct].origen;
 for (i=0;i<consoles;i++)
 {
  actiu=getEstatOperador(i);
  if (actiu==1)
  // Operador escoltant el canal especificat i lliure
  // En .info tenim indicatiu en ASCII
  {
  // Envia la comanda a MasterClk per llegir estat actual de la connexi¢
	 exec=1;
	 listCmnd[comndAct].estat=ESPERA;
	 Tx2MasterClk(RXESTAT,canal,0,0);

	 tascaEspera();

	 if (listCmnd[comndAct].estat==FIESPERA)
	 {
	  if ((listCmnd[comndAct].proces==0x40) || (listCmnd[comndAct].proces!=listCmnd[comndAct].origen+0x0a))
		/*
			No hi ha connexi¢ o operador es troba escoltant un altre canal.
			Per tant, reprogramem Master Clk.
		*/
		Tx2MasterClk(CONFIG,canal,i,0);
		// per codis l'operador reb l'indicatiu, per silenciador nom‚s cal fer la connexi¢
		if (obrir==CODIS)
		  Tx2Oper(i,listCmnd[comndAct].comnd,0,strlen(listCmnd[comndAct].info),listCmnd[comndAct].info,1);

		listOpers[i].canalAss=listCmnd[comndAct].origen;
	 }

  }
  else
	if (actiu==2) // oper escolta canal per• ocupat en un altre canal, cal retardar la tasca si no s'ha executat
	  retarda=1;
 }
 if (exec==0 && retarda==1) // retarda la tasca
  retardaTasca();

 listCmnd[comndAct].estat=FIEXEC;
}

void correAmunt(void)
{
// Aquesta funci¢ s'executa quan la tasca listCmnd[comndAct] ja s'ha acabat,
// es sap quan listCmnd[comndAct].estat=FIEXEC.
//
 unsigned int i;

 for(i=comndAct;i<lastCmnd;i++)
 {
  listCmnd[i].origen=listCmnd[i+1].origen;
  listCmnd[i].estat=listCmnd[i+1].estat;
  listCmnd[i].comnd=listCmnd[i+1].comnd;
  listCmnd[i].proces=listCmnd[i+1].proces;
  strcpy(listCmnd[i].info,listCmnd[i+1].info);
 }

 listCmnd[lastCmnd].origen=0;
 listCmnd[lastCmnd].estat=NOTRUN;
 listCmnd[lastCmnd].comnd=RES;
 listCmnd[lastCmnd].proces=0;
 strcpy(listCmnd[lastCmnd].info,"");

 if (lastCmnd>0) lastCmnd--;
}


void gravaMasterClk(void)
{
// Serveix per enviar-li al MasterClock la configuraci¢ feta pel tŠcnic.
// Inicialitza totes les conexions.
 unsigned int k,q;

// desactiva interrupcions Rx

 for(k=0;k<consoles;k++)
  for(q=0;q<canals;q++)
	Tx2MasterClk(INI_CONFIG,q,k,0);


// activa interrupcions Rx
}



void tascaEspera()
{
 // Quan s'envia una comanda al MasterClk per conŠixer l'estat de les connexions
 // o per obtenir dades la tasca es queda en espera durant un temps en el que s'espera
 // resposta per part del MasterClk.
 //
 // EL MASTERCLK SEMPRE HA DE CONTESTAR !!!!!!!!!!!!!!

 for(;listCmnd[comndAct].estat!=FIESPERA;);
}


void trucadaIndiv(unsigned int indicatiu,unsigned int canal,
						unsigned int numbytes,char *missatge,unsigned int header)
{
// Transmissi¢ missatge perm¡s de veu o anul.laci¢ veu
//
//	Es realitza la trucada pel canal associat a l'indicatiu.
//	Quan s'executa aquesta funci¢ el canal segur que li ‚s perm‚s
//	a l'operador.
//	Comprova la configuraci¢ de les plaques i la canvia si cal.
//
//
// Envia la comanda a MasterClk per llegir estat actual de la connexi¢.

	 listCmnd[comndAct].estat=ESPERA;
	 Tx2MasterClk(RXESTAT,canal,0,0);

	 tascaEspera();

	 if (listCmnd[comndAct].estat==FIESPERA)
	 {
	  if ((listCmnd[comndAct].proces==0x40) || (listCmnd[comndAct].proces!=listCmnd[comndAct].origen+0x0a))
		/*
			No hi ha connexi¢ o operador es troba en un altre canal.
			Per tant, reprogramem Master Clk.Es d¢na per bo i es truca.
		*/
		Tx2MasterClk(CONFIG,canal,listCmnd[comndAct].origen,0);
	  Tx2Radio(canal,listCmnd[comndAct].comnd,indicatiu,numbytes,missatge,header);
	 }
}


unsigned int canalPermes(void)
{
//
//	Si el m•bil al que s'est… trucant es troba en un dels canals permesos
//	a l'operador, retorna OK. Sin¢ NOTOK.
//
// encarregat de modificar indicAct canalAct

 unsigned int indicatiu,i,permis=4;
 int canal;

 indicatiu=atoi(listCmnd[comndAct].info);
 canal=canalAssociat(indicatiu);

 if (canal!=-1)
 {
  permis=bufConfig[canal][listCmnd[comndAct].origen];
  if (permis==TX || permis==TX_RX)
  {
	 canalAct=canal;
	 indicAct=indicatiu;
	 return OK; // canal programat en Tx
  }
  else return NOTOK;
 }
 else return NOTOK;
}

void retError(unsigned int desti,unsigned int numError)
{
// Aquesta funci¢ retorna error a l'operador.
// En el programa de l'operador el missatge d'error depŠn de numError.

 char tempo[5]="";
 itoa(numError,tempo,10);
 Tx2Oper(desti,ERROR,0,strlen(tempo),tempo,0);
}


void gestioPetVeu(void)
{
// Aquesta funci¢ gestiona el proc‚s de petici¢ de veu quan l'operador ha
// pressionat la tecla F1 "Veu".
// En el cas que la petici¢ de veu hagi estat rebuda per diversos operadors (estaven
// tots escoltant el mateix canal), quan un d'ells prŠm F1 equival a dir que es queda
// amb aquella incidŠncia; aix¡ doncs, queda anul.lada en els altres.
//

 unsigned int proces1,proces2,i=0;
 int posicio=-1;

 if (canalPermes()==OK)
 {
	if (canalOcupat(canalAct)==NOTOK) // lliure
	{
		if (listCmnd[comndAct].comnd==PETVEUS)
		{
		 proces1=MODFSERVEIS; // fa que s'afegeixi asterisc a l'indicatiu
		 proces2=EXTRSERVEIS; // s'anul.la la incidŠncia en altres operadors
		}
		if (listCmnd[comndAct].comnd==PETVEUE)
		{
		 proces1=MODFEMERS; // fa que s'afegeixi asterisc a l'indicatiu
		 proces2=EXTREMERS; // s'anul.la la incidŠncia en altres operadors
		}

 		Tx2Oper(listCmnd[comndAct].origen,proces1,listCmnd[comndAct].proces,strlen(listCmnd[comndAct].info),listCmnd[comndAct].info,0);

		// sïha d'anular el proc‚s en altres operadors.
		for(i=0;i<consoles;i++)
		{ // indicatius encara no porten asterisc
		 if (listCmnd[comndAct].origen!=i && (bufConfig[canalAct][i]==1 || bufConfig[canalAct][i]==3)) // un altre operador escoltant el mateix canal
		  Tx2Oper(listCmnd[comndAct].origen,proces2,listCmnd[comndAct].proces,strlen(listCmnd[comndAct].info),listCmnd[comndAct].info,0);
		}

		listCmnd[comndAct].comnd=PERVEU;
		trucadaIndiv(indicAct,canalAct,0,"",1); // perm¡s veu indicatiu canal


		if ((posicio=posicioIndicatiu(indicAct))!=-1)
		// actualitza la taula d'indicatius
		  listIndics[posicio].permisVeu=1;
	}
	else // retarda tasca
	 retardaTasca();

 }
 else
	retError(listCmnd[comndAct].origen,0); // Canal no permŠs
 listCmnd[comndAct].estat=FIEXEC;
}

void gestioIniConfig(void)
{
// Aquesta funci¢ ‚s executada quan es reb una comanda de inicialitzaci¢
// per part del tŠcnic.
//
// Inicialitza MasterClock
	gravaMasterClk();
// Ho grava al disc.Fitxer CONFIG.CPP
	gravaDisc();
	listCmnd[comndAct].estat=FIEXEC;

}

void gestioTrucIndiv(void)
{
// Quan l'operador vol trucar a un indicatiu(aquest s'ha de trobar en un
// canal perm‚s a l'operador), aquest s'afegeix a la llista d'incidŠncies
// amb asterisc, i se li envia al m•bil un missatge de perm¡s de veu.
//
 char dest[MAX_LONG_INFO];
	if ((canalPermes())==OK)
	{
	  if(canalOcupat(canalAct)==NOTOK)
	  {
		strcpy(dest,listCmnd[comndAct].info);
		strcat(dest," *");
		Tx2Oper(listCmnd[comndAct].origen,MSSERVEIS,0,strlen(dest),dest,0);

		listCmnd[comndAct].comnd=PERVEU;
		trucadaIndiv(indicAct,canalAct,0,"",1);// perm¡s veu indicatiu canal
	  }
	  else // retarda tasca
		retardaTasca();
	}
	else
		retError(listCmnd[comndAct].origen,0); // Canal no permŠs

 listCmnd[comndAct].estat=FIEXEC;
}

void gestioAnulVeu(void)
{
// L'operador ha decidit treure-li el perm¡s de veu a l'indicatiu que tenia
// en llista.
// S'envia un missatge d'anul.laci¢ de perm‚s de veu al m•bil.
//
	unsigned int indicatiu;
	int canal=-1,posicio=-1;

	indicatiu=atoi(listCmnd[comndAct].info);
	canal=canalAssociat(indicatiu);

	if (canal!=-1 && canalOcupat(canal)==NOTOK) // canal lliure
	{
		listCmnd[comndAct].comnd=FIVEU;
		trucadaIndiv(indicatiu,canal,0,"",1);// Fi veu indicatiu canal

		if ((posicio=posicioIndicatiu(indicatiu))!=-1)
		  listIndics[posicio].permisVeu=0;
	}
	else
	 if (canalOcupat(canal)==OK) // retarda tasca
	  retardaTasca();



	listCmnd[comndAct].estat=FIEXEC;
}

void gravaDisc(void)
{
// Grava en el fitxer CONFIG.CPP la configuraci¢ especificada pel tŠcnic.
//
 FILE *desti;
 unsigned int i,j;

 if ((desti=fopen("CONFIG.CPP","wb"))==NULL)
	error("Error: Obrir fitxer dest¡");

 for (i=0;i<canals;i++)
 {
  for(j=0;j<consoles;j++)
  {
	if((fputc((unsigned int)bufConfig[i][j],desti))==EOF)
	  error("Error1: escriptura fitxer desti");
  }
 }

 fclose(desti);

}

void llegeixDisc(void)
{
// Llegeix la configuraci¢ del fitxer CONFIG.CPP.
//
 FILE *desti;
 unsigned int i,j;

 if ((desti=fopen("CONFIG.CPP","rb"))==NULL)
	error("Error: Obrir fitxer dest¡");

 for (i=0;i<canals;i++)
 {
  for(j=0;j<consoles;j++)
  {
	if((bufConfig[i][j]=fgetc(desti))==EOF)
	  error("Error1: lectura fitxer desti");
  }
 }

 fclose(desti);

}

void gestioVAX(void)
{
// Transmissi¢ cap al VAX.
//
 int canal=-1,posicio=-1;
 unsigned int indicatiu;

 // FIN indica que el m•bil ha acabat el servei sense error
 indicatiu=listCmnd[comndAct].proces;
 if (listCmnd[comndAct].comnd==FIN) rstListIndics(indicatiu);

 // si per silenciador i arriba missatge cap al VAX, el deixo passar
 // si l'indicatiu ‚s erroni el VAX provocar… error en la cua canvi d'estat.
 Tx2VAX(listCmnd[comndAct].comnd,indicatiu,strlen(listCmnd[comndAct].info),
		  listCmnd[comndAct].info);

 listCmnd[comndAct].estat=FIEXEC;
}

void gestioNit(void)
{
// Quan el VAX envia una ordre de NIT.
// Si INI el stma queda obert per silenciador.
// Si FIN recupera el mode que tenia programat pel tŠcnic.
//
 unsigned int i;

 if (strcmp(listCmnd[comndAct].info,"INI")==0) // INI
 {
	obrirAnt=obrir;
	obrir=SILENCIADOR;
	for(i=0;i<lastIndic;i++)
	 listIndics[i].permisVeu=1;
	Tx2VAX(NIT,0,0,"");

 }
 else // FIN
 {
	obrir=obrirAnt;
	for(i=0;i<lastIndic;i++)
	{
	 if (obrir==CODIS) listIndics[i].permisVeu=0;
	 else listIndics[i].permisVeu=1;
	}
 }
 listCmnd[comndAct].estat=FIEXEC;
}

void  gestioTx2Mobil(void)
{
// Transmissi¢ dels missatges del VAX cap al m•bil.
//
// En .origen tenim l'indicatiu.
// En .proces el num de blocs que queden per transmetre.
 static unsigned int numBloques=0;
 int canal=-1,canalASI=-1,posicio=-1;
 unsigned int indicatiu,tempo;

 indicatiu=listCmnd[comndAct].origen;
 canal=canalAssociat(indicatiu);

 if (canal!=-1) // segur que ser… !=-1 p.q. s'ha comprovat qe estigui d'alta abans de colocar la tasca en llista
 {
	if (canalOcupat(canal)==NOTOK || (canalOcupat(canal)==OK && listCmnd[comndAct].comnd==listCanals[canal].comnd))
	// canal lliure
	// o ocupat per la tasca que es vol executar
	{
	 switch(listCmnd[comndAct].comnd)
	 {
	 case ASI:
	 case MOD:
	 case PROGINDIC:
	 // en .proces tenim el num de blocs que falten per enviar del fitxer
	 // en .origen l'indicatiu

	 if (listCmnd[comndAct].estat==NOTRUN && canalOcupat(canal)==NOTOK) // canal lliure
	 // header a ST
	 // posa el fitxer del canal en estat ESPERA quan
	 // ST indiqui que ja ha acabat inclour… la tasca en FIESPERA
	 // i el bloc a enviar en el camp .info
	 {
		actCanal(canal);
		Tx2Radio(canal,listCmnd[comndAct].comnd,indicatiu,listCmnd[comndAct].proces,"",1);
	 }
	 else // si canal ocupat i estat de la tasca es FIESPERA ‚s que ens trobem
			// a la meitat d'una transmissi¢ de fitxer
		 if (listCmnd[comndAct].estat==FIESPERA)
		 {
			numBloques=listCmnd[comndAct].proces;
			if (numBloques!=0 && listCmnd[comndAct].estat==FIESPERA)
			{
			 // mentre tinguem fitxer enviarem i esperarem RTS
			 Tx2Radio(canal,listCmnd[comndAct].comnd,0,strlen(listCmnd[comndAct].info),listCmnd[comndAct].info,0);
			 listCanals[canal].proces=--numBloques;
			}
			else// s'ha acabat la transmissi¢ fitxers
			// inicialitzem el canal
				rstCanal(canal);
		 }
	 break;
	 case ANU:
	 // Anul.la un fitxer.
	 // Es el mateix que els casos anteriors, nom‚s que no s'envia informaci¢.
	 // Encara que s'ha d'esperar que s'acabi el proc‚s ja que la placa ST pot
	 // enviar un missatge d'error.
	 // La comanda no es d¢na per acabada fins al final.
		if (listCmnd[comndAct].estat==NOTRUN && canalOcupat(canal)==NOTOK)
		{
		 actCanal(canal);
		 Tx2Radio(canal,listCmnd[comndAct].comnd,indicatiu,0,"",1);
		 listCanals[canal].proces=--listCanals[canal].proces;
		}
		else
		 if (listCmnd[comndAct].estat==FIESPERA)
			rstCanal(canal);
		break;
	 default:break;
	 }
  }
  else
  // canal ocupat per un altre fitxer o una altra cosa
  // retarda tasca
	retardaTasca();

 }
 listCmnd[comndAct].estat=FIEXEC;
}

void rstCanal(unsigned int canal)
{
// Aquesta funci¢ provoca una inicialitzaci¢ de l'estat del canal en
// la listCanals[canal].
// Quan s'han executat comandes ASI,MOD,ANU o PROGINDIC aquesta funci¢ ‚s
// cridada.
	 listCanals[canal].proces=0;
	 listCanals[canal].origen=0;
	 listCanals[canal].comnd=RES;
	 strcpy(listCanals[canal].info,"");
	 listCanals[canal].estat=NOTRUN;
}

char *tractaFitxer(char *fitxer)
{
// En aquesta funci¢ es duu a terme el canvi de format del fitxer ASI transm‚s
// pel VAX. S'afegeix \n al final de cada camp perquŠ ‚s el que espera el m•bil.
//
 unsigned int i,j,camp=0,proxim=0;
 char dest[MAX_FITXER+15+1]="";

 memset(dest,0,MAX_FITXER+15);

 for(i=0,j=0;i<MAX_FITXER && j<(MAX_FITXER+15) && fitxer[i]!='\0';i++)
 {
  if ((i==7 || i==9 || i==13 || i==17 || i==67 || i==82 || i==92 || i==105 || i==109 || i==149

		|| i==179 || i==333 || i==407 || i==410))
	{
	  proxim=0;
	  camp=1;
	}

  if(camp==1 && fitxer[i]==' ') // inici camp i espai en blanc afegeix 0x0a
  {
	dest[j++]=0x0a;
	camp=0;
  }
  else
  {
		if (fitxer[i]!=' ' && camp==0)  // no inici camp i ple de info
		{
			dest[j++]=fitxer[i];
		}
		else
		{
		 if (camp==0) // no inici de camp i espai en blanc
		 {
			if (fitxer[i+1]==' ' && fitxer[i+2]==' ' && proxim==0)
			{
			 dest[j++]=0x0a;
			 proxim=1;
			}
			else
			 if (proxim==0) dest[j++]=fitxer[i];
		 }
		 else // camp=1 inici camp i ple info
		 {
			if (dest[j-1]!=0x0a) dest[j++]=0x0a;
			dest[j++]=fitxer[i];
			camp=0;
		 }
		}
  }
 }
 dest[j]='\n';
 strcpy(fitxer,dest);
 return fitxer;
}

char *tractaFitxer2(char *fitxer)
{
// Es el mateix que tractaFitxer per• pel fitxer MOD.
//
 unsigned int i,j,camp=0,proxim=0;
 char dest[MAX_FITMOD];

 memset(dest,0,MAX_FITMOD);

 for(i=0,j=0;i<MAX_FITMOD && j<MAX_FITMOD+(15) && fitxer[i]!='\0';i++)
 {
  if (i==7 || i==9 || i==13 || i==17 || i==20)
	{
	  proxim=0;
	  camp=1;
	}

  if(camp==1 && fitxer[i]==' ') // inici camp i espai en blanc afegeix 0x0a
  {
	dest[j++]=0x0a;
	camp=0;
  }
  else
  {
		if (fitxer[i]!=' ' && camp==0)  // no inici camp i ple de info
		{
			dest[j++]=fitxer[i];
		}
		else
		{
		 if (camp==0) // no inici de camp i espai en blanc
		 {
			if (fitxer[i+1]==' ' && fitxer[i+2]==' ' && proxim==0)
			{
			 dest[j++]=0x0a;
			 proxim=1;
			}
			else
			 if (proxim==0) dest[j++]=fitxer[i];
		 }
		 else // camp=1 inici camp i ple info
		 {
			if (dest[j-1]!=0xa) dest[j++]=0xa;
			dest[j++]=fitxer[i];
			camp=0;
		 }
		}
  }

 }
 dest[j]='\n';
 strcpy(fitxer,dest);
 return fitxer;

}


void gestioTotPTT(void)
{
// Funci¢ que gestiona PTT.
  unsigned int i,permis=4;

  if (listCmnd[comndAct].proces==3 || listCmnd[comndAct].proces==2 || obrir==CODIS)
  // per codis o
  // trucada de grup amb sistema obert, funciona igual que per indicatius
	gestioPTT();
  else
  // obert per silenciador, transmissi¢ per tots els canals Tx de l'operador
  {
	 for(i=0;i<ACT_CH;i++)
	 {
		  permis=bufConfig[i][listCmnd[comndAct].origen];
		  if (permis==2 || permis==3)
		  {
			if (canalOcupat(i)==NOTOK && listCmnd[comndAct].proces==1) // lliure
			{
					// demanem a MasterClk estat actual de la conexi¢
					// establim la connexi¢ si no hi ‚s i enviem el missatge de
					// perm¡s de veu al m•bil

					listCmnd[comndAct].comnd=PERVEU;
					trucadaIndiv(indicAct,i,0,"",0);

					// Tasca espera l'av¡s de ST que
					// el missatge ja ha estat enviat.
					for(;listCanals[i].estat==ESPERA;); // ASSEGURAR_SE QUE SEMPRE REB DE ST

					// un cop transm‚s conecto PTT de l'operador per la placa MasterClk.
					conectaPTT(i);

			}
			else
				// desconecto PTT de l'operador i envia missatge de FIPERVEU al m•bil.
				if (listCmnd[comndAct].proces==0)
					 desconectaPTT(i);
		  } // de permis
	} // de for Tx
 } // de silenciador
 listCmnd[comndAct].estat=FIEXEC;
}

void conectaPTT(unsigned int canal)
{
//				Tx2MasterClk(TXDADES,canal,listCmnd[comndAct].origen,PTTON);

				Tx2Radio(canal,PTT,0,0,"",1);
				Tx2MasterClk(TXDADES,listCmnd[comndAct].origen+0x0a,listCmnd[comndAct].origen,PTTON);


				listCanals[canalAct].estat=ESPERA; // canal ocupat
				listOpers[listCmnd[comndAct].origen].estat=ESPERA;// oper ocupat
				listOpers[listCmnd[comndAct].origen].ctrlPTT=PTTON;
}

void desconectaPTT(unsigned int canal)
{
// Desactiva PTT de l'operador a trav‚s de la placa MasterClock
// i envia FIPERVEU al m•bil.

// Tx2MasterClk(TXDADES,canal,listCmnd[comndAct].origen,PTTOFF);

 Tx2Radio(canal,NOPTT,0,0,"",1);
 Tx2MasterClk(TXDADES,listCmnd[comndAct].origen+0x0a,listCmnd[comndAct].origen,PTTOFF);

 listCanals[canal].estat=FIESPERA; // canal lliure
 // llibero operador inmediatament perquŠ es tracta de trucada per tota els canals
 listOpers[listCmnd[comndAct].origen].estat=FIESPERA;
 listOpers[listCmnd[comndAct].origen].canalAss=canal;
 listOpers[listCmnd[comndAct].origen].ctrlPTT=PTTOFF;
}

void gestioTst(void)
{
// A l'inici del programa de l'operador es fa un test per provar si el CGC
// est… en marxa. Es considera que si respon ‚s perquŠ tot va b‚.
 listOpers[listCmnd[comndAct].origen].estat=NOTRUN;
 Tx2Oper(listCmnd[comndAct].origen,listCmnd[comndAct].comnd,obrir,0,"",0);
 listCmnd[comndAct].estat=FIEXEC;
}



void gestioPTT(void)
{
 unsigned int indicAnt=indicAct;

 // PTT en sistema tancat surt pel canal de l'indicatiu, en obert per tots
 // els canals programats en Tx per l'operador.
 if (canalPermes()==OK)
 {
	if (canalOcupat(canalAct)==NOTOK && (listCmnd[comndAct].proces==1 || listCmnd[comndAct].proces==3))
	// lliure i PTT ON
	{
		 indicAnt=indicAct;
		 // demanem a MasterClk estat actual de la conexi¢
		 // establim la connexi¢ si no hi ‚s i enviem el missatge de
		 // perm¡s de veu al m•bil
		if (listCmnd[comndAct].proces==3) // PTT ON de trucada de grup
		{
		 indicAnt=indicAct;
		 indicAct=0;
		}

		listCmnd[comndAct].comnd=PERVEU;
		trucadaIndiv(indicAct,canalAct,0,"",0);

		indicAct=indicAnt;

		for(;listCanals[canalAct].estat==ESPERA;); // ASSEGURAR_SE QUE SEMPRE REB DE ST

		conectaPTT2();
	} // de canal lliure
	else // desconecta PTT tot i canal ocupat
		if ((listCmnd[comndAct].proces==0 || listCmnd[comndAct].proces==2))
		  desconectaPTT2();

 }
 else
  retError(listCmnd[comndAct].origen,0); // NO CANAL PERM‚S

 listCmnd[comndAct].estat=FIEXEC;
}

void desconectaPTT2(void)
{
// desactiva PTT de l'operador a trav‚s placa Master Clock
// i s'envia missatge FIPERVEU al m•bil.

//		  Tx2MasterClk(TXDADES,canalAct,listCmnd[comndAct].origen,PTTOFF);

		  Tx2Radio(canalAct,NOPTT,0,0,"",1);
		  Tx2MasterClk(TXDADES,listCmnd[comndAct].origen+0x0a,listCmnd[comndAct].origen,PTTOFF);

		  listCanals[canalAct].estat=FIESPERA; // canal lliure
		  if (listCmnd[comndAct].proces==2)
		  // trucada de grup, llibero l'operador inmediatament
			listOpers[listCmnd[comndAct].origen].estat=FIESPERA;
		  else
		  // operador lliure despr‚s d'un ratet
			listOpers[listCmnd[comndAct].origen].estat=ESPERA;
		  listOpers[listCmnd[comndAct].origen].canalAss=canalAct;
		  listOpers[listCmnd[comndAct].origen].ctrlPTT=PTTOFF;
}

void conectaPTT2(void)
{
				int posicio=-1;
// Tasca espera l'av¡s de ST que el missatge ja ha estat enviat.
// Un cop transm‚s conecto PTT de l'operador per la placa MasterClk
// i s'envia missatge al m•bil de PERVEU.

	if ((posicio=posicioIndicatiu(indicAct))!=-1)
	 listIndics[posicio].permisVeu=1;

//	Tx2MasterClk(TXDADES,canalAct,listCmnd[comndAct].origen,PTTON);
	Tx2Radio(canalAct,PTT,0,0,"",1);
	Tx2MasterClk(TXDADES,listCmnd[comndAct].origen+0x0a,listCmnd[comndAct].origen,PTTON);

	listCanals[canalAct].estat=ESPERA; // canal ocupat
	listOpers[listCmnd[comndAct].origen].estat=ESPERA;   // oper ocupat
// actualitza el canal per on transmetr… l'operador, per defecte l'£ltim
	listOpers[listCmnd[comndAct].origen].canalAss=canalAct;
	listOpers[listCmnd[comndAct].origen].ctrlPTT=PTTON;

}

unsigned int canalAssociat(unsigned int indicatiu)
{
// retorna el canal pel que ha arribat l'indicatiu.

 int posicio=-1;
 if ((posicio=posicioIndicatiu(indicatiu))!=-1) 	  return (listIndics[posicio].canal);
 return -1;
}

void gestioErr(void)
{
 //  en .proces la funci¢ en la que tenim l'error
 //  en .origen l'indicatiu
 //  en .info el num d'expedient i servei

 if (listCmnd[comndAct].comnd==ERR && (listCmnd[comndAct].proces!=ASI && listCmnd[comndAct].proces!=MOD
	  && listCmnd[comndAct].proces!=ANU))
 // error que no va al VAX
 // en aquests moments, nom‚s pot ser de programaci¢ de flota
 // que cal indicar-ho a l'operador.
 // en .proces tenim l'operador al que va dirigit
	retError(listCmnd[comndAct].proces,6);
 // s'avisa a l'operador que no s'ha pogut comunicar amb el m•bil
 else
	Tx2VAX(listCmnd[comndAct].comnd,listCmnd[comndAct].origen,strlen(listCmnd[comndAct].info),
		  listCmnd[comndAct].info);

 rstCanal(canalAssociat(listCmnd[comndAct].origen));
 listCmnd[comndAct].estat=FIEXEC;
}


void actListIndics(unsigned int indicatiu,char *info)
{
// S'actualitza la informaci¢ del nombre d'expedient i servei
// en la llista d'indicatius.
//
 int posicio=-1;
 char expedient[8],servei[3];
 unsigned int i,j,k;

 if ((posicio=posicioIndicatiu(indicatiu))!=-1)
 {
	listIndics[posicio].lastFunc=listCmnd[comndAct].comnd;

	for(i=0;i<7 && info[i]!='\n';i++)
	 expedient[i]=info[i];
	expedient[i]='\0';

	if (info[i]=='\n') i++;

	for(j=i,k=0;k<2 && info[j]!='\n';j++,k++)
	 servei[k]=info[j];
	servei[k]='\0';

	listIndics[posicio].expedient=atol(expedient);
	listIndics[posicio].servei=atoi(servei);
 }

}

void rstListIndics(unsigned int indicatiu)
{
 // missatge FIN, final servei sense error.
 int posicio=-1;
 char expedient[7],servei[2];

 if ((posicio=posicioIndicatiu(indicatiu))!=-1)
 {
	listIndics[posicio].lastFunc=FIN;
	listIndics[posicio].expedient=0;
	listIndics[posicio].servei=0;
	strcpy(listIndics[posicio].fitxer,"");
 }
}

void gestioObrir(void)
{
 unsigned int i=0;
 if (listCmnd[comndAct].proces==0 )
 {
	 obrir=SILENCIADOR;
	 for(i=0;i<lastIndic;i++)
	  listIndics[i].permisVeu=1;
 }
 else
 {
	 obrir=CODIS;
	 for(i=0;i<lastIndic;i++)
		 listIndics[i].permisVeu=0;
 }

 listCmnd[comndAct].estat=FIEXEC;
}





int posicioIndicatiu(unsigned int indicatiu)
{
 unsigned int i,trobat=0;
 for(i=0;i<lastIndic && trobat==0;i++)
  if (listIndics[i].indicatiu==indicatiu)
  {
	trobat=1;
	return i;
  }
 return -1;
}

void retardaTasca(void)
{

 ptrCmndInfo tempo;
 unsigned int i;

 if ((tempo=(ptrCmndInfo)malloc(sizeof(cmndInfo)))==NULL)
  error("FALTA MEM•RIA 8.");


 if (((*tempo).info=(char *)malloc(sizeof(char)*MAX_BLOQUE))==NULL)
		error("FALTA MEM•RIA 6.");

 (*tempo).comnd=listCmnd[comndAct].comnd;
 (*tempo).proces=listCmnd[comndAct].proces;
 (*tempo).estat=listCmnd[comndAct].estat;
 (*tempo).origen=listCmnd[comndAct].origen;
 strcpy((*tempo).info,listCmnd[comndAct].info);

 if (lastCmnd-comndAct>2)
 {
	 for(i=lastCmnd;i>comndAct;i--)
	 {
		 listCmnd[i].comnd=listCmnd[i-1].comnd;
		 listCmnd[i].proces=listCmnd[i-1].proces;
		 listCmnd[i].estat=listCmnd[i-1].estat;
		 listCmnd[i].origen=listCmnd[i-1].origen;
		 strcpy(listCmnd[i].info,listCmnd[i-1].info);
	 }

	 listCmnd[comndAct+2].comnd=(*tempo).comnd;
	 listCmnd[comndAct+2].proces=(*tempo).proces;
	 listCmnd[comndAct+2].estat=(*tempo).estat;
	 listCmnd[comndAct+2].origen=(*tempo).origen;
	 strcpy(listCmnd[comndAct+2].info,(*tempo).info);
 }
 else
 {
	 listCmnd[lastCmnd].comnd=(*tempo).comnd;
	 listCmnd[lastCmnd].proces=(*tempo).proces;
	 listCmnd[lastCmnd].estat=NOTRUN;
	 listCmnd[lastCmnd].origen=(*tempo).origen;
	 strcpy(listCmnd[lastCmnd].info,(*tempo).info);
 }


 if (lastCmnd<MAX_NUM_COMND)
 {
  lastCmnd++;
  listCmnd[lastCmnd].comnd=RES;
  strcpy(listCmnd[lastCmnd].info,"");
  listCmnd[lastCmnd].estat=NOTRUN;
  listCmnd[lastCmnd].origen=0;
  listCmnd[lastCmnd].proces=0;
 }

 free((*tempo).info);
 free(tempo);

}

void gestioTxPrefixes(void)
{
 unsigned int i=0,j=0,k,algun=0;
 char dest[MAX_GRUP*LONG_GRUP];
 char dest2[LONG_GRUP];
 char *stringTrucsGrup[MAX_GRUP];

 for(i=0;i<MAX_GRUP;i++)
  stringTrucsGrup[i]=NULL;

// enviarem els prefixes separats amb /n

 for(i=0,k=0;k<lastIndic && i<MAX_GRUP;k++)
 {
  if (listIndics[k].grup==1)
  {
	stringTrucsGrup[i]=strdup(itoa(listIndics[k].flota,dest2,10));
	i++;
  }
 }

 for(i=0,j=0;i<MAX_GRUP;i++)
 {
  if (stringTrucsGrup[i]==NULL) dest[j++]=0x0a;
  else
  {
	algun=1;
	strcpy(dest+j,stringTrucsGrup[i]);
	j=j+strlen(stringTrucsGrup[i]);
	dest[j++]=0x0a;
  }
 }
 dest[j]='\0';

 if (algun==1)
  Tx2Oper(listCmnd[comndAct].origen,listCmnd[comndAct].comnd,0,strlen(dest),dest,0);
 else retError(listCmnd[comndAct].origen,2);
 listCmnd[comndAct].estat=FIEXEC;
}

void gestioGrup(void)
{
 int stat=NOTOK;
 switch(listCmnd[comndAct].comnd)
 {
  case  ANULGRUP:stat=anulGrup();break;
  case  DEFGRUP: stat=defGrup();break;
  case  TRUCGRUP:stat=trucGrup();break;
  default:;
 }

 if (stat==NOTOK)
 {
	 switch(listCmnd[comndAct].comnd)
	 {
	  case  ANULGRUP:  retError(listCmnd[comndAct].origen,3);break;
	  case  DEFGRUP:   retError(listCmnd[comndAct].origen,4);break;
	  case  TRUCGRUP:  retError(listCmnd[comndAct].origen,5);break;
	  default:;
	 }
 }


 listCmnd[comndAct].estat=FIEXEC;
}

int anulGrup(void)
{
 unsigned int i,permis=4,prefixe,trobat=0,tempo4;
 char dest[7]="";


 prefixe=atoi(listCmnd[comndAct].info);

 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe)
  {
	trobat=1;
	permis=bufConfig[listIndics[i].canal][listCmnd[comndAct].origen];
	if (!(permis==2 || permis==3)) return NOTOK;
  }
 }

 if (trobat==0) return NOTOK;
 // correcte, pot ser que prefixe existeixi o no, pero permisos correctes
 // Per tant s'anul.la el grup.
 // Es coloca en la llista de tasques les trucades de programaci¢
 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe)
  {

			  listCmnd[lastCmnd].comnd=PROGINDIC;
			  listCmnd[lastCmnd].estat=NOTRUN;
			  listCmnd[lastCmnd].origen=listIndics[i].indicatiu;
			  listCmnd[lastCmnd].proces=1; // num blocs

			  itoa(listIndics[i].flotaAnt,dest,10); // nou prefixe el que tenim a flotaAnt
			  if (strlen(dest)<3)
				 strcpy(dest,ompleZeros(dest,3));
			  strcpy(listCmnd[lastCmnd].info,dest);

			  itoa(listCmnd[lastCmnd].origen,dest,10); // indicatiu
			  if (strlen(dest)<3)
				 strcpy(dest,ompleZeros(dest,3));
			  strcat(listCmnd[lastCmnd].info,dest);

			  itoa(listCmnd[comndAct].origen,dest,10); // origen de la comanda
			  if (strlen(dest)<2)
				 strcpy(dest,ompleZeros(dest,2));

			  strcat(listCmnd[lastCmnd].info,dest);

			  if (lastCmnd<MAX_NUM_COMND) lastCmnd++;
  }
 }
 return OK;
}


int trucGrup()
{
 unsigned int prefixe,i,permis=4,trobat=0;
 char dest[LONG_INDIC]="";

 // en .proces tenim l'estat de PTT
 prefixe=atoi(listCmnd[comndAct].info);
 for(i=0;i<lastIndic;i++)
 {
  if (listIndics[i].flota==prefixe)
  {
	  trobat=1;
	  permis=bufConfig[listIndics[i].canal][listCmnd[comndAct].origen];
	  if (!(permis==2 || permis==3))   return NOTOK;
  }
 }

 if (trobat==0) return NOTOK;
 // tot correcte
 // trucada a grup vol dir que tots escolten, per tant gestionem PTT per a tots
 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe)
  {
			  listCmnd[lastCmnd].comnd=PTT;
			  itoa(listIndics[i].indicatiu,dest,10);
			  strcpy(listCmnd[lastCmnd].info,dest);
			  listCmnd[lastCmnd].estat=NOTRUN;
			  listCmnd[lastCmnd].origen=listCmnd[comndAct].origen;
			  listCmnd[lastCmnd].proces=listCmnd[comndAct].proces+2;
			  // per diferenciar-ho de l'individual ja que s'ha de fer servir indicatiu=0
			  // 3 si PTT ON
			  // 2 si PTT OFF

			  if (lastCmnd<MAX_NUM_COMND) lastCmnd++;
  }
 }
 return OK;
}


int defGrup()
{
 char tempo[LONG_GRUP]="";
 unsigned int permis=4,i,length,length3,indicatiu,prefixe,j,trobat=0;
 int canal=-1;
 char tempo2[LONG_INDIC]="";
 char tempo3[MAX_BUFFER_RX]="";
 unsigned tempo4;
 char dest[4]="";



 strcpy(tempo3,listCmnd[comndAct].info);

 if (tempo3[0]=='\n') return NOTOK;

 for (j=0;j<LONG_GRUP && tempo3[j]!='\n';j++) tempo[j]=tempo3[j];
 prefixe=atoi(tempo);
 length3=strlen(tempo3);
 memmove(tempo3,tempo3+strlen(tempo),sizeof(char)*(length3-strlen(tempo)));
 tempo3[length3-strlen(tempo)]='\0';

 for(;length3!=0;)
 {
	for(;(tempo3[0]=='\n') && length3!=0;)
	{
	  strcpy(tempo3,tempo3+1);
	  length3--;
	}

	if (length3!=0)
	{
		for (j=0;j<LONG_INDIC && tempo3[j]!='\n';j++) tempo2[j]=tempo3[j];
		length=strlen(tempo2);
		if (length!=0)
		{
			memmove(tempo3,tempo3+strlen(tempo2),sizeof(char)*(length3-strlen(tempo2)));
			tempo3[length3-strlen(tempo2)]='\0';
		}

		indicatiu=atoi(tempo2);
		strcpy(tempo2,"");

		for(i=0;i<lastIndic;i++)
		{
		  if (listIndics[i].indicatiu==indicatiu)
		  {
			trobat=1;
			// prefixe ja existeix
			if (listIndics[i].flota==prefixe) return NOTOK;
			canal=canalAssociat(indicatiu);
			// canal no perm‚s a l'operador
			if (canal!=-1) permis=bufConfig[canal][listCmnd[comndAct].origen];
			else return NOTOK;
			if (!(permis==2 || permis==3)) return NOTOK;
		  }
		}
		length3=strlen(tempo3);
	}
 }

 if (trobat==0) return NOTOK;
 // tot correcte
 // coloquem trucades de programaci¢ en la llista de tasques per cada indicatiu


 // TORNEM A COMEN€AR

 strcpy(tempo3,listCmnd[comndAct].info);

 for (j=0;j<LONG_GRUP && tempo3[j]!='\n';j++) tempo[j]=tempo3[j];
 length3=strlen(tempo3);
 memmove(tempo3,tempo3+strlen(tempo),sizeof(char)*(length3-strlen(tempo)));
 tempo3[length3-strlen(tempo)]='\0';

 for(;length3!=0;)
 {

	for(;(tempo3[0]=='\n') && length3!=0;)
	{
	  strcpy(tempo3,tempo3+1);
	  length3--;
	}

	if (length3!=0)
	{

	 for (j=0;j<LONG_INDIC && tempo3[j]!='\n';j++) tempo2[j]=tempo3[j];
	 length=strlen(tempo2);
	 if (length!=0)
	 {
		memmove(tempo3,tempo3+strlen(tempo2),sizeof(char)*(length3-strlen(tempo2)));
		tempo3[length3-strlen(tempo2)]='\0';
	 }

	 indicatiu=atoi(tempo2);
	 strcpy(tempo2,"");

	 for(i=0;i<lastIndic;i++)
	 {
		if(listIndics[i].indicatiu==indicatiu)
		{
			  listCmnd[lastCmnd].comnd=PROGINDIC;
			  listCmnd[lastCmnd].estat=NOTRUN;
			  listCmnd[lastCmnd].origen=indicatiu;
			  listCmnd[lastCmnd].proces=1; // num blocs


			  itoa(prefixe,dest,10); // nou prefixe
			  if (strlen(dest)<3)
				 strcpy(dest,ompleZeros(dest,3));
			  strcpy(listCmnd[lastCmnd].info,dest);

			  itoa(listCmnd[lastCmnd].origen,dest,10); // indicatiu
			  if (strlen(dest)<3)
				 strcpy(dest,ompleZeros(dest,3));
			  strcat(listCmnd[lastCmnd].info,dest);

			  itoa(listCmnd[comndAct].origen,dest,10); // origen de la comanda
			  if (strlen(dest)<2)
				 strcpy(dest,ompleZeros(dest,2));

			  strcat(listCmnd[lastCmnd].info,dest);


			  if (lastCmnd<MAX_NUM_COMND) lastCmnd++;
		}
	 }
	 length3=strlen(tempo3);
	}
 }
 return OK;
}



void gestioFiTrucGrup(void)
{
// s'ha d'enviar un missatge d'anul.laci¢ de veu a tots els indicatius
// pertanyents al grup

 unsigned int i,prefixe;
 char dest[LONG_INDIC]="";

 prefixe=atoi(listCmnd[comndAct].info);
 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe)
  {
			  listCmnd[lastCmnd].comnd=ANULVEU;
			  itoa(listIndics[i].indicatiu,dest,10);
			  strcpy(listCmnd[lastCmnd].info,dest);
			  listCmnd[lastCmnd].estat=NOTRUN;
			  listCmnd[lastCmnd].origen=listCmnd[comndAct].origen;
			  listCmnd[lastCmnd].proces=listCmnd[comndAct].proces;

			  if (lastCmnd<MAX_NUM_COMND) lastCmnd++;
  }
 }

 listCmnd[comndAct].estat=FIEXEC;
}

void gestioEstat(void)
{
// format de string que enviarem
// oper\n estat<actiu='1' noactiu=\n>

 char dest[2]="";
 char dest2[4+(ACT_OPER*4)]="99\n";
 unsigned int i,j;

 if (comHost==OK) dest2[3]='1';
 else dest2[3]='\n';

 for(i=0,j=0;i<ACT_OPER;i++)
 {
  itoa(i,dest,10);
  strcat(dest2,dest);
  j=strlen(dest2);
  dest2[j++]='\n';
  if (listOpers[i].estat==NOACTIU) dest2[j++]='\n';
  else dest2[j++]='1';
  strcpy(dest,"");
 }

 Tx2Oper(listCmnd[comndAct].origen,listCmnd[comndAct].comnd,0,strlen(dest2),dest2,0);
 listCmnd[comndAct].estat=FIEXEC;
}

void actCanal(int canal)
{

 // en .proces tenim el num de blocs que falten per enviar del fitxer
 // en .origen l'indicatiu

 unsigned int comanda,numBloques=0,length=0;
 char tempo[2];
 int posicio;

 comanda=listCmnd[comndAct].comnd;

 if ((posicio=posicioIndicatiu(listCmnd[comndAct].origen))!=-1)
 {
 // en listCanals.origen tenim el indicatiu al que va dirigit
 // en listCanals.proces el num de blocs que falten per transmetre

	listCanals[canal].comnd=comanda;
	listCanals[canal].origen=listCmnd[comndAct].origen;

	if (listCmnd[comndAct].comnd==ASI)
	{
		 strcpy(listCanals[canal].info,tractaFitxer(listIndics[posicio].fitxer));
		 actListIndics(listCmnd[comndAct].origen,listIndics[posicio].fitxer);
	}
	else
	 if (listCmnd[comndAct].comnd==MOD)
	 {
		 listCmnd[comndAct].info=tractaFitxer2(listCmnd[comndAct].info);
		 strcpy(listCanals[canal].info,listCmnd[comndAct].info);
		 actListIndics(listCmnd[comndAct].origen,listCanals[canal].info);
	 }
	 else // PROGINDIC o ANU
		{
		 if (listCmnd[comndAct].comnd==ANU)
			 actListIndics(listCmnd[comndAct].origen,listCanals[canal].info);
		 strcpy(listCanals[canal].info,listCmnd[comndAct].info);
		}



	length=strlen(listCanals[canal].info);
	numBloques=(unsigned int)(length/MAX_BLOQUE);

	if (listCmnd[comndAct].comnd==MOD || listCmnd[comndAct].comnd==ASI)
		memset((listCanals[canal].info+length),'\n',MAX_BLOQUE);

	if ((length%MAX_BLOQUE)!=0)
		numBloques++;

	listCanals[canal].proces=numBloques;

	listCmnd[comndAct].proces=numBloques; // cal transmetre el num de blocs en el header
	listCanals[canal].estat=ESPERA;
 }
}



void gestioSql(void)
{  // demana subto per validar el missatge
	// si el subto ‚s correcte en el camp .proces de la llista de canals
	// hi haur… OK, sin¢ un 99.
	// A m‚s es guarda el subto en .origen de la llista de canals

	 unsigned int canal,comanda=RES;

	 canal=listCmnd[comndAct].origen;
	 comanda=listCanals[canal].comnd;

	 if (comanda!=ANU && comanda!=MOD && comanda!=ASI && comanda!=PROGINDIC)
	 {
		 Tx2MasterClk(RXSUBTO,canal,0,0);

		 tascaEspera();

		 if (listCmnd[comndAct].estat==FIESPERA)
		 {
		  if ((listCmnd[comndAct].proces>=1) && (listCmnd[comndAct].proces<=15))
		  {
			listCanals[canal].proces=OK;
			listCanals[canal].origen=listCmnd[comndAct].proces;
		  }
		  else
			listCanals[canal].proces=0;
		 }
	 }

	 listCmnd[comndAct].estat=FIEXEC;
}


void posaTasca(unsigned int davant,char *info,unsigned int comanda,
					int proces,unsigned int origen)
{
	unsigned int i;

	if (davant==0)
	{
		  if (listCmnd[comndAct].comnd!=RES)
		  {
				if ((lastCmnd-comndAct)>=2)
				{
					for(i=lastCmnd;i>comndAct;i--)
					{
						listCmnd[i].origen=listCmnd[i-1].origen;
						listCmnd[i].estat=listCmnd[i-1].estat;
						listCmnd[i].comnd=listCmnd[i-1].comnd;
						listCmnd[i].proces=listCmnd[i-1].proces;
						strcpy(listCmnd[i].info,listCmnd[i-1].info);
					}

					for(i=0;i<MAX_BUFFER_RX && info[i]!='\0';i++)
					 listCmnd[comndAct+1].info[i]=info[i];
					listCmnd[comndAct+1].info[i]='\0';
					listCmnd[comndAct+1].comnd=comanda;
					listCmnd[comndAct+1].proces=proces; // numblocs falten transmetre
					listCmnd[comndAct+1].origen=origen; // l'indicatiu
					listCmnd[comndAct+1].estat=NOTRUN;
				}
				else
				{
					for(i=0;i<MAX_BUFFER_RX && info[i]!='\0';i++)
					 listCmnd[lastCmnd].info[i]=info[i];
					listCmnd[lastCmnd].info[i]='\0';
					listCmnd[lastCmnd].comnd=comanda;
					listCmnd[lastCmnd].proces=proces; // numblocs falten transmetre
					listCmnd[lastCmnd].origen=origen; // l'indicatiu
					listCmnd[lastCmnd].estat=NOTRUN;
				}

		  }
		  else
		  {
			  listCmnd[comndAct].comnd=comanda;
			  strcpy(listCmnd[comndAct].info,info);
			  listCmnd[comndAct].estat=NOTRUN;
			  listCmnd[comndAct].origen=origen;
			  listCmnd[comndAct].proces=proces;
		  }

	}
	else
	{
			  listCmnd[lastCmnd].comnd=comanda;
			  strcpy(listCmnd[lastCmnd].info,info);
			  listCmnd[lastCmnd].estat=NOTRUN;
			  listCmnd[lastCmnd].origen=origen;
			  listCmnd[lastCmnd].proces=proces;

	}

	if (lastCmnd<MAX_NUM_COMND)
	{
	 lastCmnd++;
	 listCmnd[lastCmnd].comnd=RES;
	 strcpy(listCmnd[lastCmnd].info,"");
	 listCmnd[lastCmnd].estat=NOTRUN;
	 listCmnd[lastCmnd].origen=0;
	 listCmnd[lastCmnd].proces=0;
	}

}



void gestioErrHost(void)
{
 sound(1000);
 delay(500);
 nosound();
 delay(50);
 sound(1000);
 delay(500);
 nosound();
 delay(50);
 sound(1000);
 delay(500);
 nosound();
 delay(50);
 sound(1000);
 delay(500);
 nosound();
 delay(50);
 sound(1000);
 delay(500);
 nosound();
 delay(50);
 sound(1000);
 delay(500);
 nosound();
}


char *ompleZeros(char *tempo,unsigned int topeZeros)
{
 unsigned int length=0,i=0;
 char dest[10]="";

 length=strlen(tempo);

 if (length<topeZeros)
 {
  strcpy(dest,tempo);
  for(i=0;i<=(topeZeros-length) && length!=0;i++)
	dest[topeZeros-i-1]=dest[length-i-1];

  for(i=0;i<(topeZeros-length);i++)
	dest[i]='0';

  dest[topeZeros]='\0';
  strcpy(tempo,dest);

 }

 return tempo;
}


void gestioRxConfig(void)
{
 char dest[(ACT_CH*ACT_OPER)+1]="";
 char dest1[2]="";
 unsigned int dada=4,i,j,length=0;

 for (i=0;i<canals;i++)
 {
  for(j=0;j<consoles;j++)
  {
	dada=bufConfig[i][j];
	itoa(dada,dest1,10);
	strcat(dest,dest1);
	strcpy(dest1,"");
  }
 }

 length=strlen(dest);
 dest[length]='\0';

 Tx2Oper(listCmnd[comndAct].origen,listCmnd[comndAct].comnd,0,
			length,dest,0);

 listCmnd[comndAct].estat=FIEXEC;
}


