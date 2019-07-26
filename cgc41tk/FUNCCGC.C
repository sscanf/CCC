
#include "defscgc.h"

extern char **listError;

extern unsigned char bufConfig[ACT_CH][ACT_OPER];
//extern ptrMsgInfo taulaMsg;
extern int RxFich;
extern ErrorFichero;
extern BrkOn;


extern unsigned int consoles,canals;

extern unsigned int  comndAct,lastExec;
extern ptrCmndInfo listCmnd,listCanals,listExec;
extern unsigned int lastCmnd; // apuntar… a l'£ltim element de la llista

extern ptrRadioInfo listIndics;
extern unsigned int lastIndic;
extern int TablaPredefs[7];

extern ptrOperInfo listOpers;

unsigned int numPetVeu=0;

int comHost=NOTOK,tascaRetard=NOTOK;
int OperNit=0;
int inter=1;
int err;

unsigned int indicAct=0,canalAct=0;

unsigned int obrir=SILENCIADOR,obrirAnt=SILENCIADOR,portsOn=1,Nit=0;

void ErrReadFic(void);

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

 FILE *f;
 struct dostime_t t;

 _dos_gettime(&t);

 if ((f=fopen("cgc.log","w+"))!=NULL)
 {
	fprintf (f,"%2d:%02d.%02d,%s\n",t.hour, t.minute,t.second,tempo);
	fclose(f);
 }

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

// En els ports 0,1,2 es troben els operadors.
// Segons la variable operador, la comanda anir… dirigida a un port o un altre.
// ctrlPetVeu/numProc s¢n £tils quan el que s'envia als operadors ‚s una comanda
// d'afegir serveis o emergŠncia. numProc cont‚ el nombre de servei o emergŠncia,
// i ens ser… £til per anul.lar-l'ho quan un altre operador ja li hagi donat
// perm¡s de veu.

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// NOTA IMPORTANT !!!!
// quan ens trobem amb els operadors ocupant els ports 1,2,3 de la
// primera placa, el primer if no hi ha de ser !!!!!!!!!!!!
// El comhost es troba en aquests moments en el port 3. !!!!!!!!!!!!
//

 int stat=NOTOK;

 if (operador>=4)
 {
	 //printf ("Operador n£mero %d ",operador); //Ahora
	 return;
 }

// if (listOpers[operador].estat==NOACTIU) return;

 do{
  if (ctrlPetVeu==1)
	 numProc=getNumPetVeu();
  // L'operador passa a ocupat quan es troba en el bucle de rebots de PTT.
  // Ens esperarem a que acabi, ASSEGURAR_NOS que surt del bucle.
  // !!!!!!!!!!!!!!!!! sempre passa a NO OCUPAT
  for(unsigned long int time=0;listOpers[operador].estat==ESPERA && time<(TIMEOUT*7);time++);

  stat=Tx2PortO(operador,comanda,numProc,numbytes,missatge);
 }while (stat==NOTOK);
 if (ctrlPetVeu==1) actNumPetVeu();
}


void Tx2Radio(unsigned int canal,unsigned int comanda,unsigned int indicatiu,
				 unsigned int numbytes,char *missatge,unsigned int header)
{
// En els ports 3,4,5,6,... s¢n de canals de r…dio.
// Segons la variable canal, la comanda anir… dirigida a un canal o altre.
// La variable header controla la transmissi¢ de EC,A1,COMND,...
// No cal enviar-ho quan es tracta de blocs ASI o MOD.
//
 int stat=NOTOK;

// Quan enviem un missatge per r…dio, primer tallarem la veu als operadors que es
// troben escoltant el canal; enviarem el missatge i despr‚s els
// tornarem a connectar.

/* for(i=0;i<consoles;i++)
 {
  Tx2MasterClk(RXESTAT,i+0x0a,0,0);
  tascaEspera(i+0x0a,0,0);

  if (listExec[0].estat==FIESPERA && listExec[0].proces==canal)
  // oper es troba connectat al canal
  {
	listOpers[i].canalAss=canal;
	listOpers[i].TxRadio=1;
	Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
  }
 }*/

 //printf ("Tx Radio \n"); // Ahora
 if (comanda==ANU) comanda=0x09;
 do{
  stat=Tx2PortR(canal+4,comanda,indicatiu,numbytes,missatge,header);
 }while (stat==NOTOK);


/* for(i=0;i<consoles;i++)
 {
  if (listOpers[i].canalAss==canal && listOpers[i].TxRadio==1)
  {
	listOpers[i].TxRadio=0;
	Tx2MasterClk(CONFIG,listExec[0].origen+0x0a,canal,0);
  }
 }*/

}

