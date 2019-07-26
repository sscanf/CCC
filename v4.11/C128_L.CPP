#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <process.h>
#include <stdlib.h>

#define RXreg  0
#define TXreg  0
#define IERreg 1  // Interrupt Enable
#define IIDreg 2	// Interrupt Identification
#define FCRreg 2	// Fifo control write
#define CRreg  3	// Line Control
#define MCRreg 4	// Modem Control
#define LSRreg 5	// Line Status
#define MSRreg 6	// Modem Status
#define SCRreg 7  // Scratch register
#define LObaud 0	// Divisor Latch (LSB)
#define HIbaud 1	// Divisor Latch (MSB)
#define RDreg  (-1)

const MAX_LEN = 512;

const CERRADO = 0;
const ABIERTO = 1;
const OFF	  = 0;

const INT_MST = 0xff;  // Interrupci¢n por modem status
const INT_TXD = 0x02;  // Interrupci¢n por tx data
const INT_RXL = 0x06;  // Interrupci¢n por rx line status
const INT_RXD = 0x04;  // Interrupci¢n por rx data
const INT_RLS = 0xc0;

const WordLng_5 = 0x0;
const WordLng_6 = 0x1;
const WordLng_7 = 0x2;
const WordLng_8 = 0x3;

const ParityNone=0x0;
const ParityEven=0x18;
const ParityOdd =0x8;

const StopBit_1 =0x0;
const StopBit_2 =0x4;

void muestra (int);
void far IntERROR (void);
void UartRst(int port);
//void far *ServicioInter (int port);
void ServicioInter (int port);
void PonDLAB (int port ,int modo);

int sio_init(int IrqNumber1, int IrqNumber2, int Mbase, int Mintvctr1, int Mintvctr2, long xtal,int NumPorts);
void sio_reset(void);
int sio_getports (void);
int sio_open (int port);
void sio_close (int port);
int sio_getch(int port);
int sio_linput (int port, char *buff, int len, int term);
int sio_putch (int port, int code);
int sio_putb (int port,char *buff, int len);
int sio_write (int port,char *buff, int len);
int sio_lstatus (int port);
int sio_cnt_irq (int port, void (far *func)(void),int cont);
int sio_ofree (int port);
int sio_oqueue (int port);
char sio_rx(int port);
int  sio_lctrl (int port, int mode);
int sio_ifree (int port);

int  sio_modem_irq (int port, void (far *func)());
void UartRst(int port);
void sio_end(void);
int  sio_ioctl (int port, int baud, char parity, int nBits, int nStops);
void sio_tx(int port,char byte);
int sio_flush (int,int);

void interrupt IntMPort1(...);
void interrupt IntMPort2(...);
void interrupt (*ViejIntMOXA1)(...);   /*vieja interrupci¢n com2:*/
void interrupt (*ViejIntMOXA2)(...);   /*vieja interrupci¢n com2:*/

void (far *TablaIntsRXD[16])(void);
void (far *TablaIntsRXL[16])(void);
void (far *TablaIntsMST[16])(void);
void (far *TablaIntsRLS[16])(void);
void iret(void);

int AddrsBase;
int picA = 0;
extern int DebugMode;
int flag=0;

 struct sio
{
	int baud;
	int parity;
	int bits;
	int stops;
	char BufferTx[MAX_LEN];
	char BufferRx[MAX_LEN];
	char estado; // 0 = CERRADO, 1= ABIERTO
	int  cabeza;
	int  cola;
	int  cont;	// Indica cada cuantos bytes ha de generar interrupt.
	int  IntCntRx;
	int  IntCntTx;
	int  ifree;
	int  overlap;	// Modo 0: Cuando el buffer est  lleno no puede
						// ser machacado. El £ltimo byte recibido se perder .
						//
						// Modo 1: Cuando el buffer est  lleno puede ser machacado.
						// El primer byte en el buffer se perder .
} ptrsio[16];

