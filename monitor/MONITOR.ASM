;******************************************************************************
;*                                                                            *
;* NOMBRE	: MONITOR.ASM						      *
;* DESCRIPCION	: MONITOR V25 PARA PLACA RTX-21 			      *
;* LENGUAJE	: ENSAMBLADOR PARA MASM V6.0				      *
;* EDITOR	: PWB							      *
;* FECHA	: 22/11/1994						      *
;* OBSERVACIONES:							      *
;*                                                                            *
;*                                                                            *
;******************************************************************************


;------------------------------------------------------------------------------
; Funciones:
;
;	L - Carga un fichero con HEX-INTEL en la memoria indicada con locate.
;	    Sin par metros.
;
;	D - Muestra un area de memoria.
;
;	    D Offset
;	    D Segmento:Offset comienzo,Offset final
;
;	    * Siempre se entre el segmento, se ha de poner el offset final.
;
;	E - Cambia una determinada direcci¢n de memoria.
;
;	    E Segmento:Offset
;
;	    * Siempre se ha de entrar el segmento y el offset.
;
;	F - Rellena un area de memoria con un valor.
;
;	    F seg:offset,offset,dato
;	    F offset,offset,dato
;	    F offset,dato
;
;	G - Carga el IP con la direcci¢n de memoria indicada.
;
;	    G Segmento:Offset
;	    G offset
;	    G
;
;	    * En el caso de ning£n par metro, se coge el segmento que indica
;	      el CS y offset que indica el IP.
;
;	B - Pone un break point en una determinada direcci¢n.
;
;	    B Offset
;	    B Segmento:offset
;
;	N - Quita el break point.
;	    Sin par metros.
;
;
;
; Hay que tener en cuenta que si se pone un break point, no se ha de poner
; otro hasta que se quite el que hay puesto con la instrucci¢n 'N'.
;
; El segmento de datos est  ubicado en el la direcci¢n 890h, si se cambia
; por cualquier motivo, se ha de cambiar la constante RAM definida al prin-
; cipio de este programa por la nueva direcci¢n.
;
; Esto sucede porque la rutina de break point 'intBREAK' recupera el valor
; de DS por medio de esta constante.
;
; Enamblar y lincar el programa por medio del PWB.
;
; Para crear el fichero HEX se ha de hacer los siguiente:
;
; Crear el fichero 'MONITOR.CFG' con los siguientes par metros.
;
; class	 data	 =	 0x0040		 ;address of ram
; class	 code	 =	 0xf000 	 ;code address
; class	 stack	 =	 0x0840
;
; order	 stack data code ;create the correct ordering
; rom	 code
;
; desde la l¡nea de comandos del DOS:
;
; locate -b -h monitor
;
;----------------------------------------------------------------------------
; include dos.inc

 EndChar equ 0



 RAM	equ 890h ;Direcci¢n del data segment.

 SCM1   equ 078h ;Serial Mode Register
 SCC1	equ 079h ;Serial Control Register
 BRG1	equ 07ah ;Baud Rate Generator Register
 SRIC1	equ 07dh ;Serial Receive Interrupt Request Control Register
 STIC1	equ 07eh ;Serial Transmit Interrupt Request Control Register
 RXB1   equ 070h ;Receive Buffer Register
 TXB1	equ 072h ;Transmit Buffer Register
 WTCL   equ 0e8h ;Wait Control Register Hight
 WTCH	equ 0e9h ;Wait Control Register Low
 RSHM	equ 0e1h ;Refresh mode Register
 PRC	equ 0ebh ;Processor Control Register.
 BREAK  equ 0cch ;Instrucci¢n de break


 INTP0  equ 060h ;Vector interrupci¢n INTP0
 NMI	equ 08h  ;Vector interrupci¢n rutina NMI
 INTBRK equ 0ch	 ;Vector interrupci¢n break points
 INTSR1 equ 044h ;Vector interrupci¢n rutina RX
 INTST1 equ 048h ;Vector interrupci¢n rutina TX




 .MODEL small

  fint macro
  db 0fh,92h
 endm

.STACK [256]


;---------------------- SEGMENTO DE RAM -------------------------------------

.DATA

bufferRx    db 60h dup (0)
bufferTx    db 61h dup (0)

BufValVirt db 0,0     ;ax
	   db 0,0     ;bx
	   db 0,0     ;cx
	   db 0,0     ;dx
	   db 255,238 ;sp
	   db 0,0     ;bp
	   db 0,0     ;si
	   db 0,0     ;di
	   db 30,18   ;ds
	   db 30,18   ;es
	   db 30,18   ;ss
	   db 30,18   ;cs
	   db 1,0     ;ip
	   db 0,0     ;flags

StackBr    dw 0	      ;ax	;Stack temporal para registros en rutina BREAK
	   dw 0       ;dx


BufFlgVirt  db 12 dup(12)
param	    db 20 dup(0)
rebote	    db 0
MaxParam    dw 0
NumParam    dw 0
posicio     dw 0
OffFunc     dw 0
ErrFlag     dw 0
OffIni	    dw 0
OffFi	    dw 0
SegIni	    dw 0
carac	    db 41
dada	    db 0ffh
SegGo	    dw 0
OffGo	    dw 0
RxDada	    db 0
dadesRx     dw 0
CanviLin    dw 0
punterTx    dw 0
punterRx    dw 0
FiTx	    db 0
return	    db 0
adrBufferTx dw 0
maxTx	    dw 60
MemBreak    db 0    ;Memoriza el byte machacado por el break.
OffBreak    dw 0
SegBreak    dw 0