unsigned int getEstatOperador(unsigned int oper,unsigned int ch)
{
// Aquesta funci¢ ens informa de l'estat de l'operador.
 // 0 oper no escolta el canal
 // 1 oper lliure
 // 2 oper ocupat

unsigned int estat=0,config=4;

 config=bufConfig[ch][oper];
 switch (config)
 {
  case 1: // Rx
				 if (listOpers[oper].estat==ESPERA) // operador ocupat
				  estat=2;
				 else estat=1;break;
  case 2: // Tx
				 estat=0;break;
  case 3: // Rx&Tx
				if (listOpers[oper].estat==ESPERA) // operador ocupat
				  estat=2;
				else  estat=1;break;
  case 4:  estat=0;break; // configurat com res
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
	listExec[0].origen.
*/
{
 unsigned int i=0,actiu=0,exec=0,retarda=0;
 int canal=-1,posicio=-1;
 char dest[12]=" Canal ";
 char dest2[3]="";
 //char tmp[10]="";
 int x;


 canal=listExec[0].origen;
// if (listExec[0].comnd==MSEMER && canal!=4) strcat (listExec[0].info," *");
 if (listExec[0].comnd==TRUC_SOCI)
 {
	  listExec[0].comnd = MSSERVEIS;
	  strcat (listExec[0].info," \1");
 }

 for (i=0;i<consoles;i++)
 {
  actiu=getEstatOperador(i,canal);
  if (actiu==1)
  {

  // Operador escoltant el canal especificat i lliure
  // En .info tenim l'indicatiu en ASCII
  // Envia la comanda a MasterClk per llegir estat actual de la connexi¢
	 exec=1;
	 listExec[0].estat=ESPERA;
	 Tx2MasterClk(RXESTAT,i+0x0a,0,0);
	 tascaEspera(i+0x0a,0,0);

	 if (listExec[0].estat==FIESPERA)
	 {
	  if (listExec[0].proces!=canal)
	  //   No hi ha connexi¢.Per tant, reprogramem Master Clk.
	  {
		Tx2MasterClk(CONFIG,canal,i+0x0a,0); // entrada, sortida
	  }

	  // per codis l'operador reb l'indicatiu, per silenciador nom‚s cal fer la connexi¢
	  if (listExec[0].comnd!=SQLON) // MSSERVEIS, MSEMER o VEU
	  {

		posicio=posicioIndicatiu(atoi(listExec[0].info));
//		printf ("Pos.%d | Per.Veu %d | ",posicio,listIndics[posicio].permisVeu);
		if (posicio!=-1 && ((listIndics[posicio].permisVeu==0 && listExec[0].comnd==MSSERVEIS) || listExec[0].comnd==MSEMER))
		{
		// si ja t‚ veu no l'afegim a la llista Incid, s¡ en la de SOS
		// actualitza la taula d'indicatius

		Tx2Oper(i,listExec[0].comnd,0,strlen(listExec[0].info),listExec[0].info,1);
//		printf ("Tx Oper %d %d %s\n",i,listExec[0].comnd,listExec[0].info); //Ahora
	  }

	  }
	  else
	  // SqlOn
		if (obrir==SILENCIADOR)
		{
		// NOTA: f¡sicament tenim canal 0,1,2,3,...
		//       per l'operador s¢n el 1,2,3,4,...
		 itoa((listExec[0].origen+1),dest2,10);
		 strcat(dest,dest2);
		 Tx2Oper(i,OPERSQLON,0,strlen(dest),dest,0);
		 strcpy(dest," Canal ");
		 strcpy(dest2,"");
		 // l'operador quedar… assignat al canal i ocupat.
		 listOpers[i].canalAss=canal;
		 listOpers[i].estat=ESPERA;
		}

	}
  }
  else
	if (actiu==2)
	// oper escolta canal per• ocupat en un altre canal,
	// caldria retardar la tasca si no s'ha executat
	  retarda=1;
 }
 if (exec==0 && retarda==1)
 // retarda la tasca, si no s'ha pogut passar la comanda a cap operador
  retardaTasca();
 listExec[0].estat=FIEXEC;
}

void correAmunt(void)
{
// Aquesta funci¢ s'executa quan la tasca listExec[0] ja s'ha acabat,
// ho sabem p.q. listExec[0].estat=FIEXEC.
//
 unsigned int i;


 if (tascaRetard==NOTOK)
 // las tasca s'ha executat
  indicaTrafic();
 else tascaRetard=NOTOK;


 for(i=0;i<lastExec && i<MAX_NUM_EXEC;i++)
 {
  listExec[i].comnd=listExec[i+1].comnd;
  listExec[i].proces=listExec[i+1].proces;
  listExec[i].origen=listExec[i+1].origen;
  listExec[i].estat=listExec[i+1].estat;
  memccpy(listExec[i].info,listExec[i+1].info,0,MAX_FITXER);
 }

 if (lastExec>0 && lastExec<(MAX_NUM_EXEC-1)) lastExec--;

 listExec[lastExec].comnd=RES;
 listExec[lastExec].proces=0;
 listExec[lastExec].origen=0;
 listExec[lastExec].estat=NOTRUN;
 memset(listExec[lastExec].info,'\0',MAX_FITXER);


}


void gravaMasterClk(void)
{
// Serveix per enviar-li al MasterClock la configuraci¢ feta pel tŠcnic.
// Inicialitza totes les conexions.
 unsigned int k,q;

// desactiva interrupcions Rx

 for(k=0;k<consoles;k++)
  for(q=0;q<canals;q++)
	Tx2MasterClk(INI_CONFIG,q,k+0x0a,0);


// activa interrupcions Rx
}



void tascaEspera(unsigned int entrada,unsigned int sortida,unsigned int flag)
{
 //unsigned int long time=0;
 // Quan s'envia una comanda al MasterClk per conŠixer l'estat de les connexions
 // o per obtenir dades la tasca es queda en espera durant un temps en el que s'espera
 // resposta per part del MasterClk.
 //
 // EL MASTERCLK SEMPRE HA DE CONTESTAR !!!!!!!!!!!!!!

 for(;listExec[0].estat!=FIESPERA;);

}


void trucadaIndiv(unsigned int indicatiu,unsigned int canal,
						unsigned int numbytes,char *missatge,unsigned int header)
{
// Transmissi¢ missatge perm¡s de veu o anul.laci¢ veu
//
// Es realitza la trucada pel canal associat a l'indicatiu.
// Quan s'executa aquesta funci¢ el canal segur que li ‚s perm‚s
// a l'operador.
// Comprova la configuraci¢ de les plaques i la canvia si cal.
//
//
// Envia la comanda a MasterClk per llegir estat actual de la connexi¢.

	 listExec[0].estat=ESPERA;
	 Tx2MasterClk(RXESTAT,canal,0,0);

	 tascaEspera(canal,0,0);

	 if (listExec[0].estat==FIESPERA)
	 {
	  if (listExec[0].proces!=listExec[0].origen+0x0a)
	  /*
			No hi ha connexi¢ o operador es troba en un altre canal.
			Per tant, reprogramem Master Clk.Es d¢na per bo i es truca.
	  */
		Tx2MasterClk(CONFIG,listExec[0].origen+0x0a,canal,0);
	  Tx2Radio(canal,listExec[0].comnd,indicatiu,numbytes,missatge,header);
	}
}


unsigned int canalPermes(void)
{
//
// Si el m•bil al que s'est… trucant es troba en un dels canals permesos
// a l'operador, retorna OK. Sin¢ NOTOK.
//
// encarregat de modificar indicAct canalAct

 unsigned int indicatiu,i;
 int canal;
 int posicio;
 int subto;
 int permis=0;
 //static int vez=0;

 indicatiu=atoi(listExec[0].info);
 canal=canalAssociat(indicatiu);

 if (canal!=-1)
 {
  permis=bufConfig[canal][listExec[0].origen];
  if (permis==TX || permis==TX_RX)
  {
	 canalAct=canal;
	 indicAct=indicatiu;
	 return OK; // canal programat en Tx
  }
  else return NOTOK;
 }
 return (0);
}

void retError(unsigned int desti,unsigned int numError)
{
// Aquesta funci¢ retorna error a l'operador.
// En el programa de l'operador el missatge d'error depŠn de numError.

 char tempo[5]="";
  itoa(numError,tempo,10);
  Tx2Oper(desti,ERROR,numError,strlen (listExec[0].info),listExec[0].info,0);
}


void gestioPetVeu(void)
{
// Aquesta funci¢ gestiona el proc‚s de petici¢ de veu quan l'operador ha
// pressionat la tecla F1 "Veu".
// En el cas que la petici¢ de veu hagi estat rebuda per diversos operadors (estaven
// tots escoltant el mateix canal), quan un d'ells prŠm F1 equival a dir que es queda
// amb aquella incidŠncia; aix¡ doncs, queda anul.lada en els altres.
//

 unsigned int proces1,proces2;
 int posicio=-1;


 if (canalPermes()==OK)
 {
	if (canalOcupat(canalAct)==NOTOK)
	{
		if (listExec[0].comnd==PETVEUS)
		{
		 proces1=MODFSERVEIS; // fa que s'afegeixi asterisc a l'indicatiu
		 proces2=EXTRSERVEIS; // s'anul.la la incidŠncia en altres operadors
		}
		if (listExec[0].comnd==PETVEUE)
		{
		 proces1=MODFEMERS; // fa que s'afegeixi asterisc a l'indicatiu
		 proces2=EXTREMERS; // s'anul.la la incidŠncia en altres operadors
		}

		Tx2Oper(listExec[0].origen,proces1,listExec[0].proces,strlen(listExec[0].info),listExec[0].info,0);
		 if (canalAct!=4)
			Tx2Oper(listExec[0].origen,cSEND_PROG,0,strlen(listExec[0].info),listExec[0].info,0);


/*
		// sïha d'anular el proc‚s en altres operadors.
		for(i=0;i<consoles;i++)
		{ // indicatius encara no porten asterisc
		 if (listExec[0].origen!=i && (bufConfig[canalAct][i]==1 || bufConfig[canalAct][i]==3)) // un altre operador escoltant el mateix canal
		  Tx2Oper(listExec[0].origen,proces2,listExec[0].proces,strlen(listExec[0].info),listExec[0].info,0);
		}
*/
		listExec[0].comnd=PERVEU;
		trucadaIndiv(indicAct,canalAct,0,"",1); // perm¡s veu indicatiu canal


		if ((posicio=posicioIndicatiu(indicAct))!=-1)
		{
		// actualitza la taula d'indicatius
		  listIndics[posicio].permisVeu=1;
		  listCanals[canalAct].comnd=PERVEU;
		  itoa (listExec[0].origen,listCanals[canalAct].info,10);
		}
	}
	else // retarda tasca
	 retardaTasca();

 }
 else
	retError(listExec[0].origen,0); // Canal no permŠs
 listExec[0].estat=FIEXEC;
}

void gestioIniConfig(void)
{
// Aquesta funci¢ ‚s executada quan es reb una comanda de inicialitzaci¢
// per part del tŠcnic.
//
// Ho grava al disc.Fitxer CONFIG.CPP
// Es pot fer una inicialitaci¢ global de la matriu del Master Clk,
// encara que no es necessari ja que controlem les connexions abans de Rx i Tx.
	gravaDisc();
	listExec[0].estat=FIEXEC;

}

void gestioTrucIndiv(void)
{
// Quan l'operador vol trucar a un indicatiu(aquest s'ha de trobar en un
// canal perm‚s a l'operador), s'afegeix a la llista d'incidŠncies
// amb asterisc, i se li envia al m•bil un missatge de perm¡s de veu.
//
 char dest[MAX_LONG_INFO];
 int posicio=-1;

 if ((canalPermes())==OK)
 {
	  if (canalOcupat(canalAct)==NOTOK)
	  {
	 memccpy(dest,listExec[0].info,0,MAX_LONG_INFO);
	 strcat(dest," *");
	 Tx2Oper(listExec[0].origen,MSSERVEIS,0,strlen(dest),dest,0);

		listExec[0].comnd=PERVEU;
		trucadaIndiv(indicAct,canalAct,0,"",1);// perm¡s veu indicatiu canal

		if (canalAct!=4)
			Tx2Oper (listExec[0].origen,cSEND_PROG,0,strlen(dest),dest,0);
		else
				Tx2MasterClk(CONFIG,canalAct,listExec[0].origen+0x0a,0); // entrada, sortida


	 if ((posicio=posicioIndicatiu(indicAct))!=-1)
	 {
		// actualitza la taula d'indicatius
		listIndics[posicio].permisVeu=1;
		listCanals[canalAct].comnd=PERVEU;
		itoa (listExec[0].origen,listCanals[canalAct].info,10);
	 }

	  }
	  else // retarda tasca
		retardaTasca();
 }
 else
	 retError(listExec[0].origen,0); // Canal no permŠs

 listExec[0].estat=FIEXEC;
}

void gestioAnulVeu(void)
{
// L'operador ha decidit treure-li el perm¡s de veu a l'indicatiu que tenia
// en llista.
// S'envia un missatge d'anul.laci¢ de perm¡s de veu al m•bil.
//
	unsigned int indicatiu;
	int canal=-1,posicio=-1;
//	static int vez=0;


	indicatiu=atoi(listExec[0].info);
	canal=canalAssociat(indicatiu);

	if (canal!=-1 && canalOcupat(canal)==NOTOK) // canal lliure
	{
	listExec[0].comnd=FIVEU;
		trucadaIndiv(indicatiu,canal,0,"",1);// Fi veu indicatiu canal

		if (canal!=4)
			Tx2Oper(listExec[0].origen,cCLEARED,0,strlen (listExec[0].info),listExec[0].info,0);
		if ((posicio=posicioIndicatiu(indicatiu))!=-1)
		  listIndics[posicio].permisVeu=0;
	}
	else
	 if (canalOcupat(canal)==OK) // retarda tasca
		retardaTasca();
	listExec[0].estat=FIEXEC;

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
 unsigned int salir=0;

 if ((desti=fopen("CONFIG.CPP","rb"))==NULL)
 {
	ErrReadFic();
 }
 else
 {
	 for (i=0;i<canals&&salir==0;i++)
	 {
	  for(j=0;j<consoles&&salir==0;j++)
	  {
		if((bufConfig[i][j]=fgetc(desti))==(unsigned char)EOF)
		{
		  ErrReadFic();
		  salir=1;
		}
	  }
	 }

	 fclose(desti);
  }
}

void ErrReadFic(void)
{
	printf ("\n\n\nATENCION, el fichero de configuraci¢n est  da¤ado o no existe \n");
	printf ("es necesario reconfigurar el sistema.\n\n\n\n");
	printf ("Pulse cualquier tecla para continuar ... \n");


	while (!kbhit())
	{
		for (int n=0;n<10;n++)
		{
			sound (1000);
			delay (100);
			nosound();
			delay (100);
		}
	}

	gravaDisc();
}
void gestioVAX(void)
{
// Transmissi¢ cap al VAX.
//
 //int canal=-1;
 unsigned int indicatiu;

 // FIN indica que el m•bil ha acabat el servei sense error
 indicatiu=listExec[0].proces;
 if (listExec[0].comnd==FIN) rstListIndics(indicatiu);

 // si per silenciador i arriba missatge cap al VAX, el deixo passar
 // si l'indicatiu ‚s erroni el VAX provocar… error en la cua canvi d'estat.
 Tx2VAX(listExec[0].comnd,indicatiu,strlen(listExec[0].info),
		  listExec[0].info);

 listExec[0].estat=FIEXEC;
}
/*
void gestioNit(void)
{
// Quan el VAX envia una ordre de NIT.
// Si INI el sistema queda obert per silenciador.
// Si FIN recupera el mode que tenia programat pel tŠcnic.
//
 unsigned int i;

 if (strcmp(listExec[0].info,"INI")==0 || OperNit==1) // INI
 {
	if (Nit==1)
	{
		obrir=obrirAnt;
		Nit=0;
	}
	else
	{
		Nit=1;
		OperNit=0;
		obrirAnt=obrir;
		obrir=SILENCIADOR;
//    Tx2VAX(NIT,0,0,"");
	}

 }

 for(i=0;i<consoles;i++)
	Tx2Oper(i,ININITOPER,Nit,0,"",0);

 listExec[0].estat=FIEXEC;
}

void gestioOperNit(void)
{
 FILE *desti;
 unsigned int i,j;
 unsigned int buffer[ACT_CH][ACT_OPER];
 OperNit=1;

 gestioNit();
 if (Nit==1)
 {
  // donem perm¡s a l'operador per tots els canals

  for(i=0;i<canals;i++)
  {
	for (j=0;j<consoles;j++)
	{
		bufConfig[i][j]=3;
	}
  }


 }
 else
 // recuperem l'antiga configuraci¢
 {

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
 Tx2Oper(listExec[0].origen,TST,obrir,0,"",0);
 listExec[0].estat=FIEXEC;
}
*/

void gestioNit(void)
{
// Quan el VAX envia una ordre de NIT.
// Si INI el stma queda obert per silenciador.
// Si FIN recupera el mode que tenia programat pel tŠcnic.
//
 unsigned int i;


 if (strcmp(listExec[0].info,"INI")==0) // INI
 {
//	Nit=1;
//	obrirAnt=obrir;
//	obrir=SILENCIADOR;
	Tx2VAX(NIT,0,0,"");

 }
 else // FIN
 {
//	obrir=obrirAnt;
//	Nit=0;
 }


// for(i=0;i<consoles;i++)
//	Tx2Oper(i,ININITOPER,Nit,0,"",0);

 listExec[0].estat=FIEXEC;

}

void gestioOperNit(void)
{
 FILE *desti;
 unsigned int i,j;
 unsigned int buffer[ACT_CH][ACT_OPER];

  if (Nit==1)
  {
	 obrir=obrirAnt;
	 Nit=0;

  }
  else
  {
	 Nit=1;
	obrirAnt=obrir;
	obrir=SILENCIADOR;

  }

 if (Nit==1)
 {
  // donem perm¡s a l'operador per tots els canals

  for(i=0;i<canals;i++)
  {
	for (j=0;j<consoles;j++)
	{
		bufConfig[i][j]=3;
	}
  }

 }
 else
 // recuperem l'antiga configuraci¢
 {
 if ((desti=fopen("CONFIG.CPP","rb"))==NULL)
	error("Error: Obrir fitxer dest¡");

  for (i=0;i<canals;i++)
  {
	for(j=0;j<consoles;j++)
	{
	if((bufConfig[i][j]=fgetc(desti))==(unsigned char)EOF)
	  error("Error1: lectura fitxer desti");
	}
  }
  fclose(desti);


//  for(i=0;i<canals;i++)
//	bufConfig[i][listExec[0].origen]=buffer[i][listExec[0].origen];

 }
//	printf ("%d ",Nit);

	for(i=0;i<consoles;i++)
		Tx2Oper(i,ININITOPER,Nit,0,"",0);

 Tx2Oper(listExec[0].origen,TST,obrir,0,"",0);
 listExec[0].estat=FIEXEC;
}


void  gestioTx2Mobil(void)
{
// Transmissi¢ dels missatges del VAX cap al m•bil.
//
// En .origen tenim l'indicatiu.
// En .proces el num de blocs que queden per transmetre.
 static unsigned int numBloques=0;
 int canal=-1,posicio=-1;
 unsigned int indicatiu,tempo;


 indicatiu=listExec[0].origen;
  canal=canalAssociat(indicatiu);

  if (err)
  {
	err=0;
	posicio=posicioIndicatiu(indicatiu);
	listIndics[posicio].numReTx=NUM_RETX;
	gotoxy (40,5);
	printf ("%d %d",canal,indicatiu);
	rstCanal(canal);
	RxFich=0;
  }
  else
  {

	 if (canal!=-1) // segur que ser… !=-1 p.q. s'ha comprovat qe estigui d'alta abans de colocar la tasca en llista
	 {
		if (canalOcupat(canal)==NOTOK || (canalOcupat(canal)==OK && listExec[0].comnd==listCanals[canal].comnd))
		// canal lliure
		// o ocupat per la tasca que es vol executar
		{
		 switch(listExec[0].comnd)
		 {
		 case ASI:
		 case MOD:
		 case PROGINDIC:
		 // en .proces tenim el num de blocs que falten per enviar del fitxer
		 // en .origen l'indicatiu
		 if (listExec[0].estat==NOTRUN && canalOcupat(canal)==NOTOK) // canal lliure
		 // header a ST
		 // posa el fitxer del canal en estat ESPERA quan
		 // ST indiqui que ja ha acabat inclour… la tasca en FIESPERA
		 // i el bloc a enviar en el camp .info
		 {

			RxFich=1;

//			gotoxy (29,20);
//			printf("Fitxer header %s",indicatiu);

			actCanal(canal);
			Tx2Radio(canal,listExec[0].comnd,indicatiu,listExec[0].proces,"",1);
			sleep(1);
		 }
		 else // si canal ocupat i estat de la tasca es FIESPERA ‚s que ens trobem
				// a la meitat d'una transmissi¢ de fitxer
		 {
			 if (listExec[0].estat==FIESPERA && ErrorFichero==0)
			 {
				numBloques=listExec[0].proces;

				if (numBloques!=0 && listExec[0].estat==FIESPERA)
				{
				 // mentre tinguem fitxer enviarem i esperarem RTS

				 Tx2Radio(canal,listExec[0].comnd,0,strlen(listExec[0].info),listExec[0].info,0);
				 listCanals[canal].proces=--numBloques;
				}
				else// s'ha acabat la transmissi¢ fitxers
				// inicialitzem el canal
				{
					posicio=posicioIndicatiu(indicatiu);
					listIndics[posicio].numReTx=NUM_RETX;
					rstCanal(canal);
					RxFich=0;
				}
			}
		 }
		 break;
		 case ANU:
		 // Anul.la un fitxer.
		 // Es el mateix que els casos anteriors, nom‚s que no s'envia informaci¢.
		 // Encara que s'ha d'esperar que s'acabi el proc‚s ja que la placa ST pot
		 // enviar un missatge d'error.
		 // La comanda no es d¢na per acabada fins al final.
			if (listExec[0].estat==NOTRUN && canalOcupat(canal)==NOTOK)
			{
			 actCanal(canal);
			 Tx2Radio(canal,listExec[0].comnd,indicatiu,0,"",1);
			 listCanals[canal].proces=--listCanals[canal].proces;
			}
			else
			 if (listExec[0].estat==FIESPERA)
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
  }
	 listExec[0].estat=FIEXEC;
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
//	 memset(listCanals[canal].info,'\0',MAX_BLOQUE+MAX_FITXER);
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
			 dest[j++]=' ';
			 proxim=1;
			}
			else
			 if (proxim==0) dest[j++]=fitxer[i];
		 }
		 else // camp=1 inici camp
		 {
			if (dest[j-1]!=0x0a) dest[j++]=0x0a;
			dest[j++]=fitxer[i];
			camp=0;
		 }
	  }
 }
 dest[j]='\n';
 memccpy(fitxer,dest,0,MAX_FITXER);
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
			 dest[j++]=' ';
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
 dest[j]='\n';
 memccpy(fitxer,dest,0,MAX_FITXER);
 return fitxer;

}


void gestioTotPTT(void)
{
// Funci¢ que gestiona PTT.
  unsigned int i,permis=4,ctrlPTT=0,flgCanal=0;
  char dest[MAX_LONG_INFO]="";
  char *tempo;
  int canalTx=-1;

// ctrlPTT valdr… 2 o 3 en cas de trucada de  grup per indica PTTOFF o PTTON
// ctrlPTT valdr… 0 o 1 en comanda de PTT normal, per indicatiu.


  ctrlPTT=listExec[0].proces;
// Sabem que en cas de Tx per canal, ens arribar…: " Canal <num>"
// Del camp .info, ens quedem amb " Canal"
// flgCanal=1 si ‚s " Canal" o 0 en qualsevol altre cas.
  memccpy(dest,listExec[0].info,0,MAX_LONG_INFO);
  dest[6]='\0';
  flgCanal=(!(strcmp(dest," Canal")));
  if (flgCanal)
  {
	// ens quedem nom‚s amb el num de canal
	memccpy(dest,listExec[0].info,0,MAX_LONG_INFO);
	tempo=dest;
	tempo+=7;
	canalTx=atoi(tempo);
	// NOTA: per l'operador es parla de canal 1,2,3,4,...
	//       f¡sicament tenim 0,1,2,3,...
	canalTx--;
  }
  else  canalTx=-1;


  // canalTx=-1 per indicatius o trucada grup, !=-1 per canal.
  // Per canal, ctrlPTT valdr… 0 o 1.
  if (canalTx==-1 || ctrlPTT==3 || ctrlPTT==2)
  // per codis o
  // trucada de grup o indicatiu amb sistema obert, funciona igual que per codis
	gestioPTTCodis(ctrlPTT);
  else
  // Tx pel canal especificat des de l'operador
	 gestioPTTCanal(canalTx,ctrlPTT);


 listExec[0].estat=FIEXEC;
}

void gestioPTTCanal(int canalTx,unsigned int ctrlPTT)
{
 unsigned int permis=4;
 char dest[12]=" Canal ";
 char dest2[3]="";
 int x;


		  permis=bufConfig[canalTx][listExec[0].origen];

		  if (permis==2 || permis==3)
		  {
			 if (canalOcupat(canalTx)==NOTOK && ctrlPTT==1)
			 // Si canal ocupat per fitxer li d¢na prioritat al fitxer
			 // Es a dir, connexi¢ si canal lliure.
			 {

					// demanem a Master  el canal que l'operador escolta
					// ho guardem per tornar-lo a connectar en PTTOff,
					// i en cas de SqlOn doni preferŠncia.
					listExec[0].estat=ESPERA;
					Tx2MasterClk(RXESTAT,listExec[0].origen+0x0a,0,0);
					tascaEspera(listExec[0].origen+0x0a,0,0);
					if (listExec[0].estat==FIESPERA)
					{
					 if (listExec[0].proces==0x040) listOpers[listExec[0].origen].canalAss=-1;
					 else
					 {
					  listOpers[listExec[0].origen].canalAss=listExec[0].proces;
					  itoa((listExec[0].proces+1),dest2,10);
					  strcat(dest,dest2);
					  Tx2Oper(listExec[0].origen,OPERSQLOFF,0,strlen(dest),dest,0);
					 }
					}


					// demanem a MasterClk estat actual de la conexi¢
					// establim la connexi¢ si no est… feta
					listExec[0].estat=ESPERA;
					Tx2MasterClk(RXESTAT,canalTx,0,0);
					tascaEspera(canalTx,0,0);

					if (listExec[0].estat==FIESPERA)
					{
					 if (listExec[0].proces!=listExec[0].origen+0x0a)
					 /*
							No hi ha connexi¢ o operador es troba en un altre canal.
							Per tant, reprogramem Master Clk.
					 */
					  Tx2MasterClk(CONFIG,listExec[0].origen+0x0a,canalTx,0); // entrada,sortida

					 // un cop transm‚s conecto PTT i desconecto operador com a sortida.

					 Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
					 conectaPTT(canalTx);
				  }
//					 gotoxy (x,7+canalTx);
//					 printf ("TX SI");

			 }
			 else
				// desconecto PTT  si el canal es troba connectat per PTT
				if (ctrlPTT==0 && (canalOcupat(canalTx)==NOTOK || listCanals[canalTx].comnd==PTT))
				{
					 desconectaPTT(canalTx);
//						gotoxy (x,7+canalTx);
//						 printf ("TX NO");
				}
				else
					retError(listExec[0].origen,0); // Canal no permŠs
		  } // de permis
		  else
			 if (ctrlPTT==1) retError(listExec[0].origen,0); // Canal no permŠs
}

void conectaPTT(unsigned int canal)
{
	Tx2Radio(canal,PTT,0,0,"",1);
	listCanals[canal].comnd=PTT;
	listCanals[canal].estat=ESPERA; // canal ocupat
	listOpers[listExec[0].origen].estat=ESPERA;// oper parlant
	listOpers[listExec[0].origen].ctrlPTT=PTTON;
}

void desconectaPTT(unsigned int canal)
{
 int canalAnt=-1;
 char dest[12]=" Canal ";
 char dest2[3]="";
 //unsigned int sqlOn=1;

 Tx2Radio(canal,NOPTT,0,0,"",1);
 // desconecto operador com a sortida
 Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
 itoa((canal+1),dest2,10);
 strcat(dest,dest2);
 Tx2Oper(listExec[0].origen,OPERSQLOFF,0,strlen(dest),dest,0);
 strcpy(dest," Canal ");
 strcpy(dest2,"");
 rstCanal(canal);

 // si hi ha activitat en el canal que desconnecto el deixo passar
 // en cas contrari mirarem el canal anterior.

 if ((canalActiu(canal))==OK && (bufConfig[canal][listExec[0].origen]==1 || bufConfig[canal][listExec[0].origen]==3))
 {
	  Tx2MasterClk(CONFIG,canal,listExec[0].origen+0x0a,0); // entrada, sortida

	  itoa((canal+1),dest2,10);
	  strcat(dest,dest2);
	  Tx2Oper(listExec[0].origen,OPERSQLON,0,strlen(dest),dest,0);
	  // l'operador quedar… assignat al canal i ocupat.
	  listOpers[listExec[0].origen].canalAss=canal;
	  listOpers[listExec[0].origen].estat=ESPERA;
 }
 else
 {
		 canalAnt=listOpers[listExec[0].origen].canalAss;
		 if (canalAnt!=-1 && canalActiu(canalAnt)==OK && (bufConfig[canalAnt][listExec[0].origen]==1 || bufConfig[canalAnt][listExec[0].origen]==3))
		 {
				  // l'operador quedar… assignat al canal i ocupat.

				  Tx2MasterClk(CONFIG,canalAnt,listExec[0].origen+0x0a,0); // entrada, sortida

				  itoa((canalAnt+1),dest2,10);
				  strcat(dest,dest2);
				  Tx2Oper(listExec[0].origen,OPERSQLON,0,strlen(dest),dest,0);
				  // l'operador quedar… assignat al canal i ocupat.
				  listOpers[listExec[0].origen].canalAss=canalAnt;
				  listOpers[listExec[0].origen].estat=ESPERA;
		  }
		  else
		  {
					  // llibero operador.
				  listOpers[listExec[0].origen].estat=FIEXEC;
				  listOpers[listExec[0].origen].canalAss=-1;
		  }
 }
 listOpers[listExec[0].origen].ctrlPTT=PTTOFF;
}



void gestioTst(void)
{
	int SndObrir=obrir;
// A l'inici del programa de l'operador es fa un test per provar si el CGC
// est… en marxa. Es considera que si respon ‚s perquŠ tot va b‚.
 listOpers[listExec[0].origen].estat=NOTRUN;

  if (bufConfig[4][listExec[0].origen]!=4) SndObrir=1;

  Tx2Oper(listExec[0].origen,listExec[0].comnd,SndObrir,0,"",0);
 sleep(2);
 Tx2Oper(listExec[0].origen,ININITOPER,Nit,0,"",0);
 listExec[0].estat=FIEXEC;
}



void gestioPTTCodis(unsigned int ctrlPTT)
{
 unsigned int indicAnt=indicAct;
 double TimeOut;

 // PTT en sistema tancat surt nom‚s pel canal de l'indicatiu, en obert per tots
 // els canals programats en Tx per l'operador.
 if (canalPermes()==OK)
 {
	if (canalOcupat(canalAct)==NOTOK  && (ctrlPTT==1 || ctrlPTT==3))
	// lliure i PTT ON
	{

		indicAnt=indicAct;
		if (ctrlPTT==3) // PTT ON de trucada de grup
		{
		 indicAnt=indicAct;
		 indicAct=0;
		}

		// Para compatibilizar el sistrema con la STK:
		// Cuando el operador pulsa PTT encima de un indicativo, transmite
		// siempre la llamada individual, que para la STK es hacer un llamada.
		// Esto no ha de ocurrir, por lo que si el canal corresponde al canal
		// de trunking y ya est  efectuada la conexi¢n entre el indicativo y
		// centeral, solo transmitir  la orden de PTT sin la llamada individual.

		if (canalAct !=4)
		{

			// demanem a MasterClk estat actual de la conexi¢
			// establim la connexi¢ si no hi ‚s i enviem el missatge de
			// perm¡s de veu al m•bil
			listExec[0].comnd=PERVEU;
			trucadaIndiv(indicAct,canalAct,0,"",1);
			indicAct=indicAnt;

			// ASSEGURAR_SE QUE SEMPRE REB DE ST !!!!!!!!!!!!!!!!!!!!
			listCanals[canalAct].estat=ESPERA;
			for(TimeOut=0;((listCanals[canalAct].estat==ESPERA)&& TimeOut<1777340);TimeOut++);

		}
		Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
		conectaPTT2();
	} // de canal lliure
	else // desconecta PTT

		if ((listCanals[canalAct].comnd==PTT || canalOcupat(canalAct)==NOTOK) && (ctrlPTT==0 || ctrlPTT==2))
		{
		  desconectaPTT2();
		}
		else
			retError(listExec[0].origen,0); // Canal no permŠs
 }
 else
  retError(listExec[0].origen,0); // NO CANAL PERM‚S

 listExec[0].estat=FIEXEC;
}

void desconectaPTT2(void)
{
	  Tx2Radio(canalAct,NOPTT,indicAct,0,"",1);
	  Tx2MasterClk(CONFIG,0x40,listExec[0].origen+0x0a,0);
	  rstCanal(canalAct);
	  // alliberem operador
	  listOpers[listExec[0].origen].estat=FIEXEC;
	  listOpers[listExec[0].origen].ctrlPTT=PTTOFF;
}

void conectaPTT2(void)
{
	// connexi¢ PTT
	listCanals[canalAct].comnd=PTT;
	listCanals[canalAct].estat=ESPERA; // canal ocupat
	listOpers[listExec[0].origen].estat=ESPERA;   // oper parlant
	// actualitza el canal per on transmetr… l'operador, per defecte l'£ltim
	listOpers[listExec[0].origen].canalAss=canalAct;
	listOpers[listExec[0].origen].ctrlPTT=PTTON;
	Tx2Radio(canalAct,PTT,indicAct,0,"",1);

}

unsigned int canalAssociat(unsigned int indicatiu)
{
// retorna el canal pel que ha arribat l'indicatiu.

 int posicio=-1;

 if ((posicio=posicioIndicatiu(indicatiu))!=-1)
	return (listIndics[posicio].canal);

 return -1;
}

void gestioSEND_PROG (void)
{
	unsigned int proces1,proces2;
//	int posicio=-1;
	int i;
	Tx2Oper (listExec[0].proces,cSEND_PROG,0,strlen (listExec[0].info),listExec[0].info,0);
	listExec[0].estat=FIEXEC;

}

void gestioTrucSoci(void)
{
 int indicatiu,canal=-1;
 char dest[MAX_LONG_INFO];

 memccpy (dest,listExec[0].info,0,MAX_LONG_INFO);

 Tx2Oper(listExec[0].origen,MODFSERVEIS,listExec[0].proces,strlen(listExec[0].info),listExec[0].info,0);

 dest[strlen (dest)-1] =0;

 indicatiu=atoi (dest);
  canal=canalAssociat(indicatiu);

  listExec[0].proces=canal;
  listExec[0].comnd=TRUCSOCI;

	Tx2Radio(canal,listExec[0].comnd,indicatiu,0,"",1);

	listCanals[canal].comnd=TRUCSOCI;
	itoa (listExec[0].origen,listCanals[canal].info,10);

	listExec[0].estat=FIEXEC;
}

void gestioCLEARED (void)
{
	int indicatiu;
	int posicio;

	indicatiu = atoi (listExec[0].info);
	if ((posicio=posicioIndicatiu(indicatiu))!=-1)
		  listIndics[posicio].permisVeu=0;

	Tx2Oper (listExec[0].proces,cCLEARED,0,strlen (listExec[0].info),listExec[0].info,0);
	listExec[0].estat=FIEXEC;
	desconectaPTT2();
	listOpers[listExec[0].proces].estat=FIESPERA;
}

void gestioErr(void)
{
 int posicio=-1,canal=-1,n;
 int indicatiu;

 //  en .proces la funci¢ en la que tenim l'error
 //  en .origen l'indicatiu
 //  en .info el num d'expedient i servei

 if (listExec[0].comnd==ERR && (listExec[0].proces!=ASI && listExec[0].proces!=MOD
	  && listExec[0].proces!=ANU))
 {
 // error que no va al VAX
 // en aquests moments, nom‚s pot ser de programaci¢ de flota
 // que cal indicar-ho a l'operador.
 // en .proces tenim l'operador al que va dirigit
 // S'avisa a l'operador que no s'ha pogut comunicar amb el m•bil.

		indicatiu = atoi (listExec[0].info);
	if ((posicio=posicioIndicatiu(indicatiu))!=-1)
		  listIndics[posicio].permisVeu=0;

	for (n=0;n<consoles;n++)
		retError(n,6);
 }
 else
	{
		Tx2VAX(listExec[0].comnd,listExec[0].origen,strlen(listExec[0].info),
				 listExec[0].info);
	}

 err=1;
 canal=canalAssociat(listExec[0].origen);
 if (canal>=0 && canal<canals) rstCanal(canal);

 listExec[0].estat=FIEXEC;
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
	listIndics[posicio].lastFunc=listExec[0].comnd;

	for(i=0;i<7 && info[i]!='\n' && info[i]!='\0';i++)
 	 expedient[i]=info[i];
	expedient[i]='\0';

	if (info[i]=='\n') i++;

	for(j=i,k=0;k<2 && info[j]!='\n' && info[j]!='\0';j++,k++)
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

 if ((posicio=posicioIndicatiu(indicatiu))!=-1)
 {
	listIndics[posicio].lastFunc=FIN;
	listIndics[posicio].expedient=0;
	listIndics[posicio].servei=0;
	memset(listIndics[posicio].fitxer,'\0',MAX_FITXER);
	listIndics[posicio].numReTx=NUM_RETX;

 }
}

void gestioObrir(void)
{
 unsigned int i=0;
 // Obre o tanca el stma segons la configuraci¢ del tŠcnic.
 // Informa als operadors del nou estat. Ho fa internament, es nota amb
 // les possibilitats de transmissi¢ d'aquest.
 if (listExec[0].proces==0 )
	 obrir=SILENCIADOR;
 else
	 obrir=CODIS;

 for(i=0;i<consoles;i++)
  Tx2Oper(i,TST,obrir,0,"",0);

 listExec[0].estat=FIEXEC;
}


int posicioIndicatiu(unsigned int indicatiu)
{
 // Retorna la posici¢ de l'indicatiu en la taula listIndics[].
 // En cas de no trobar-lo retorna -1.
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
 // Retarda la comanda p.q s'executi m‚s tard.
 // Normalment, aquesta funci¢ ‚s cridada quan el canal per on s'ha de
 // realitzar la operaci¢ est… ocupat.

 // Hi cap la tasca a retardar i alguna m‚s ?
 // c…rrega d'alguna tasca m‚s, si hi cap
 int byte;

  if (lastExec<(MAX_NUM_EXEC-1)) gestioCarrega();

 // retardem l'actual
  listExec[lastExec].comnd=listExec[0].comnd;
  listExec[lastExec].proces=listExec[0].proces;
  listExec[lastExec].estat=listExec[0].estat;
  listExec[lastExec].origen=listExec[0].origen;
  memccpy(listExec[lastExec].info,listExec[0].info,0,MAX_FITXER);
  actLastExec();
 tascaRetard=OK;


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
	memccpy(dest+j,stringTrucsGrup[i],0,MAX_GRUP*LONG_GRUP);
	j=j+strlen(stringTrucsGrup[i]);
	dest[j++]=0x0a;
  }
 }
 dest[j]='\0';

 if (algun==1)
  Tx2Oper(listExec[0].origen,listExec[0].comnd,0,strlen(dest),dest,0);
 else retError(listExec[0].origen,2);
 listExec[0].estat=FIEXEC;
}