struct Moxa
{
	int base;
	int IntVctr1;
	int IntVctr2;
	long XtalFrec;
	int NumeroIrq1;
	int NumeroIrq2;
	int TotalPlacas;
	int NumPorts;		// N£mero de puertos en cada placa 4 ports
							//									 		  8 ports

} set;

//				 IRQ0 IRQ1 IRQ2 IRQ3 IRQ4 IRQ5 IRQ6 IRQ7

char irq[]= {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};


int sio_init(int IrqNumber1, int IrqNumber2, int Mbase, int Mintvctr1, int Mintvctr2, long xtal,int NumPorts)
{
	char pic,port;
	int  vectr1=0,vectr2=0,n;
	char IrqComp1=0, IrqComp2=0;
	int size;

	if (IrqNumber1==2) vectr1=10;
	if (IrqNumber1==3) vectr1=11;
	if (IrqNumber1==4) vectr1=12;
	if (IrqNumber1==5) vectr1=13;
	if (IrqNumber1==7) vectr1=15;

	if (IrqNumber2==2) vectr2=10;
	if (IrqNumber2==3) vectr2=11;
	if (IrqNumber2==4) vectr2=12;
	if (IrqNumber2==5) vectr2=13;
	if (IrqNumber2==7) vectr2=15;

	IrqComp1=irq[IrqNumber1];
	IrqComp2=irq[IrqNumber2];

		set.base			=Mbase;
		set.IntVctr1	=Mintvctr1;
		set.IntVctr2	=Mintvctr2;
		set.XtalFrec	=xtal;
		set.NumeroIrq1	=IrqNumber1;
		set.NumeroIrq2	=IrqNumber2;
		set.NumPorts	=NumPorts;

	if (IrqNumber1!=0)
	{
		ViejIntMOXA1=getvect (vectr1);  /* Obtiene vector int. antigua */
		setvect(vectr1,IntMPort1); 		  /* Redefine vector int.Nueva */
		set.TotalPlacas=1;
	}

	if (IrqNumber2!=0)
	{
		ViejIntMOXA2=getvect (vectr2);  /* Obtiene vector int. antigua */
		setvect(vectr2,IntMPort2); 		  /* Redefine vector int.Nueva */
		set.TotalPlacas=2;
	}

	for (n=0;n<16;n++)
	{
		TablaIntsRXD[n]=IntERROR;
		TablaIntsRXL[n]=IntERROR;
		TablaIntsRLS[n]=IntERROR;
	}

	if (IrqNumber1!=0)
	{
		pic = inp (0x21);
		pic&=IrqComp1;
		outp (0x21,pic);
	}

	if (IrqNumber2!=0)
	{
		pic = inp (0x21);
		pic&=IrqComp2;
		outp (0x21,pic);
	}

//	if (!(ptrsio= new  (sio[set.TotalPlacas*set.NumPorts]))) //Reservamos memoria para info.
//		return (-3);		  //Error, no hay suficiente memoria

	for (n=0;n<set.TotalPlacas*set.NumPorts;n++)
	{
		ptrsio[n].baud    = 0;
		ptrsio[n].parity  = 0;
		ptrsio[n].bits	   = 0;
		ptrsio[n].stops   = 0;
		ptrsio[n].estado  = CERRADO;
		ptrsio[n].cabeza  = 0;
		ptrsio[n].cola	   = 0;
		ptrsio[n].cont    = 0;
		ptrsio[n].IntCntRx= 0;
		ptrsio[n].IntCntTx= 0;
		ptrsio[n].overlap = 1;
	}

//	sio_reset();
	return (0);
}

// Resetea todos los ports


int sio_overlap (int port, int mode)
{
	if (ptrsio[port].estado==CERRADO) return (-1); // Error, el puerto no est  abierto.
	ptrsio[port].overlap=mode;
	return (0);
}