.CODE


.STARTUP

start:
	 call InitVars
	 call InitVars2
         mov punterTX,0
         mov punterRX,0

         jmp inici

;-------------------------- ROM DATA ---------------------------------------

cabecera   db 'Electronica Barcelona S.L.',13,10
	   db 'Monitor ROM.Copyrigth(c) 1994',13,10

prompt	   db '.'
ErrMsg	   db 'Error',13,10

BufferReg  db 'AX=BX=CX=DX=',13,10
	   db 'SP=BP=SI=DI=',13,10
	   db 'DS=ES=SS=CS=IP=',13,10
	   db 'F',32



;-------------------------------------------------------------------------

inici:
	 call V25init
	 call V25Program
	 call SacaCabe
	 call RxOn	    ;Activa recepci¢n
	 sti

main:
	 mov rebote,1
	 mov BufferTx,10
	 mov BufferTx+1,13
	 mov BufferTx+2,'.'
	 mov BufferTx+3,EndChar
	 mov ErrFlag,0
	 call TxBuffer	    ;Transmite el prompt

EsperaRx:

	 cmp return,1
	 jnz EsperaRx

	 mov BufferTx,10
	 mov BufferTx+1,13
	 mov BufferTx+2,EndChar
	 call TxBuffer

	 mov return,0
         call OutSpace
	 call scanner
         mov ax,ErrFlag
	 cmp ax,1
	 jz error

	 mov di,OffFunc
	 call word ptr di  ; crida a la funci¢ a executar
	 mov ax,ErrFlag
	 cmp ax,1
	 jz error
         call InitVars2
	 jmp main

error:
	mov di,0
        mov cx,7
Epasa:
        mov al,cs:ErrMsg[di]
	mov BufferTx[di],al
	inc di
	loop Epasa

	mov BufferTx[di],EndChar
	call TxBuffer	;Activa transmisi¢n
	jmp main

InitVars:
	 mov BufFlgVirt[12],10
	 mov BufFlgVirt[11],13
	 mov BufFlgVirt[09],' '
	 mov BufFlgVirt[08],' '
	 mov BufFlgVirt[07],' '
	 mov BufFlgVirt[06],' '
	 mov BufFlgVirt[05],' '
	 mov BufFlgVirt[04],' '
	 mov BufFlgVirt[03],' '
	 mov BufFlgVirt[02],' '
	 mov BufFlgVirt[01],' '
	 mov BufFlgVirt[00],' '
	 call GuardaFlags
	 ret


InitVars2:

	 pushf
	 pop dx

	 mov ErrFlag,0
	 mov CanviLin,0
	 mov MaxParam,0
	 mov NumParam,0
	 mov posicio,1
	 mov return,0 ; temporalment
         mov cx,20
	 mov bx,0

OmpleParam:	  ; Inicialitza buffer par…metres.

	  mov param[bx],0 ; Cada par…metre s¢n 4 bytes.
          inc bx
          loop OmpleParam
	  mov cx,30
	  mov bx,0

OmpleBuf:	 ; Inicialitza buffer dades entrada.
	  mov bufferRx[bx],0ffh
          inc bx
	  loop OmpleBuf
	  ret



GuardaFlags:

      mov ax,dx
      and ax,01h
      cmp ax,1
      jnz car2

      mov BufFlgVirt[09],'C'
      mov BufFlgVirt[10],'Y'
      jmp parity2

car2:
      mov BufFlgVirt[09],'N'
      mov BufFlgVirt[10],'C'

parity2:
      mov ax,dx
      and ax,0004h
      cmp ax,4
      jz Part2
      mov BufFlgVirt[6],'P'
      mov BufFlgVirt[7],'O'
      jmp z2

part2:
      mov BufFlgVirt[6],'P'
      mov BufFlgVirt[7],'E'

z2:
      mov ax,dx
      and ax,0040h
      cmp ax,64
      jz zero2
      mov BufFlgVirt[3],'A'
      mov BufFlgVirt[4],'C'
      jmp ints2

zero2:
      mov BufFlgVirt[3],'N'
      mov BufFlgVirt[4],'A'

ints2:
      mov ax,dx
      and ax,0200h
      cmp ax,0200h
      jz dis2
      mov BufFlgVirt[0],'E'
      mov BufFlgVirt[1],'I'
      jmp FiGuardaFlag2

dis2:
      mov BufFlgVirt[0],'D'
      mov BufFlgVirt[1],'I'

FiGuardaFlag2:
      ret


V25Init:
     cli
     mov ax,0fff0h     ; Port SŠrie
     mov es,ax

     mov byte ptr es:wtcl,0ffh     ; es pot programar wait states
     mov byte ptr es:wtch,0ffh
     mov byte ptr es:rshm,0fch     ;refresh mode
     mov byte ptr es:prc ,0ch      ;clock ratio
     ret

