#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <mem.h>

void status (void);
void LineaFina (void);
void LineaGruesa (void);
void GuardaScrn (void);
void LoadScrn (void);
void text (void);

int buffer[4000];
char *panta;
int bnc=0;

char banco[]={'Ú','À','¿','Ú','Ù','À','¿','Ù','É','È','»','É','¼','È','»','¼'};
main()
{
	int x=1,y=1;
	int write=0;
	int chr='³';
	int flg=255;

	char a;
	clrscr();
	a=0;
	gotoxy (1,1);

	while (a!='q')
	{
		a = getch();

		if (a=='s' && x<79)
		{
			 gotoxy (x,y);
			 if (flg==3 && !write) putch (banco[bnc]);
			 if (flg==1 && !write) putch (banco[bnc+1]);
			 if (bnc==0) chr='Ä';
				else
			 chr='Í';

			 flg=0;
			 x++;
		}

		if (a=='z'&& y<25 )
		{
			 gotoxy (x,y);
			 if (flg==0 && !write) putch (banco[bnc+2]);
			 if (flg==2 && !write) putch (banco[bnc+3]);
			 flg=1;
			 if (bnc==0) chr='³';
				else
			 chr='º';
			 y++;
		}

		if (a=='w'&& y>1 )
		{
			 gotoxy (x,y);
			 if (flg==0 && !write) putch (banco[bnc+4]);
			 if (flg==2 && !write) putch (banco[bnc+5]);
			 flg=3;
			 if (bnc==0) chr='³';
				else
			 chr='º';
			 y--;
		}

		if (a=='a' && x>1 )
		{
			 gotoxy (x,y);
			 x--;
			 if (flg==3 && !write) putch (banco[bnc+6]);
			 if (flg==1 && !write) putch (banco[bnc+7]);
			 if (bnc==0) chr='Ä';
				else
			 chr='Í';
			 flg=2;
		}

		if (a=='p') status();
		if (a==' ') write=~write;
		if (a=='f') LineaFina();
		if (a=='g') LineaGruesa();
		if (a=='d') GuardaScrn();
		if (a=='l') LoadScrn();
		if (a=='o') putch(' ');
		if (a=='t') text();
		if (a=='1') putch ('Ë');
		if (a=='2') putch ('Ê');
		if (a=='3') putch ('Ì');
		if (a=='4') putch ('¹');
		if (a=='5') putch ('Î');


		gotoxy (x,y);
		if (write==0) putch (chr);
	}

	delete (buffer);
}


void status (void)
{
	int n;
	for (n=0;n<4000;n++)
		buffer[n]=peek (0xb800,n);

//	movedata(0xb800, 0, _DS, (unsigned)buffer, 200);

	clrscr ();
	getch();
	for (n=0;n<4000;n++)
		poke (0xb800,n,buffer[n]);
//	movedata(_DS,(unsigned)buffer, 0xb800, 0, 200);
}
void GuardaScrn (void)
{
	FILE *f;

	int n;

	f=fopen ("cgc.scr","w");

	for (n=0;n<4000;n++)
		fputc (peek (0xb800,n),f);
	fclose (f);

}

void LoadScrn (void)
{
	FILE *f;

	int n;

	if ((f=fopen ("cgc.scr","rt"))!=NULL)
	{
		for (n=0;n<4000;n++)
			poke (0xb800,n,fgetc (f));

		fclose (f);
	}
	else
		printf ("No puedo abrir el fiechero\n");
}

void LineaGruesa (void)

{
	bnc=8;
}
void LineaFina (void)

{
	bnc=0;
}

void text (void)
{
	char tec;
	tec=0;
	while (tec!=27)
	{
		tec=getch();
		if (tec!=27) putch (tec);
	}
}