void gestioGrup(void)
{
 int stat=NOTOK;
 switch(listExec[0].comnd)
 {
  case  ANULGRUP:stat=anulGrup();break;
  case  DEFGRUP: stat=defGrup();break;
  case  TRUCGRUP:stat=trucGrup();break;
  default:;
 }

 if (stat==NOTOK)
 {
	 switch(listExec[0].comnd)
	 {
	  case  ANULGRUP:  retError(listExec[0].origen,3);break;
	  case  DEFGRUP:   retError(listExec[0].origen,4);break;
	  case  TRUCGRUP:  retError(listExec[0].origen,5);break;
	  default:;
	 }
 }


 listExec[0].estat=FIEXEC;
}

int anulGrup(void)
{
 unsigned int i,permis=4,prefixe,trobat=0,tempo4,posada=0;
 char dest[7]="";


 prefixe=atoi(listExec[0].info);

 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe)
  {
	trobat=1;
	permis=bufConfig[listIndics[i].canal][listExec[0].origen];
	if (!(permis==2 || permis==3)) return NOTOK;
  }
 }

 if (trobat==0) return NOTOK;
 // correcte, pot ser que prefixe existeixi o no, pero permisos correctes
 // Per tant s'anul.la el grup.
 // Es coloca en la llista de tasques les trucades de programaci¢
 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe && lastExec<(MAX_NUM_EXEC-1))
  {
		posada=1;
		listExec[lastExec].origen=listIndics[i].indicatiu;
		listExec[lastExec].estat=NOTRUN;
		listExec[lastExec].comnd=PROGINDIC;
		listExec[lastExec].proces=1; // numblocs
		itoa(listIndics[i].flotaAnt,dest,10); // nou prefixe el que tenim a flotaAnt
		if (strlen(dest)<3)
		 memccpy(dest,ompleZeros(dest,3),0,7);
		strcpy(listExec[lastExec].info,dest);

		itoa(listExec[lastExec].origen,dest,10); // indicatiu
		if (strlen(dest)<3)
		 memccpy(dest,ompleZeros(dest,3),0,7);
		strcat(listExec[lastExec].info,dest);

		itoa(listExec[0].origen,dest,10); // origen de la comanda
		if (strlen(dest)<2)
		 memccpy(dest,ompleZeros(dest,2),0,7);
		strcat(listExec[lastExec].info,dest);
		actLastExec();

  }
 }
 if (posada==1) return OK;
 else return NOTOK;
}