V25Program:	       ; Programaci¢ V25
     mov ax,0fff0h     ; Port SŠrie
     mov es,ax

     mov byte ptr es:scm1 ,09h
     mov byte ptr es:scc1 ,04h
     mov byte ptr es:brg1 ,52
     mov byte ptr es:stic1,07h
     mov byte ptr es:sric1,07h

     mov ax,0h	      ; Taula Vectors Interrupci¢
     mov es,ax

     mov si,INTSR1
     mov ax,OFFSET RSIRx
     mov es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax

     mov si,INTST1
     mov ax,OFFSET RSITx
     mov word ptr es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax

     mov si,INTBRK
     mov ax,offset IntBREAK	;Rutina de interrupci¢n para break point.
     mov word ptr es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax

     mov si,NMI
     mov ax,offset IntNMI	;Rutina de interrupci¢n para el NMI.
     mov word ptr es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax

     mov si,INTP0
     mov ax,OFFSET HandIntP0
     mov word ptr es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax
     ret
;- - - - - - - - - - - - - TRANSMITE CABECERA - - - - - - - - - - - - - - - -

SacaCabe:

	mov di,0
	mov cx,59
pasa:

	mov al,cs:cabecera[di]
	mov BufferTx[di],al
	inc di
	loop pasa

	mov BufferTx[di],EndChar
	call TxBuffer	;Transmite el buffer
	ret

;- - - - - - - - - - - - - ACTIVA RECEPCION - - - - - - - - - - - - - - - -
RxOn:
	mov ax,0fff0h
	mov es,ax

	mov al,byte ptr es:scm1
	or al,40h
	mov byte ptr es:scm1,al ;Activa recepci¢n
	ret


;- - - - - - - - - - - - - DESACTIVA RECEPCION - - - - - - - - - - - - - - - -
RxOff:
	mov ax,0fff0h
	mov es,ax

	mov al,byte ptr es:scm1
	and al,0bfh
	mov byte ptr es:scm1,al ;Desactiva recepci¢n
	ret


;- - - - - - - - - - - - - ACTIVA TRANSMISION - - - - - - - - - - - - - - - -
TxOn:
	push ax
	push es

	mov ax,0fff0h
	mov es,ax

	mov al,byte ptr es:scm1
	or al,80h
	mov byte ptr es:scm1,al ;Activa transmisi¢n

	pop es
	pop ax
	sti
	ret


;- - - - - - - - - - - - - DESACTIVA TRANSMISION - - - - - - - - - - - - - - - -
TxOff:
	push ax
	push es

	mov ax,0fff0h
	mov es,ax

	mov al,byte ptr es:scm1
	and al,7fh
	mov byte ptr es:scm1,al ;Desactiva transmisi¢n

	pop es
	pop ax
	ret


;- - - - - - - - - - - - - TRANSMITE BUFFER TX - - - - - - - - - - - - - - - -

TxBuffer:

	mov FiTx,0
	call TxOn   ;Activa transmisi¢n.
TxBuf1:
	cmp FiTx,0
	jz TxBuf1   ;Si FiTx es 1 quiere decir buffer transmitido.
	ret

OutSpace:	; Treu els espais en blanc del buffer d'entrada
	 push cx
	 push bx
	 push di
	 mov cx,30
	 mov bx,0
busca:
	 cmp bufferRx[bx],13
	 jz FiOutSpace
	 cmp bufferRx[bx],32
	 jz treu
	 inc bx
	 loop busca
FiOutSpace:
	 pop di
	 pop bx
	 pop cx
	 ret

treu:	 ; bx apunta a la posici¢ on es troba l'espai en blanc
	 clc
	 mov ax,30
	 sbb ax,bx
	 mov di,0
	 mou:
	  mov dl,bufferRx[bx+di+1]
	  mov bufferRx[bx+di],dl
	  inc di
	  cmp di,ax
	  jnz mou
	   jmp busca

scanner:	       ; mira quina ‚s la funci¢ que s'ha d'executar.
     mov al,bufferRx
     cmp al,'D'
     jz volcar
     cmp al,'F'
     jz llenar
     cmp al,'R'
     jz RegDis
     cmp al,'B'
     jz BreakPoint
     cmp al,'N'
     jz NoBreak
     cmp al,'G'
     jz go
     cmp al,'E'
     jz entry
     cmp al,'L'
     jz load
     mov ErrFlag,1
FiScanner:
     ret

load:	  ; Carga en memoria una fila HEX
	  ; Sin par metros.



   mov MaxParam,0
   mov OffFunc,OFFSET FuncLoad
   call GetParam1
   jmp FiScanner

entry:	  ; ensenya el que hi ha en mem•ria. Si s'especifica un nou valor
	  ; queda gravat en mem•ria, return no modifica el valor i '.' acaba.
	  ; E seg:offset
	  ; E offset
   mov MaxParam,2
   mov OffFunc,OFFSET FuncEntry
   call GetParam1
   jmp FiScanner

volcar:   ; visualitza l'espai de mem•ria especificat per l'usuari.
	  ; D seg:offset,offset
	  ; D offset,offset
	  ; D offset

	mov MaxParam,3
	mov OffFunc,OFFSET FuncVolcar
	call GetParam1
	jmp FiScanner

llenar:   ; omple espai de mem•ria amb dada. Tot especificat per l'usuari.
	  ; F seg:offset,offset,dada
	  ; F offset,offset,dada
	  ; F offset,dada
	mov MaxParam,4
	mov OffFunc,OFFSET FuncLlenar
	call GetParam1
	jmp FiScanner

