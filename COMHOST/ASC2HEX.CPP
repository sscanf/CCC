#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>

void Ascii2Hex(char *missatge);

char mensaje[] = "Esto es una prueba que estoy haciendo para probar la rutina que he hecho para que muestre  cadenas ascii en hex";

void main (void)
{
	clrscr();
	Ascii2Hex (mensaje);
}

void Ascii2Hex(char *missatge)
{

	int cnt=0;
	int n,i;
	int PosX;
	int PosY;

	printf (" 1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  g       123456789abcdefg\n");
	printf ("-----------------------------------------------       ----------------\n");

	for (n=0;n<strlen(missatge);n+=16)
	{
		for (i=n;i<n+16;i++)
			printf ("%02x ",missatge[i]);

		printf ("      ");
		for (i=n;i<n+16;i++)
		{
			if (missatge[i]=='\n' || missatge[i]=='\a' || missatge[i]=='\0')
				printf (".");
			else
			printf ("%c",missatge[i]);
		}
	  printf ("\n");

	}
	  printf ("\n");
}