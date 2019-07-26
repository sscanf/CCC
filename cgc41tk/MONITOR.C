
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <c128.h>

int ch;
void interrupt IntPort (void)
{
		ch=sio_getch (3);
}

void main (void)
{
	int ch;

	sio_open (3);
	sio_ioctl (3,0xe3);
	sio_cnt_irq   (3 ,IntPort ,1);

	while (!kbhit())
	{
		if (ch>-1)
		{
			printf ("%x ",ch);
			ch=-1;
		}
	}

	sio_close (1);
}