RegDis:   ; visualitza el valor dels registres (virtual).
	  ; R
	mov MaxParam,0
	mov OffFunc,OFFSET FuncRegDis
	jmp FiScanner

BreakPoint:

	mov MaxParam,2		      ; definici¢ d'un breakpoint
	mov OffFunc,OFFSET FuncBreak  ; B seg:offset
	call GetParam1
	jmp FiScanner

NoBreak:
	mov MaxParam,0		      ; reset del breakpoint
	mov OffFunc,OFFSET FuncNBreak ; N
	jmp FiScanner

go:
	mov MaxParam,2		  ; execuci¢  a partir de l'adre‡a especificada per l'usuari o
	mov OffFunc,OFFSET FuncGo ; continua si la parada ‚s provocada per breakpoint
	call GetParam1		  ; G seg:offset
	jmp FiScanner		  ; G ( per continuar)

FuncVolcar:

	call ParamTest	; controla que els par…metres siguin correctes
	cmp ErrFlag,1

	jz ErrVolcar
	call volca	; treu per pantalla el que hi ha es espai de memo indicat
	cmp ErrFlag,1
	jz ErrVolcar

FiFVolcar:
	mov ErrFlag,0
	ret

ErrVolcar:
	ret

ParamTest:
	mov ah,BufValVirt[16]  ; si l'usuari no ho canvia, per defecte Seg=ds
	mov al,BufValVirt[17]
	mov SegIni,ax
	cmp NumParam,1
	jz volca1
	cmp NumParam,2
	jz volca2
	cmp NumParam,3
	jz volca3
	jmp ErrParTest

FiParTest:
	mov ErrFlag,0
	ret

ErrParTest:
	mov ErrFlag,1
	ret

volca1:   ; offset d'inici especificat per l'usuari.
	  ; offset fi= offset inici + 256 o fi de segment.
     mov bx,0
     call paste
     mov OffIni,ax
     add ax,256
     cmp ax,0ffffh
     jbe InitFi
     mov OffFi,0ffffh
     jmp FiParTest
InitFi:
    mov OffFi,ax
    jmp FiParTest

volca2: ; offset d'inici i fi especificats per l'usuari.
     mov bx,0
     call paste
     mov OffIni,ax
     mov bx,4
     call paste
     mov OffFi,ax
     jmp FiParTest

volca3: ; segment, offset d'inici i fi especificats per l'usuari.
    mov bx,0
    call paste
    mov SegIni,ax
    mov bx,4
    call paste
    mov OffIni,ax
    mov bx,8
    call paste
    mov OffFi,ax
    jmp FiParTest

paste: ; Rutina que genera un valor hexa de 4 d¡gits a partir
       ; de buffer param, bx a bx+4.
       ; El resultat en ax.
     push cx
     push dx
     mov al,param[bx]
     and al,0fh
     mov dl,param[bx+1]
     mov cl,4
     shl dl,cl
     or al,dl
     mov ah,param[bx+2]
     and ah,0fh
     mov dh,param[bx+3]
     shl dh,cl
     or ah,dh
     pop dx
     pop cx
     ret

volca:	; mostra en pantalla els valors de mem•ria compresos entre SegIni:OffIni
	; i SegIni:OffFi.
     mov dx,SegIni
     mov cx,OffFi
     mov bx,OffIni
     cmp cx,bx
     jb ErrVolca

     sub cx,bx	     ; en cx es guarda el num de vegades que s'executar… el bucle
		     ; num vegades= OffFi-OffIni.
     mov ax,OffIni
     add ax,16
     mov CanviLin,ax ; canvi de l¡nia cada 16 bytes.
     mov es,dx	     ; posa en es el SegIni
     call SacaAdre   ; ensenya la adre‡a SegIni:bx.

bucle:

     call retu
     mov si,bx
     call SacaValor2
     mov BufferTx,32
     mov BufferTx+1,EndChar
     call TxBuffer
     inc bx
     loop bucle

     jmp FiVolca
ErrVolca:
     mov ErrFlag,1
     ret
FiVolca:
     ret

retu:			; mira si cal provocar el canvi de l¡nia en la visualitzaci¢.
     push dx
     mov dx,bx
     cmp dx,CanviLin
     jnz FiRetu

     mov BufferTx,13	; provoca canvi de l¡nia : return+10.
     mov BufferTx+1,10
     mov BufferTx+2,EndChar
     call TxBuffer

     call SacaAdre	; ensenya la nova adre‡a SegIni:bx.
     add CanviLin,16
Firetu:
     pop dx
     ret

SacaAdre:		; visualitza l'adre‡a SegIni:bx

	push dx
	push cx

	mov dx,SegIni
	mov al,dh
	mov cl,4
	shr al,cl

	call Hex2ASCII
	mov BufferTx+1,al

	mov al,dh
	and al,0fh
	call Hex2ASCII
	mov BufferTx+2,al

	mov dx,SegIni
	mov al,dl
	mov cl,4
	shr al,cl
	call Hex2ASCII
	mov BufferTx+3,al

	mov dx,SegIni
	mov al,dl
	and al,0fh
	call Hex2ASCII
	mov BufferTx+4,al

	mov BufferTx+5,':'

	mov dx,bx
	mov al,dh
	mov cl,4
	shr al,cl
	call Hex2ASCII
	mov BufferTx+6,al

	mov dx,bx
	mov al,dh
	and al,0fh
	call Hex2ASCII
	mov BufferTx+7,al

	mov dx,bx
	mov al,dl
	mov cl,4
	shr al,cl
	call Hex2ASCII
	mov BufferTx+8,al

	mov dx,bx
	mov al,dl
	and al,0fh
	call Hex2ASCII
	mov BufferTx+9,al

	mov BufferTx+10,32
	mov BufferTx+11,EndChar
	call TxBuffer
	pop cx
	pop dx
	ret

