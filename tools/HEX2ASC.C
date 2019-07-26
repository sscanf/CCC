
#include <stdio.h>
#include <dos.h>
#include <conio.h>

union REGS ent,sal;


char RxData();

main()
{
	char a;

	clrscr();

	ent.x.ax = 0xe3;         // 0x9f = 1200,e,8,2
									 // 0xe3 = 9600,n,8,1
									 // 0xc3 = 4800,n,8,1
	ent.x.dx = 1;            // com1
	int86(0x14,&ent,&sal);   // total (OPEN "COM1:9600,N,8,1")


	for (;;)
	{
		a=RxData();
		cprintf ("%02x ",a);
	}
}


char RxData()
{
	char a;

	while (1)
	{
		a=inportb (0x2fd);
		a &= 0x01;
		if (a) return ((char)inportb (0x2f8));
   }
}