int trucGrup()
{
 unsigned int prefixe,i,permis=4,trobat=0,posada=0;
 char dest[LONG_INDIC]="";

 // en .proces tenim l'estat de PTT
 prefixe=atoi(listExec[0].info);
 for(i=0;i<lastIndic;i++)
 {
  if (listIndics[i].flota==prefixe)
  {
	  trobat=1;
	  permis=bufConfig[listIndics[i].canal][listExec[0].origen];
	  if (!(permis==2 || permis==3))   return NOTOK;
  }
 }

 if (trobat==0) return NOTOK;
 // tot correcte
 // trucada a grup vol dir que tots escolten, per tant gestionem PTT per a tots
 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe && lastExec<(MAX_NUM_EXEC-1))
  {
		posada=1;
		listExec[lastExec].origen=listExec[0].origen;
		listExec[lastExec].estat=NOTRUN;
		listExec[lastExec].comnd=PTT;
		listExec[lastExec].proces=listExec[0].proces+2;
		itoa(listIndics[i].indicatiu,dest,10);
		memccpy(listExec[lastExec].info,dest,0,LONG_INDIC);
		// per diferenciar-ho de l'individual ja que s'ha de fer servir indicatiu=0
		// 3 si PTT ON
		// 2 si PTT OFF
		actLastExec();

  }
 }
 if (posada==1) return OK;
 else return NOTOK;
}