Hex2ASCII: ; Converteix de hexa a ASCII el contingut de al.
	   ; El resultat en carac.
   and al,0fh
   cmp al,9
   jbe num
   add al,55

;   mov BufferTx+1,EndChar
   ret
num:
   add al,48
;   mov BufferTx,al
;   mov BufferTx+1,EndChar
   ret

FuncLlenar:
    call ParamTest1 ; control de par…metres correctes.
    cmp ErrFlag,1
    jz ErrLlenar
    call llena	    ; omple espai de mem•ria especificat per l'usuari.
FiFLlenar:
    mov ErrFlag,0
    ret
ErrLlenar:
    ret

llena:
   mov dx,SegIni
   mov cx,OffFi
   mov bx,OffIni
   cmp cx,bx
   jb ErrLlena ; error si OffFi< OffIni
   sub cx,bx  ; num vegades = OffFi-OffIni.
   mov es,dx
 bucle1:
   mov al,dada
   mov es:[bx],al ; posa dada en mem•ria.
   inc bx
 loop bucle1
   mov ErrFlag,0
   ret
ErrLlena:
   mov ErrFlag,1
   ret


ParamTest1:
    mov ah,BufValVirt[16] ; per defecte SegIni = ds
    mov al,BufValVirt[17]
    mov SegIni,ax
    cmp NumParam,3
    jz llena3
    cmp NumParam,4
    jz llena4
    cmp NumParam,2
    jz llena2
    jmp ErrParTest1
FiParTest1:
    mov ErrFlag,0
    ret
ErrParTest1:
    mov ErrFlag,1
    ret


llena3: ; offset d'inici, fi i dada especificats per l'usuari.
     mov bx,0
     call paste
     mov OffIni,ax
     mov bx,4
     call paste
     mov OffFi,ax
     mov al,param[9]
     mov cl,4
     shl al,cl
     mov dl,al
     mov al,param[8]
     and al,0fh
     or al,dl
     mov dada,al
     jmp FiParTest1

llena4:  ; segment, offset d'inici, fi i dada especificats per l'usuari.
    mov bx,0
    call paste
    mov SegIni,ax
    mov bx,4
    call paste
    mov OffIni,ax
    mov bx,8
    call paste
    mov OffFi,ax
    mov al,param[13]
    mov cl,4
    shl al,cl
    mov dl,al
    mov al,param[12]
    and al,0fh
    mov dada,al
    jmp FiParTest1

llena2: ; offset d'inici i dada especificats per l'usuari.
	; offset fi = offset d'inici+1.
    mov bx,0
    call paste
    mov OffIni,ax
    inc ax
    mov OffFi,ax
    mov al,param[5]
    mov cl,4
    shl al,cl
    mov dl,al
    mov al,param[4]
    and al,0fh
    mov dada,al
    jmp FiParTest1

FuncNBreak:
    mov es,OffBreak
    mov di,SegBreak
    mov al,MemBreak
    mov es:[di],al
    ret

FuncGo:
    cmp NumParam,0
    jz GoBreak
    cmp NumParam,1
    jz GetOffGo
    cmp NumParam,2
    ja ErrFuncGo

    mov bx,0
    call paste
    mov SegGo,ax
    mov bx,4
    call paste
    mov OffGo,ax
    jmp GetIP

GetOffGo:
    mov bx,0
    call paste
    mov OffGo,ax
    mov ah,BufValVirt[22]
    mov al,bufValVirt[23]
    mov SegGo,ax

GetIP:
    pop ax	; recupera offset de l'adre‡a retorn  a monitor
		; neteja stack
		; canviem l'adre‡a de retorn per la de la nova funci¢ a executar, OffGo+SegGo defineixen nova adre‡a
		; carreguem registres de la nova funci¢

   mov ah,BufValVirt[10]
   mov al,BufValVirt[11]
   mov bp,ax
   mov ah,BufValVirt[12]
   mov al,BufValVirt[13]
   mov si,ax
   mov ah,BufValVirt[14]
   mov al,BufValVirt[15]
   mov di,ax
   mov bh,BufValVirt[2]
   mov bl,BufValVirt[3]
   mov ch,BufValVirt[4]
   mov cl,BufValVirt[5]
   mov dh,BufValVirt[6]
   mov dl,BufValVirt[7]
   mov ah,BufValVirt[18]
   mov al,BufValVirt[19]
   mov es,ax
   mov ah,BufValVirt[20]
   mov al,BufValVirt[21]
   mov ss,ax
   mov ah,BufValVirt[8]
   mov al,BufValVirt[9]
   mov sp,ax
   mov ax,SegGo	   ; segment de la nova adre‡a
   mov BufValVirt[22],ah
   mov BufValVirt[23],al
   mov ax,OffGo	    ; offset de la nova adre‡a
   mov BufValVirt[24],ah
   mov BufValVirt[25],al

   mov ah,BufValVirt[26]
   mov al,BufValVirt[27]
   push ax		; pone valor flags
   push SegGo		; posa el nou segment a l'stack
   push OffGo		; posa el nou offset a l'stack