void sio_reset(void)
{
	int port;

	for (port=0+set.base;port<set.TotalPlacas*set.NumPorts+set.base;port++);
	{
		for (int n=0;n<7;n++)
		inp (port*set.NumPorts+n);
	}
}

// Devuelve el n£mero total de ports que est n instalados.

int sio_getports (void)
{
	return (set.TotalPlacas*set.NumPorts);
}

// Permite interrupciones de tx y rx en la UART especificada en port.

int sio_open (int port)
{
	char ier,n;
	char lcr=0;
	int puerto=(port*set.NumPorts)+set.base;

	lcr = inp (puerto+CRreg);
	lcr&=0x7f;
	outp (puerto+CRreg,lcr);

	ier=inp (puerto+IERreg);
	ier|=0x1;
	outp (puerto+IERreg,ier);		// Permite interrupciones de RX.

	UartRst (port);
	ptrsio[port].estado=ABIERTO;

	return (0);
}

// Devuelve un byte del buffer de recepci¢n.
// Si no hay ningun byte en el buffer espera a que entre uno.

int sio_getch(int port)
{
//	char time1 = peekb (0x40,0x6d);
	char byte;
	char temp1;
	char temp2;
	char temp[10];

	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.
	// Espera que entre un byte

//	while (ptrsio[port].cola==ptrsio[port].cabeza);


	if (ptrsio[port].cola==ptrsio[port].cabeza) return (-4);


	byte = ptrsio[port].BufferRx[ptrsio[port].cola];
	if (++ptrsio[port].cola>=MAX_LEN) ptrsio[port].cola=0;
	ptrsio[port].IntCntRx--;
	return (byte);
}

int sio_linput (int port, char *buff, int len, int term)
{
	int puerto=(port*set.NumPorts)+set.base;

	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.

	memset (buff,0,len);
	for (int n=0;n<=len && buff[n-1]!=term;n++)
		buff[n] = sio_getch (puerto);

	return (n);
}

int sio_read (int port, char *buff, int len)
{
	int puerto=(port*set.NumPorts)+set.base;


	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.
	memset (buff,0,len);
	for (int n=0;n<=len;n++)
		buff[n] = sio_getch (puerto);

	return (n);
}


int sio_putch (int port, int code)
{
	int cont;
	char ier,lcr;
	int puerto=(port*set.NumPorts)+set.base;

	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.

	cont=ptrsio[port].IntCntTx;
	if (cont>=MAX_LEN) return (0); // Error, buffer lleno

	ptrsio[port].BufferTx[cont]=code;
	ptrsio[port].IntCntTx++;

	if (DebugMode==1) muestra (port);

	lcr = inp (puerto+CRreg);
	lcr&=0x7f;
	outp (puerto+CRreg,lcr);

	ier=inp (puerto+IERreg);
	ier|=0x3;
	outp (puerto+IERreg,ier);		// Permite interrupciones de TX e inicializa
											// el proceso de transmisi¢n.
	return (1);
}

void PonDLAB (int port, int modo)
{
	char lcr;

	port= (port*set.NumPorts)+set.base;

	lcr = inp (port+CRreg);

	if (modo==0)
		lcr&=0x7f;
	else
		lcr|=0x80;

	outp (port+CRreg,lcr);
}
int sio_putb (int port, char *buff, int len)
{
	int cont;
	int tmp;
	int n;

	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.

	cont=ptrsio[port].IntCntTx;
	tmp=MAX_LEN-cont;

	if (!tmp || tmp<len) return (0); // Error, buffer lleno
	tmp=1;

	for (n=0;n<=len&&tmp==1;n++)
		tmp=sio_putch (port,buff[n]);

	return (n);
}