int defGrup()
{
 char tempo[LONG_GRUP]="";
 unsigned int permis=4,i,length,length3,indicatiu,prefixe,j,trobat=0,posada=0;
 int canal=-1;
 char tempo2[LONG_INDIC]="";
 char tempo3[MAX_BUFFER_RX]="";
 unsigned tempo4;
 char dest[4]="";



 memccpy(tempo3,listExec[0].info,0,MAX_BUFFER_RX);

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
	  memccpy(tempo3,tempo3+1,0,MAX_BUFFER_RX);
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
			if (canal!=-1) permis=bufConfig[canal][listExec[0].origen];
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

 memccpy(tempo3,listExec[0].info,0,MAX_BUFFER_RX);

 for (j=0;j<LONG_GRUP && tempo3[j]!='\n';j++) tempo[j]=tempo3[j];
 length3=strlen(tempo3);
 memmove(tempo3,tempo3+strlen(tempo),sizeof(char)*(length3-strlen(tempo)));
 tempo3[length3-strlen(tempo)]='\0';

 for(;length3!=0;)
 {

	for(;(tempo3[0]=='\n') && length3!=0;)
	{
	  memccpy(tempo3,tempo3+1,0,MAX_BUFFER_RX);
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
		if(listIndics[i].indicatiu==indicatiu && lastExec<(MAX_NUM_EXEC-1))
		{
			posada=1;
			listExec[lastExec].origen=indicatiu;
			listExec[lastExec].estat=NOTRUN;
			listExec[lastExec].comnd=PROGINDIC;
			listExec[lastExec].proces=1; // numblocs


			itoa(prefixe,dest,10); // nou prefixe
			if (strlen(dest)<3)
			 memccpy(dest,ompleZeros(dest,3),0,4);
			memccpy(listExec[lastExec].info,dest,0,4);

			itoa(listExec[lastExec].origen,dest,10); // indicatiu
			if (strlen(dest)<3)
			 memccpy(dest,ompleZeros(dest,3),0,4);
			strcat(listExec[lastExec].info,dest);

			itoa(listExec[0].origen,dest,10); // origen de la comanda
			if (strlen(dest)<2)
			 memccpy(dest,ompleZeros(dest,2),0,4);
			strcat(listExec[lastExec].info,dest);
			actLastExec();
		}
	 }
	 length3=strlen(tempo3);
	}
 }
 if (posada==1) return OK;
 else return NOTOK;
}