FiFuncGo:		     ; el ret em retornar… a la funci¢ a executar
    mov ErrFlag,0
    mov ah,BufValVirt[0]
    mov al,BufValVirt[1]
    push ax

    mov ah,BufValVirt+16
    mov al,BufValVirt+17
    mov ds,ax
    pop ax
    retf

ErrFuncGo:
    mov ErrFlag,1
    ret

GoBreak:
    mov ah,BufValVirt[22]
    mov al,BufValVirt[23]
    mov SegGo,ax
    mov ah,BufValVirt[24]
    mov al,BufValVirt[25]
    mov OffGo,ax
    jmp GetIP

FuncRegDis:  ; visualitza l'espai de mem•ria reservat als registres virtuals.

    mov di,0
    mov si,0

bucle3:

    mov al,BufferReg[di]
    cmp al,13
    jz	noreg
    cmp al,10
    jz	noreg
    cmp al,'F'
    jz flags
    jmp regs

noreg:
    mov al,BufferReg[di]
    mov BufferTx,al
    mov BufferTx+1,EndChar
    call TxBuffer
    inc di
    jmp bucle3

regs:
    mov al,BufferReg[di]
    mov BufferTx,al

    mov al,BufferReg[di+1]
    mov BufferTx+1,al

    mov al,BufferReg[di+2]
    mov BufferTx+2,al
    mov BufferTx+3,EndChar
    call TxBuffer

    call SacaValor ; es:[si] i es:[si+1]
    mov BufferTx,32
    mov BufferTx+1,EndChar
    call TxBuffer

    add di,3
    add si,2
    jmp bucle3
FiFRegDis:
    mov ErrFlag,0
    ret

SacaValor:
    mov ax,ds
    mov es,ax
    mov ax,OFFSET BufValVirt
    add ax,si
    push si
    mov si,ax
    call SacaValor2 ; ds:BufValVirt[si]
    pop si
    mov ax,OFFSET BufValVirt
    inc ax
    add ax,si
    push si
    mov si,ax
    call SacaValor2  ; ds:BufValVirt[si+1]
    pop si
    ret

FuncBreak:

    cmp NumParam,1
    jz ErrFuncBreak

    cmp NumParam,1
    jz GetOffBr1

    cmp NumParam,2
    ja ErrFuncBreak

    mov bx,0
    call paste
    mov es,ax

    mov bx,4
    call paste
    mov di,ax
    mov al,es:[di]
    mov MemBreak,al

    mov al,BREAK
    mov es:[di],al	;Introduce un break


    mov OffBreak,es
    mov SegBreak,di
    jmp FiFuncBreak


GetOffBr1:

     mov ah,BufValVirt[22]
     mov al,BufValVirt[23]
     mov es,ax
     mov bx,0
     call paste
     mov di,ax
     mov al,es:[di]
     mov MemBreak,al
     mov al,BREAK
     mov es:[di],al

FiFuncBreak:
     mov ErrFlag,0
     ret

ErrFuncBreak:
     mov ErrFlag,1
     ret

flags:
      mov al,BufferReg[di]
      mov BufferTx,al

      mov BufferTx+1,32
      mov BufferTx+2,EndChar
      call TxBuffer

      mov cx,13
      mov di,0

bucle4:
      mov al,BufFlgVirt[di]
      mov BufferTx,al
      mov BufferTx+1,EndChar
      call TxBuffer
      inc di
      loop bucle4
      jmp FiFRegDis

FuncEntry:

     mov ax,NumParam ; control par…metres correctes
     cmp ax,1
     jz entry1
     cmp ax,2
     jz entry2
     jmp ErrFuncEntry

FiFuncentry:
     call RxEntry  ; crida  a la funci¢ si no hi ha error en els par…metres.
     ret

ErrFuncentry:
     mov ErrFlag,1
     ret

entry1:     ; Per defecte SegIni= ds, l'offset l'especifica l'usuari.

     mov ah,BufValVirt[16]
     mov al,BufValVirt[17]
     mov SegIni,ax
     mov bx,0
     call paste
     mov OffIni,ax
     jmp FiFuncentry

entry2:    ; Offset i Segment especificats per l'usuari.
     mov bx,0
     call paste
     mov SegIni,ax
     mov bx,4
     call paste
     mov OffIni,ax
     jmp FiFuncentry

RxEntry:   ; mostra el valor actual de la mem•ria i el canvi si l'usuari
	   ; n'especifica un de nou, return no modifica i '.' acaba.

     mov ax,SegIni
     mov es,ax
     mov bx,OffIni

bucle7:
     mov si,bx
     call SacaValor2  ; visualitza seg:[bx]
     mov BufferTx,'>'
     mov BufferTx+1,' '
     mov BufferTx+2,EndChar
     call TxBuffer

     call GetTeclat  ; obt‚ un byte del teclat
     cmp ErrFlag,1
     jz FiRxEntry

     cmp RxDada,13
     jz  salta7

     cmp RxDada,'.'
     jz FiRxEntry

     mov cl,RxDada
     mov es:[bx],cl

     mov BufferTx+1,13
     mov BufferTx+2,10
     mov BufferTx+3,EndChar
     call TxBuffer

salta7:
     inc bx
     jmp bucle7
