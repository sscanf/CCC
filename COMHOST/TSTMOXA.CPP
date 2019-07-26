
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <process.h>
#include <stdlib.h>
#include <d:\comhost\head-c.h>

int carac;
int cont;

int RxByte[20];
int IniPorts(void);
void interrupt IntPort7();


void main()
{
	char string[80];
	int port;
	int dato;
	int n;

	clrscr();
	if ((IniPorts())!=0)
	{
		printf ("Fallo inicializaci¢n");
		exit (-1);
	}


	for (;;)
	{
		printf ("Entre port a transmitir ");
		gets(string);

		port=atoi(string);

		printf ("Pulse [f] para acabar\n");


		while (string[0]!='f')
		{
			printf ("\nEntre valor a transmitir (en decimal) ");
			gets (string);

			if (RxByte[0]!=0)
			{
				for (n=0;n<cont;n++)
				{
					printf ("%x ",RxByte[n]);
					RxByte[n]=0;
				}
				cont=0;
			}

			if (string[0]!='f')
			{
				dato=atoi (string);
				sio_putch (port,dato);
			}
		}

	}
}


int IniPorts(void)
{
	int n;

	for (n=0;n<8;n++)
	{
	  if (sio_open (n)!=0) return(-1);

	  if ((sio_ioctl (n,B9600,BIT_8|STOP_1|P_NONE))!=0)
			 return(-1);

	  sio_enableTx (n);

	}
  if (((n=sio_cnt_irq (7 ,IntPort7 ,1))!=0))
  {
	printf ("%d ",n);
	return(-1);
  }

}

void interrupt IntPort7()
{

	carac=(int)sio_getch(7);
	RxByte[cont++] = (int)carac;

}