#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <mem.h>
#include <process.h>
#include <stdlib.h>
#include <std.h>

const F1 = 59;
const F2 = 60;
const F3 = 61;
const F4 = 62;

#define ARRIBA 72
#define ABAJO  80
#define IZDA	75
#define DRCHA	77
#define INICIO 71
#define REPAG	73
#define FIN		79
#define AVPAG	81

void presenta(void);
void GestionaChar (unsigned char ch);
void PutSwitch (char status);

int x=0;
int y=0;
int puerto;

void main (int argc, char **argv)
{
	char ch,tecla;

	if (*argv[1]=='1') puerto=COM1;
	if (*argv[1]=='2') puerto=COM2;

	if (*argv[1]!='1' && *argv[1]!='2')
	{
		printf ("\nformato:    demo [puerto]\n\n");
		printf ("1	COM1\n");
		printf ("2	COM2\n\n");
		exit (-1);
	}

	directvideo=1;
	presenta();
	sio_init (0x3f8,0x2f8,4,3,1843200);

	sio_open (COM2);
	sio_ioctl (COM2,1200,'E',8,2);
	sio_overlap(COM2,1);

	while (tecla!=27)
	{
		while (!kbhit())
		 if (sio_ifree (COM2)>0) GestionaChar (sio_getch(COM2));
		tecla = getch();
	}


	sio_close (COM2);
	sio_end();
}

void presenta (void)
{
	clrscr();
	textbackground (1);
	textcolor (15);
	clrscr();
	cprintf ("                                                        ESTADO REMOTO          \r\n");
	cprintf ("      Electr¢nica Barcelona S.L.              ÚÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ¿\r\n");
	cprintf ("      Test para TL01RSC1.                     ³ PUL.1 ³ PUL.2 ³ PUL.3 ³ PUL.4 ³\r\n");
	cprintf ("                                              ³       ³       ³       ³       ³\r\n");
	cprintf ("                                              ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ\r\n\r\n");
	cprintf ("ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ[ TEXTO ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("³                                                                             ³\r\n");
	cprintf ("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r\n");
	gotoxy (1,25);
	cprintf ("[Esc]- Fin.");
	gotoxy (2,8);
}

void GestionaChar (unsigned char ch)
{
	int n;
	char flag=0;
	char status;
	static int CntBytes=2;

	if (y==0) y=1;
	if (x==0) x=1;

	window (2,8,78,23);
	gotoxy (x,y);
	cprintf ("%x ",ch);
	x=wherex();
	y=wherey();
	window (1,1,79,25);
	gotoxy (x+1,y+7);
}

void PutSwitch (char status)
{
	int n,x=51;
	int x1,y1;
	unsigned char byte=0x08;
	unsigned char byte2=0,byte3;

	sound (700);
	delay (10);
	nosound();
	x1=wherex();
	y1=wherey();
	textcolor (LIGHTRED);

	for (n=0;n<4;n++,x+=8,byte>>=1)
	{
		byte2=status & byte;

		if (byte2==0)
		{
			gotoxy (x,4);
			cprintf (" ");
		}
		else
		{
			gotoxy (x,4);
			cprintf ("*");
		}
	}
	textcolor (15);
	gotoxy (x1,y1);
}

