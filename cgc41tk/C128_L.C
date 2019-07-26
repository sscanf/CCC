#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <process.h>
#include <stdlib.h>
#include <c128.h>

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

const OFF	  = 0;

void far IntERROR (void);
void UartRst(int port);
//void far *ServicioInter (int port);
//void ServicioInter (int port);
void PonDLAB (int port ,int modo);



int sio_overlap (int port, int mode)
{
	return (-1); // Error, el puerto no est  abierto.
}

void sio_reset(void)
{
}


int sio_open (int port)
{
	int byte;

	asm mov ax,0x0c00
	asm mov dx,port

	asm int 0x14

	return (_AX);
}

// Devuelve un byte del buffer de recepci¢n.
// Si no hay ningun byte en el buffer espera a que entre uno.

int sio_getch(int port)
{
	int code;

	asm mov ax,0x0600
	asm mov dx,port
	asm int 0x14

	asm mov code,ax;

	return (code);
}

int sio_linput (int port, char *buff, int len, int term)
{
	return (0);
}

int sio_read (int port, char *buff, int len)
{
	return (0);
}


int sio_putch (int port, int code)
{
	asm mov ax,0x0700
	asm mov dx,port
	asm mov cx,code
	asm int 0x14

	return (_AX);
}

int sio_putb (int port, char *buff, int len)
{
	return (0);
}

int sio_write (int port, char *buff, int len)
{

	int n;
	int rt;

	for (n=0;n<len;n++)
		if ((rt=sio_putch (port,buff[n]))!=1) return (rt);

	return (n);

}

int sio_flush (int port, char func)
{
	asm mov dx,port
	asm mov ah,0x0f
	asm mov al,func
	asm int 0x14

	return (_AL);
}

int sio_ifree (int port)
{
	return (0);
}

int sio_ofree (int port)
{
	asm sti
	asm mov ax,0x0e00
	asm mov dx,port
	asm int 0x14

	return (_AX);
}
int sio_oqueue (int port)
{
	return (0);
}
int sio_lstatus (int port)
{

	asm mov ax,0x1000
	asm mov dx,port
	asm int 0x14

	return (_AX);
}

int sio_lctrl (int port, int mode)
{
	return (0);
}


int sio_close (int port)
{
	asm mov ax,0x0d00
	asm mov dx,port
	asm int 0x14

	return (_AX);
}


int sio_ioctl (int port, char init)
{

	asm mov ah,0x08
	asm mov al,0xe3
	asm mov dx,port

	asm int 0x14

	return (_AX);
}



void sio_tx(int port,char byte)
{
}



char sio_rx(int port)
{
	return (0);
}

int sio_cnt_irq (int port, void (interrupt (*func)()),int cont)
{
	char a;

	asm mov ax,0x0900
	asm mov dx,port

	asm les bx,func;
	asm mov cx,cont
	asm int 0x14
	return (_AX);

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

int sio_modem_irq (int port, void (interrupt (*func)()))
{
	asm mov ax,0x0a00
	asm mov dx,port

	asm les bx,func;
	asm int 0x14
	return (_AX);
}

// Resetea la UART especificada en port.

void UartRst(int port)
{
}