void gestioFiTrucGrup(void)
{
// s'ha d'enviar un missatge d'anul.laci¢ de veu a tots els indicatius
// pertanyents al grup

 unsigned int i,prefixe,posada=0;
 char dest[LONG_INDIC]="";

 prefixe=atoi(listExec[0].info);
 for(i=0;i<lastIndic;i++)
 {
  if(listIndics[i].flota==prefixe && lastExec<(MAX_NUM_EXEC-1))
  {
			  posada=1;
			  listExec[lastExec].comnd=ANULVEU;
			  itoa(listIndics[i].indicatiu,dest,10);
			  memccpy(listExec[lastExec].info,dest,0,MAX_FITXER);
			  listExec[lastExec].estat=NOTRUN;
			  listExec[lastExec].origen=listExec[0].origen;
			  listExec[lastExec].proces=listExec[0].proces;
			  actLastExec();

  }
 }
 if (posada==0) retardaTasca();

 listExec[0].estat=FIEXEC;
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

 Tx2Oper(listExec[0].origen,listExec[0].comnd,0,strlen(dest2),dest2,0);
 listExec[0].estat=FIEXEC;
}

void actCanal(int canal)
{

 // en .proces tenim el num de blocs que falten per enviar del fitxer
 // en .origen l'indicatiu

 unsigned int comanda,numBloques=0,length=0,i;
 char tempo[2];
 int posicio=-1;
 char tempo2[MAX_FITXER];
 //char dest[MAX_BLOQUE+1]="";

 listCanals[canal].estat=ESPERA;

 comanda=listExec[0].comnd;

// memset(tempo2,'\0',MAX_FITXER);
 if ((posicio=posicioIndicatiu(listExec[0].origen))!=-1)
 {
 // en listCanals.origen tenim el indicatiu al que va dirigit
 // en listCanals.proces el num de blocs que falten per transmetre

	listCanals[canal].comnd=comanda;
	listCanals[canal].origen=listExec[0].origen;

	if (listExec[0].comnd==ASI)
	{
		setmem (listCanals[canal].info,0,MAX_FITXER);

		 // guardem fitxer ASI original rebut, per possible reTxs
//     strcpy(tempo2,listIndics[posicio].fitxer);
		 // modifiquem el fitxer optimitzant els camps
		 // OJO!! el contingut listIndics[posicio].fitxer ser… modificat,
		 memccpy(listCanals[canal].info,tractaFitxer(listIndics[posicio].fitxer),0,MAX_FITXER);
		 actListIndics(listExec[0].origen,listIndics[posicio].fitxer);
		 // tornem a recuperar el fitxer original per reTx
//     strcpy(listIndics[posicio].fitxer,tempo2);
	}
	else
	 if (listExec[0].comnd==MOD)
	 {
		 // guardem fitxer per possible reTxs
//     strncpy(listIndics[posicio].fitxer,listExec[0].info);
		 listExec[0].info=tractaFitxer2(listExec[0].info);
		 memccpy(listCanals[canal].info,listExec[0].info,0,MAX_FITXER);
		 actListIndics(listExec[0].origen,listCanals[canal].info);
	 }
	 else // PROGINDIC o ANU
		{
//     strncpy(listIndics[posicio].fitxer,listExec[0].info);
		 memccpy(listCanals[canal].info,listExec[0].info,0,MAX_FITXER);
		 if (listExec[0].comnd==ANU)
			 actListIndics(listExec[0].origen,listCanals[canal].info);
		}



	length=strlen(listCanals[canal].info);
	numBloques=(unsigned int)(length/MAX_BLOQUE);


	if (listExec[0].comnd==MOD || listExec[0].comnd==ASI)
		memset((listCanals[canal].info+length),'\n',MAX_BLOQUE);

//	listCanals[canal].info[length]='\n';
//	listCanals[canal].info[length+1]='\0';

	if ((length%MAX_BLOQUE)!=0)
		numBloques++;

	listCanals[canal].proces=numBloques;

	listExec[0].proces=numBloques; // cal transmetre el num de blocs en el header
 }
}



