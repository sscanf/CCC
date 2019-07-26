/*--------------------- TRANSMISION ----------------------------------*/

void TranEntra(unsigned int nbytes)
{
	int rnd;
	char byte;
	unsigned int n;

	disable();
	outp (rci,0);              /*desactiva interrupciones 8259 */

/*	while (rnd==0)
	{
		rnd=inp(LineStat);
		rnd&=0x10;					 Espera que se cierre el squelch
	}*/

	outp (rcm1,inp (rcm1)|0x02);					/*Pone la emisora en transmisi¢n */
	espera(50);

	for (n=0;(n<nbytes) && (n<212);n++)
		sacabyte(BufTx[n]);

	espera(10);
	outp (rcm1, tdl | out2);

	outp (rci,1);              /*activa interrupciones 8259 */
	enable();
}


void espera(unsigned long int tiemp)
{
	unsigned long int i;

	for(i=0;i<(BucTime*tiemp);i++);
}

void sacabyte(unsigned int byte)
{
	unsigned char UartLibre=0;

	while (UartLibre!=0x60)
		UartLibre=inportb (rel) & 0x60;
	outp (com1,byte);
}

/*----------------------------- RECEPCION -----------------------------*/

/*------------------------Rutina Interrupci¢n--------------------------*/


void far interrupt cojebuff()
{
	unsigned int ch;
	int byte;

	unsigned int ContL;
	unsigned int ContH;
	unsigned int conta=0;
	int n;
	static int flag=0;


		outp (rci,0);         /* desactiva interrupciones de la UART */

		byte=inp (0x3fa);
		if((byte&=0x01)==0)
			byte=inp(0x3fe);

		ch=inbyte();

		if (ch==0xff && flag==0)
		{
			temporal[0]=ch;
			flag=1;
			ch=inbyte();
		}

		if (ch==0x81 && flag==1)
		{
				flag=0;
				temporal[1]=ch;

				ContL=inbyte();		/* Contador bytes low   */
				ContH=inbyte();		/* Contador bytes hight */

				temporal[2]=ContL;
				temporal[3]=ContH;

				ContL &= 0x00ff;
				ContH = ContH << 8;
				conta = ContH | ContL;
				/*conta(bytes)=data+data chksum(2bytes)*/

				if (conta<MAXCONTA)
				{
					for (n=4;(n<(conta+4));n++)
						temporal[n]=inbyte();

					FinRx=1;
				}
		}

	  outp (rcm1, tdl | out2);
	  outp (cop2,0x20);  /* Indica Fin de interrupt */
	  outp (rci,1);      /*ACTIVA INTERRUPCIONES UART */
}


unsigned int inbyte ()
{
	unsigned char tmp=0;
	unsigned long int timeout=0;
	unsigned int bt;

	while (tmp==0 && timeout<10000)
	{
		tmp = inportb (rcm);
		tmp &= 0x01;
		timeout++;
	}
	bt=inp (com1);
	return(bt);
}




