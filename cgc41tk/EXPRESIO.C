#include <stdio.h>
#include <string.h>



void main (void)
{
	char *(*puntero[10])(char&,char (*)(char),char,char *);

	char info[]="Esto es una prueba";

	printf ("%d",sizeof(char)*(strlen(info)-2));
}