int sio_write (int port, char *buff, int len)
{
	int cont;
	int tmp;
	int n;
	unsigned char ier;

	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.

	cont=ptrsio[port].IntCntTx;
	tmp=MAX_LEN-cont;

	if (!tmp) return (0); // Error, buffer lleno
	tmp=1;

	int puerto= (port*set.NumPorts)+set.base;

	PonDLAB (port,OFF);
	ier=inp (puerto+IERreg);
	ier&=0xfd;
	outp (puerto+IERreg,ier);		// Quita interrupciones

	len--;
	for (n=0;n<=len&&tmp==1;n++)
	{
		ptrsio[port].BufferTx[ptrsio[port].IntCntTx]=buff[n];
		ptrsio[port].IntCntTx++;
	}
	if (DebugMode==1) muestra(port);

	ier=inp (puerto+IERreg);
	ier|=2;
	outp (puerto+IERreg,ier);	// Quita interrupciones

	return (n);
}

int sio_flush (int port, int func)
{
	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.

	switch (func)
	{
		case 0:
			ptrsio[port].cola=0;
			ptrsio[port].cabeza=0;
			break;
		case 1:
			ptrsio[port].IntCntTx=0;
			break;
		case 2:
			ptrsio[port].cola=0;
			ptrsio[port].cabeza=0;
			ptrsio[port].IntCntTx=0;
			break;
	}

	return (0);
}

int sio_ifree (int port)
{
	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.
	return (ptrsio[port].IntCntRx);

}

int sio_ofree (int port)
{
	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.
	return (MAX_LEN - ptrsio[port].IntCntTx);
}
int sio_oqueue (int port)
{
	return (ptrsio[port].IntCntTx);
}
int sio_lstatus (int port)
{
	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.
	return (inp (((port*set.NumPorts)+set.base)+MSRreg));
}

int sio_lctrl (int port, int mode)
{
	if (ptrsio[port].estado==CERRADO) return (-5); // Error, el puerto no est  abierto.
	outp (((port*set.NumPorts)+set.base)+MCRreg,mode);
	return (0);
}


void sio_end(void)
{
	if (set.NumeroIrq1!=0) setvect (set.NumeroIrq1,ViejIntMOXA1);
	if (set.NumeroIrq2!=0) setvect (set.NumeroIrq2,ViejIntMOXA2);

//	delete ptrsio;
}

// Esta rutina es interna de las librerias y se ejecuta cuando se detecta un
// error en alguna UART.

void far IntERROR (void)
{
	int port;
/*
	for (port=0;port<16;port++)
		UartRst(port);
*/
	pokeb (0xb800,0,'E');
}


// Resetea la UART y desactiva las interrupciones de la misma.

void sio_close (int port)
{
	PonDLAB (port,OFF);
	UartRst(port);
	outp ((port*set.NumPorts)+set.base+IERreg,0);
}


// Programa la UART especificada en port.
// baud   =  Velocidad en baudios.
// parity =  Paridad ('N' o 'E').
// nBits  =  N£mero de bits de cada byte (8,7,6,5).
// nStops =  N£mero de bits de stop (1,2).

int sio_ioctl (int port, unsigned int baud, char parity, int nBits, int nStops)
{
	int  Port=0;
	char lcr=0;

	int  BaudRate;
	char BaudH;
	char BaudL;

	if (ptrsio[port].estado == CERRADO) return (-1);


	Port = ((port*set.NumPorts)+set.base);

	outp (Port+MCRreg,0x0b);
//	outp (Port+CRreg,0x0b);

	BaudRate = set.XtalFrec / ((long int)baud*16); // Calculamos el Baud Rate.

	if (parity=='n' || parity == 'N') lcr |= ParityNone;
	if (parity=='e' || parity == 'E') lcr |= ParityEven;
	if (parity=='o' || parity == 'O') lcr |= ParityOdd;

	if (nBits==8) lcr |= WordLng_8;
	if (nBits==7) lcr |= WordLng_7;
	if (nBits==6) lcr |= WordLng_6;
	if (nBits==5) lcr |= WordLng_5;

	if (nStops==1) lcr|=StopBit_1;
		else
		 lcr|=StopBit_2;

	BaudH = (BaudRate & 0xff00)>>8;
	BaudL = (BaudRate & 0x00ff);
	lcr|=0x80;
	outp (Port+CRreg,lcr);
	outp (Port+LObaud,BaudL);
	outp (Port+HIbaud,BaudH);

	ptrsio[port].baud   = baud;
	ptrsio[port].parity = parity;
	ptrsio[port].bits   = nBits;
	ptrsio[port].stops  = nStops;

	return (0);
}