FiRxEntry:
     ret

SacaValor2: ; treu per pantalla el valor de la posici¢ de mem•ria es:[si]

     push cx
     mov al,es:[si]
     mov cl,4
     shr al,cl
     call Hex2ASCII
     mov BufferTx,al

     mov al,es:[si]
     and al,0fh
     call Hex2ASCII
     mov BufferTx+1,al
     mov BufferTx+2,EndChar
     call TxBuffer
     pop cx
     ret

GetTeclat:	; agafa del teclat el valor i el posa en RxDada en hexa
     push cx
     push bx
     push ax
     push di

     call GetChar

     cmp al,'.'
     jz punt

     cmp al,13
     jz nomodif

     mov cl,al	; agafa 4 LSB
     call ASCII2Hex	; converteix a Hexa
     cmp ErrFlag,1
     jz FiGetTeclat
     mov al,cl
     mov cl,4
     shl al,cl
     mov RxDada,al	; guarda en RxDada els 4 MSB
     call Getchar	; agafa 4 LSB

     mov cl,al
     call ASCII2Hex	; convertex a hexa
     cmp ErrFlag,1
     jz FiGetTeclat
     or RxDada,cl	; guarda en RxDada els 4 MSB i els 4 LSB
     jmp FiGetTeclat
punt:

     mov RxDada,'.'
     jmp FiGetTeclat

nomodif:
     mov RxDada,13
     mov BufferTx,10
     mov BufferTx+1,13
     mov BufferTx+2,EndChar
     call TxBuffer

FiGetTeclat:
     pop di
     pop ax
     pop bx
     pop cx
     ret


GetChar:

     cmp PunterRx,0
     jz GetChar

     mov PunterRx,0
     mov al,BufferRx ;En al queda el byte recibido.
     ret

FuncLoad:

     xor bx,bx		;BX= Suma total de bytes para checksum.
     xor cx,cx		;CX= N£mero de bytes a recibir.
     xor ax,ax
     mov rebote,0	;no har  rebotes.

FuncL:

     call GetChar	;Espera ':' para sincronizarse
     cmp al,':'
     jnz FuncL

     call GetTeclat
     mov  cl,RxDada	;en cx est  el n£mero de bytes a recibir.
     add  bx,cx

     call GetDada
     mov  di,ax 	;en DI est  el offset donde empezar a escribir.

     xor ax,ax
     call GetTeclat
     mov al,RxDada	;determinal el tipo de registro
     add bx,ax

     cmp al,02		;Segmento
     jz  segmento
     cmp al,00		;datos
     jz  datos
     cmp al,03
     jz  Lentry
     cmp al,01
     jz  LoadFin
     jmp ErrLoad


segmento:

     call GetDada
     mov  es,ax 	;En ES queda el segmento.

     call GetTeclat	;Byte de checksum.
     xor ax,ax
     mov al,RxDada
     add bx,ax
     cmp  bl,0
     jnz  ErrLoad	;El checksum no es correcto.
     jmp FuncLoad


datos:
     xor ax,ax

     call GetTeclat
     mov  al,RxDada
     mov  byte ptr es:[di],al ;carga el byte recibido en memoria.
     add  bx,ax
     inc  di
     loop datos

     call GetTeclat	;Byte de checksum.
     mov  al,RxDada
     add  bx,ax
     cmp  bl,0
     jnz  ErrLoad	;El cheksum no es correcto.
     jmp  FuncLoad


Lentry:

     call GetDada

     mov BufValVirt+22,ah
     mov BufValVirt+23,al   ;moidifica el registro CS en la tabla virtual
			    ;con el segmento del entry point

     call GetDada
     mov BufValVirt+24,ah
     mov BufValVirt+25,al   ;moidifica el registro IP en la tabla virtual
			    ;el offset	del entry point


     call GetTeclat	;Byte de checksum.
     mov  al,RxDada
     add  bx,ax
     cmp  bl,0
     jnz  ErrLoad	;El cheksum no es correcto.
     jmp  FuncLoad



ErrLoad:
     mov rebote,1	;permite rebotes.
     mov ErrFlag,1
     ret

LoadFin:
     mov rebote,1	;permite rebotes.
     mov ErrFlag,0
     ret

GetDada:

     xor dx,dx
     call GetTeclat
     mov  ah,RxDada
     mov  dl,ah
     add  bx,dx

     call GetTeclat
     mov  al,RxDada
     mov  dl,al
     add  bx,dx
     ret



GetParam1:

    mov NumParam,0
    mov cx,MaxParam

captura1:

    mov di,posicio
    mov dh,0
    mov dl,bufferRx[di]
    cmp dl,13
    jz FiGetParam
    cmp dl,','
    jz IncPos
    cmp dl,':'
    jz IncPos

    push cx
    call GetNum
    pop cx
    cmp ErrFlag,1
    jz ErrParam
    loop captura1

    mov di,posicio
    mov dh,0
    mov dl,bufferRx[di]
    cmp dl,13
    jz FiGetParam
    jmp ErrParam

ErrParam:

    mov ErrFlag,1
    ret


FiGetParam:
	 inc NumParam
         mov ErrFlag,0
	 ret
IncPos:
	 inc posicio
	 inc NumParam
	 jmp captura1

GetNum:
	mov cx,4
	mov di,NumParam