void gestioSql(void)
{  // demana subto
	// si el subto ‚s correcte el guarda en el listCanals[].proces
	// per guardar-lo en listIndics quan arribi missatge

 //	 int x;
	 unsigned int canal,subto=0,sqlOn=1,time2,rt;
	 unsigned long int time=0,timeout;
	 static unsigned long int i=0;

		 sqlOn=1;
	 canal=listExec[0].origen;

	 for (timeout=0;timeout<3;)
	 {
	 for(;sqlOn==1 && time<10;time++)
	 {
			listExec[0].estat=ESPERA;
			Tx2MasterClk(RXSUBTO,canal,0,0);
			tascaEspera(canal,0,0);
			if (listExec[0].estat==FIESPERA)
			 sqlOn=((listExec[0].proces&0x10)>>4);
	 }
		 if (sqlOn==0)
		 {

				listExec[0].estat=ESPERA;
				Tx2MasterClk(RXSUBTO,canal,0,0);
				tascaEspera(canal,0,0);
				if (listExec[0].estat==FIESPERA)
				{
				 subto=listExec[0].proces&0x0f;
				 sqlOn=((listExec[0].proces&0x10)>>4);
				}

	//			  subto=0; // ahora
				 if (((subto==1 && canal==3) || (subto==13 && canal==0) || (subto==15 && canal==1) || (canal==4 && subto==0)
						|| (subto==7 && canal==2)) && sqlOn==0) // tenim subt• b• i sql encara obert

				 {
					subto=listExec[0].proces&0x0f;
					itoa(subto,listExec[0].info,10);
					if (obrir!=CODIS)
						gestioTxOpers();
					timeout=3;
					rt=0;
				 }
//				 else
	//				 rt=1;
	//				 retardaTasca();
		  }
		  else
			timeout++;

	}
	  listExec[0].estat=FIEXEC;
}