// Transmite un byte a la UART especificada en port.
// byte = byte a transmitir.

void sio_tx(int port,char byte)
{
	char lcr=0;
	char lsr=0;
	char ier=0;

	int puerto = ((port*set.NumPorts)+set.base);
	directvideo=1;

	lcr = inp (puerto+CRreg);
	lcr&=0x7f;
	outp (puerto+CRreg,lcr);

	while (lsr==0)
		lsr=inp (puerto+LSRreg) & 0x20;

	outp (puerto+TXreg,byte);

	lsr=0;
//	while (lsr==0)
//		lsr=inp (puerto+LSRreg) & 0x40;
}



char sio_rx(int port)
{
	return (inp ((port*set.NumPorts)+set.base));
}

int sio_cnt_irq (int port, void (far *func)(),int cont)
{
	if (ptrsio[port].estado == CERRADO) return (-1);
	TablaIntsRXD[port]= func;
	ptrsio[port].cont = cont;

	return (0);
}


// Permite interrupciones de modem en la UART especificada en port.
// void (*func)(void)= rutina de gesti¢n de interrupt definida por el usuario.
//
// ej.
// 	 sio_IntEnalbeModemStat (0,MiRutina())
//
// Direcciona la interrupci¢n de modem de la UART 0 a la rutina de interrpt MiRutina().
//
// Se efectuar  una interrupci¢n cada vez que /DTR, /RTS, /OUT 1 y /OUT 2 cambian.

int sio_modem_irq (int port, void (far *func)())
{

	char ier;
	char lcr=0;
	int n;
	int puerto = (port*set.NumPorts)+set.base;

	if (ptrsio[port].estado == CERRADO) return (-1);

	TablaIntsMST[port]=func;

	lcr = inp (puerto+CRreg);
	lcr&=0x7f;
	outp (puerto+CRreg,lcr);

	ier=inp (puerto+IERreg);
	ier|=0x8;
	outp (puerto+IERreg,ier);

	return (0);
}

// Permite interrupciones de l¡nea en la UART especificada en port.
// void (*func)(void)= rutina de gesti¢n de interrupt definida por el usuario.
//
// ej.
// 	 sio_IntEnalbleLineStat (0,MiRutina())
//
// Direcciona la interrupci¢n de l¡nea de la UART 0 a la rutina de interrpt MiRutina().
//
// Se efectuar  una interrupci¢n cada vez que DR, OE, PE, FE, BI, THRE, TEMT cambian.
//
// DR   = Data ready
// OE   = Overrun Error
// PE   = Parity Error
// FE   = Framinig Error
// BI   = Break Interrupt
// THRE = Transmitter Holding Register Empty
// TEMT = Transmitter empty indicator.

void sio_IntEnableLineStat (int port,void (far *func)(void))
{
	char ier;
	char lcr=0;
	int n;
	int puerto=(port*set.NumPorts)+set.base;

	TablaIntsRXL[port]=func;

	lcr = inp (puerto+CRreg);
	lcr&=0x7f;
	outp (puerto+CRreg,lcr);

	ier=inp (puerto+IERreg);
	ier|=0x4;
	outp (puerto+IERreg,ier);

}


// Resetea la UART especificada en port.

void UartRst(int port)
{
	port=(port*set.NumPorts)+set.base;

	for (int n=0;n<7;n++)
	inp (port+n);
}

void muestra (int port)
{
	gotoxy (3,port+1);
	cprintf ("PORT %d = %d / %d ",port,sio_ofree (port),sio_ifree(port));
}