control:
	 mov si,posicio
	 mov cl,bufferRx[si]
	 call ASCII2Hex
	 mov si,posicio
	 mov bufferRx[si],cl
	 mov ErrFlag,1
	 jz ErrGetNum

	 push cx
	 mov cl,2
	 mov di,NumParam
	 mov ax,di
	 shl al,cl
	 mov di,ax
	 pop cx
	 mov al,param[di+2]
	 mov param[di+3],al
	 mov al,param[di+1]
	 mov param[di+2],al
	 mov al,param[di]
	 mov param[di+1],al
	 mov si,posicio
	 mov al,bufferRx[si]
	 mov param[di],al
	 inc posicio
	 mov si,posicio
	 mov al,bufferRx[si]
	 mov ah,0
	 cmp ax,','
	 jz FiGetNum
	 cmp ax,13
	 jz FiGetNum
	 cmp ax,':'
	 jz FiGetNum
	 loop control

	 cmp ax,','
	 jz FiGetNum
	 cmp ax,13
	 jz FiGetNum
	 cmp ax,':'
	 jz FiGetNum
	 jmp ErrGetNum

ErrGetNum:
	 mov ErrFlag,1
	 ret

FiGetNum:
	mov ErrFlag,0
	ret

ASCII2Hex: ; converteix de ASCII a hexa el valor de cl.
	   ; resultat en cx.
        mov ch,0
	cmp cx,'0'
	jl ErrASCII
	cmp cx,'9'
	jg lletra
	add cx,0d0h
	jmp FiASCII2Hex
lletra:
	cmp cx,'F'
	jg ErrASCII
	cmp cx,'A'
	jl ErrASCII
	add cx,0c9h
	jmp FiASCII2Hex
ErrASCII:
	mov ErrFlag,1
	ret
FiASCII2Hex:
	mov ErrFlag,0
	ret

;-------------------- RUTINAS DE INTERRUPCION -------------------------------

RSITx:
     push cx
     push bx
     push es
     push ax
     push di
     push dx

     mov di,punterTx
     mov al,BufferTx[di]
     mov bx,0fff0h
     mov es,bx
     inc punterTX
     mov byte ptr es:txb1,al

     cmp al,EndChar
     jnz FiRSITX

     mov punterTx,0
     mov FiTx,1
     call TxOff     ;Desactiva la transmisi¢n

FiRSITX:

     mov dx,0fff0h
     mov es,dx
     mov byte ptr es:stic1,07h	;Resetea flag de interrupci¢n

     pop dx
     pop di
     pop ax
     pop es
     pop bx
     pop cx
     sti
     fint
     iret

RSIRx:
     push bx
     push es
     push cx
     push di
     push ax

     mov bx,0fff0h
     mov es,bx
     mov byte ptr es:sric1,07h	    ;Resetea flag de RX


     mov cl,es:rxb1	  ;El dato recibido queda en CL.
     cmp cl,13
     jz  NoLLeno

     cmp punterRx,59h	  ;Comprueba que el buffer no est‚ lleno.
     jnz NoLLeno

err:
     mov BufferTx,07	  ;Beep de buffer lleno
     mov BufferTx+1,EndChar
     call TxOn
     jmp FiRSIRx

NoLLeno:

     mov di,punterRx
     mov bufferRx[di],cl
     inc punterRx

     cmp cl,13		  ;Mira si pulsan <RETURN>
     jnz NoReturn

     mov return,1
     mov punterRX,0
     jmp FiRSIRx

NoReturn:


     cmp rebote,0
     jz  FiRSIRx	;Si el flag de rebote es 0 no rebota.

     mov BufferTx,cl
     mov BufferTx+1,EndChar
     call TxOn		;Transmitir  el buffer recibido (rebote)


FiRSIRx:
     pop ax
     pop di
     pop cx
     pop es
     pop bx
     sti
     fint
     iret


IntBREAK:

    push ds

    push ax
    mov ax,RAM
    mov ds,ax
    pop ax

    mov BufValVirt,ah
    mov BufValVirt+1,al ;ax

    mov BufValVirt+2,bh
    mov BufValVirt+3,bl

    mov BufValVirt+4,ch
    mov BufValVirt+5,cl

    mov BufValVirt+6,dh
    mov BufValVirt+7,dl

    mov ax,sp
    mov BufValVirt+8,ah
    mov BufValVirt+9,al

    mov ax,bp
    mov BufValVirt+10,ah
    mov BufValVirt+11,al

    mov ax,si
    mov BufValVirt+12,ah
    mov BufValVirt+13,al

    mov ax,di
    mov BufValVirt+14,ah
    mov BufValVirt+15,al

    mov ax,es
    mov BufValVirt+18,ah
    mov BufValVirt+19,al

    mov ax,ss
    mov BufValVirt+20,ah
    mov BufValVirt+21,al

    pop ax		    ;ahora AX toma el valor de DS
    mov BufValVirt+16,ah
    mov BufValVirt+17,al    ;ds



    pop ax	;IP
    pop bx	;CS
    pop dx	;flags

    dec ax
    mov BufValVirt+22,bh
    mov BufValVirt+23,bl    ;CS

    mov BufValVirt+24,ah    ;IP
    mov BufValVirt+25,al

    call GuardaFlags

    push dx
    push cs
    mov ax,offset start
    push ax
    fint
    iret

intNMI:
     fint
     iret


HandIntP0:

     fint
     iret

     END