void posaTasca(unsigned int davant,char *info,unsigned int comanda,
					unsigned int proces,unsigned int canal,unsigned int origen,unsigned int estat)
{
 unsigned int vegades,posada;

 directvideo=1;

	 for(posada=0,vegades=0;posada==0 && vegades<MAX_NUM_COMND;vegades++)
	 {

	  if (listCmnd[lastCmnd].comnd==RES)
	  {
		posada=1;
		listCmnd[lastCmnd].origen=origen;
		listCmnd[lastCmnd].estat=estat;
		listCmnd[lastCmnd].comnd=comanda;
		listCmnd[lastCmnd].proces=proces;
		listCmnd[lastCmnd].canal=canal;
		memccpy(listCmnd[lastCmnd].info,info,0,MAX_FITXER);
	  }


	  if (lastCmnd<MAX_NUM_COMND) lastCmnd++;
	  else lastCmnd=0;
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

 listExec[0].estat=FIEXEC;
}


char *ompleZeros(char *tempo,unsigned int topeZeros)
{
 unsigned int length=0,i=0;
 char dest[10]="";

 length=strlen(tempo);

 if (length<topeZeros)
 {
  memccpy(dest,tempo,0,10);
  for(i=0;i<=(topeZeros-length) && length!=0;i++)
	dest[topeZeros-i-1]=dest[length-i-1];

  for(i=0;i<(topeZeros-length);i++)
	dest[i]='0';

  dest[topeZeros]='\0';
  memccpy(tempo,dest,0,10);

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

 Tx2Oper(listExec[0].origen,listExec[0].comnd,0,
			length,dest,0);

 listExec[0].estat=FIEXEC;
}


void gestioSqlOff(void)
{
  unsigned int i,permis=4,noConnexio=1,canal;
  char dest[12]=" Canal ";
  char dest2[3]="";
  int x;

  canal=listExec[0].origen;
	  for(i=0;i<ACT_OPER;i++)
	  {
		if (i==0) x=34;
		else
		x=52;
//		gotoxy (x,7+canal);
//		printf ("  ");

		permis=bufConfig[listExec[0].origen][i];
		if (permis==3 || permis==1) // En Rx o Tx/Rx
		{
		 // Mirem si l'operador est… escoltant el canal per on ha vingut la comanda
		 // de SQLOFF.
		 listExec[0].estat=ESPERA;
		 Tx2MasterClk(RXESTAT,i+0x0a,0,0);
		 tascaEspera(i+0x0a,0,0);

		 if (listExec[0].estat==FIESPERA)
		 {
		  // Si operador estava escoltant el canal en el que s'ha tancat sql,
		  // cal desfer la connexi¢.
		  noConnexio=(listExec[0].proces&0x40)>>7;
		  if ((listExec[0].proces&0x0f)==listExec[0].origen || noConnexio==1)
		  {
			Tx2MasterClk(CONFIG,0x40,i+0x0a,0);
			// enviem a l'operador fi d'activitat al canal
			itoa((listExec[0].origen+1),dest2,10);
			strcat(dest,dest2);
			Tx2Oper(i,OPERSQLOFF,0,strlen(dest),dest,0);
			strcpy(dest," Canal ");
			strcpy(dest2,"");
			// alliberem operador;
			listOpers[i].estat=FIEXEC;
			listOpers[i].canalAss=-1;
		  }
		 }
		}
	  }
	listExec[0].estat=FIEXEC;
}

int canalActiu(unsigned int canal)
{
	 unsigned int subto=0,sqlOn=1;
	 unsigned long int time=0;


	 for(;sqlOn==1 && time<10;time++)
	 {
			listExec[0].estat=ESPERA;
			Tx2MasterClk(RXSUBTO,canal,0,0);
			tascaEspera(canal,0,0);
			if (listExec[0].estat==FIESPERA)
			 sqlOn=((listExec[0].proces&0x10)>>4);
	 }
	 if (sqlOn==0)
	 {
		  for(time=0;(!(subto>=1 && subto<=15)) && time<200 && sqlOn==0;time++)
		  {
			listExec[0].estat=ESPERA;
			Tx2MasterClk(RXSUBTO,canal,0,0);
			tascaEspera(canal,0,0);
			if (listExec[0].estat==FIESPERA)
			{
			 subto=listExec[0].proces&0x0f;
			 sqlOn=((listExec[0].proces&0x10)>>4);
			}
		  }

		 if (((subto==7 && canal==3) || (subto==13 && canal==0) || (subto==15 && canal==1)
				|| (subto==0 && canal==2)) && sqlOn==0) // tenim subt• b• i sql encara obert
		 {
			return OK;
		 }
		 else
		 {
		  return NOTOK;
		 }
	 }
	 else
	 {
	  return NOTOK;
	 }
}


void activaComs(void)
{
// unsigned int i=3;

 outp (rci1,0x09);       // Activa interrupciones: modem i recepci¢.
 outp (rci2,0x01);       // Activa interrupciones: recepci¢.
 enable();
}


void desactivaComs(void)
{

 disable();
 outp(rci1,0);
 outp(rci2,0);

}


void gestioCarrega(void)
{

	 if (listCmnd[comndAct].comnd!=RES)
	 {
	  listExec[lastExec].origen=listCmnd[comndAct].origen;
	  listExec[lastExec].estat=listCmnd[comndAct].estat;
	  listExec[lastExec].comnd=listCmnd[comndAct].comnd;
	  listExec[lastExec].proces=listCmnd[comndAct].proces;
	  listExec[lastExec].canal=listCmnd[comndAct].canal;
	  memccpy(listExec[lastExec].info,listCmnd[comndAct].info,0,MAX_FITXER);

	  listCmnd[comndAct].origen=0;
	  listCmnd[comndAct].estat=NOTRUN;
	  listCmnd[comndAct].comnd=RES;
	  listCmnd[comndAct].proces=0;
	  listCmnd[comndAct].canal=0;
	  memset(listCmnd[comndAct].info,'\0',418);
	  actLastExec();
	 }

 if (comndAct<MAX_NUM_COMND) comndAct++;
 else comndAct=0;
}

void netejaSqlOn(unsigned int canal)
{

 unsigned int i,j;

 for(i=0;i<lastExec && i<MAX_NUM_EXEC;i++)
 {
  if (listExec[i].comnd==SQLON && listExec[i].origen==canal)
  {

	for(j=i;j<lastExec && j<MAX_NUM_EXEC;j++)
	{
	 listExec[j].comnd=listExec[j+1].comnd;
	 listExec[j].proces=listExec[j+1].proces;
	 listExec[j].origen=listExec[j+1].origen;
	 listExec[j].estat=listExec[j+1].estat;
	 memccpy(listExec[j].info,listExec[j+1].info,0,MAX_FITXER);
	}
	if (lastExec>0 && lastExec<(MAX_NUM_EXEC-1)) lastExec--;

	listExec[lastExec].comnd=RES;
	listExec[lastExec].proces=0;
	listExec[lastExec].origen=0;
	listExec[lastExec].estat=NOTRUN;
	memset(listExec[lastExec].info,'\0',MAX_FITXER);

  }
 }
}


void actLastExec(void)
{
  if (lastExec<(MAX_NUM_EXEC-1))
  {
	lastExec++;
	listExec[lastExec].comnd=RES;
	memset(listExec[lastExec].info,'\0',MAX_FITXER);
	listExec[lastExec].estat=NOTRUN;
	listExec[lastExec].origen=0;
	listExec[lastExec].proces=0;
  }
}



void indicaTrafic(void)
{
 unsigned int i=0,trobat=0,j=0,indicatiu,flota,comanda=RES,validOrg=0,validFlota=0,validInfo=0;
// unsigned int validIndic=0;
 //char tempo[MAX_MSGTRAFIC]="";
 //char tempo2[LONG_INDIC]="";
 int origen;
// int posicio=-1;

/* comanda=listExec[0].comnd;

 if (comanda!=NIT && comanda!=ERR && comanda!=ASI && comanda!=ANU
	  && comanda!=MOD && comanda!=PROGINDIC && comanda!=ERRHOST)
 {
  validOrg=1;
  // canal o operador
  origen=listExec[0].origen+1;
 }

 if (comanda!=SQLON && comanda!=NIT && comanda!=SQLOFF && comanda!=FINTRUCGRUP
	  && comanda!=DEFGRUP && comanda!=ANULGRUP && comanda!=TRUCGRUP && comanda!=ERRHOST && comanda!=ESTATPTT)
 {
  validIndic=1;
  // canal o operador

  if (comanda==MSSERVEIS || comanda==MSEMER || comanda==ANULVEU || comanda==PETVEUE
		|| comanda==PETVEUS|| comanda==TRUCINDIV)
	 indicatiu=atoi(listExec[0].info);
  else
  {
	 if (comanda==ASI || comanda==ANU || comanda==MOD || comanda==PROGINDIC
		  || comanda==ERR)
	  indicatiu=listExec[0].origen;
	 else
	  indicatiu=listExec[0].proces; // obe ,rep, atu,..
  }
 }

 if ((validIndic==1 || comanda==ANULGRUP || comanda==TRUCGRUP) && comanda!=ERR)
 {
  validFlota=1;
  if (comanda==TRUCINDIV) flota=0;
  else
  {
	if (validIndic==1)
	{
	 posicio=posicioIndicatiu(indicatiu);
	 flota=listIndics[posicio].flota;
	}
	else flota=atoi(listExec[0].info);
  }
 }

 if (comanda==NIT || comanda==FINTRUCGRUP || comanda==SQLON || comanda==DEFGRUP
	  || comanda==OBE || comanda==ERR)
  validInfo=1;

 memset(tempo,'\0',MAX_MSGTRAFIC);

 strcpy(tempo," ");
 for(i=0;i<NUM_MISSATGES && trobat==0;i++)
 // Busquem a la taula de missatges
 {
	if (taulaMsg[i].num==listExec[0].comnd)
	{
	trobat=1;
	strcat(tempo,taulaMsg[i].info);
	strcat(tempo,"   ");

	// flota
	if (validFlota==1)
	{
	 itoa(flota,tempo2,10);
	 strcat(tempo,ompleZeros(tempo2,3));
	}
	else strcat(tempo,"---");
	strcat(tempo,"      ");


	// indicatiu
	if (validIndic==1)
	{
	 itoa(indicatiu,tempo2,10);
	 strcat(tempo,ompleZeros(tempo2,3));
	}
z	else strcat(tempo,"---");

	strcat(tempo,"        ");


	// el canal o operador
	if (validOrg==1)
	{
	 itoa(origen,tempo2,10);
	 strcat(tempo,ompleZeros(tempo2,2));
	}
	else strcat(tempo,"--");
	strcat(tempo,"    ");

	if (validInfo==1)
	{
	 if (strlen(listExec[0].info)>20) listExec[0].info[20]='\0';
	 strcat(tempo,listExec[0].info);
	}
	else strcat(tempo,"---");

	if (strlen(tempo)>MAX_MSGTRAFIC) tempo[MAX_MSGTRAFIC-1]='\0';

	for(j=0;j<consoles;j++)
	  Tx2Oper(j,MSTRAFIC,0,strlen(tempo),tempo,0);

	}
 }*/
}