// ----------------------- RUTINAS INTERRUPCION --------------------------

void interrupt IntMPort1(...)
{

	int  n=7;
	unsigned char byte=0;
	char byte1;
	unsigned char pic;
	void far (*direccion)(void)=0;

	if (flag==0)
	{
		flag=1;
		disable();

		pic = inp (0x21);
		pic|=0x08;
		outp (0x21,pic);

		do{
			byte=inp (set.IntVctr1); // Miramos primera placa MOXA

			for (n=0;n<set.NumPorts;n++,byte>>=1) // Comprobamos primera placa
			{
				byte1 = byte & 0x01;
				if (byte1==0) ServicioInter (n);
			}

		} while (inp (set.IntVctr1)!=0xff);

		pic = inp (0x21);
		pic&=0xf7;
		outp (0x21,pic);

		enable();
		flag=0;
	}
	outp (0x20,0x20);
}


void interrupt IntMPort2(...)
{
	int n=7;
	char byte=0;
	int byte1;
	unsigned char pic;

	void far (*direccion)(void)=0;

	if (flag==0)
	{
		flag=1;
		disable();

		pic = inp (0x21);
		pic|=0x20;
		outp (0x21,pic);

		do{
			byte=inp (set.IntVctr2); // Miramos primera placa MOXA

			for (n=0;n<set.NumPorts;n++,byte>>=1) // Comprobamos primera placa
			{
				byte1 = byte & 0x01;
				if (byte1==0) ServicioInter (n+8);
			}

		} while (inp (set.IntVctr2)!=0xff);

		pic = inp (0x21);
		pic&=0xdf;
		outp (0x21,pic);

		enable();
		flag=0;
	}
	outp (0x20,0x20);
}


void ServicioInter (int port)
{
	int puerto,n;
	int lcr;
	int ier;
	static char temp=0;
	int byte=0,byte1=0,byte2;
	int IntPen;

	puerto=set.base+((port*set.NumPorts));

	lcr = inp (puerto+CRreg);
	lcr&=0x7f;
	outp (puerto+CRreg,lcr);

//	do
//	{
		byte  = inp (puerto+IIDreg);
		IntPen=byte & 0x01;		// Si este bit es 1 es que no hay nterrupciones
									// pendientes.

		if (IntPen==NULL)
		{
			byte1 = byte & INT_RXD;
			if (byte1==INT_RXD)
			{
				byte2=inportb (puerto);

				ptrsio[port].IntCntRx++;
				ptrsio[port].BufferRx[ptrsio[port].cabeza] = byte2;
				++ptrsio[port].cabeza;
				if (DebugMode==1) muestra (port);

				if (ptrsio[port].cabeza>=MAX_LEN)
					ptrsio[port].cabeza=0;


				if (ptrsio[port].cont && ptrsio[port].cont>=ptrsio[port].IntCntRx)
				{
					TablaIntsRXD[port]();
				}
			}

			byte1 = byte & INT_TXD;

			if (byte1==INT_TXD)
			{
				if (ptrsio[port].IntCntTx)
				{
					sio_tx (port,ptrsio[port].BufferTx[0]);
					memmove (ptrsio[port].BufferTx,ptrsio[port].BufferTx+1,MAX_LEN);
					ptrsio[port].IntCntTx--;
				}
/*				else
				{
					PonDLAB (port,OFF);
					ier=inp (puerto+IERreg);
					ier&=0xfd;
					outp (puerto+IERreg,ier);		// Quita interrupciones
				}*/
			}

			byte1 = byte & INT_RXL;
			if (byte1==INT_RXL)
				TablaIntsRXL[port]();

			byte1 = byte & INT_RLS;
			if (byte1==INT_RLS)
				TablaIntsRLS[port]();

			byte1 = byte & INT_MST;
			if (!byte1)
			{
				byte1=inportb (puerto+MSRreg);
				TablaIntsMST[port]();
			}
		}
//	} while (IntPen==NULL);
}


