;*************************************************************************
; V25Prova.ASM
; Data: 2-12-1994
;************************************************************************

 SCM1	equ 078h ;Serial Mode Register
 SCC1	equ 079h ;Serial Control Register
 BRG1	equ 07ah ;Baud Rate Generator Register
 SRIC1	equ 07dh ;Serial Receive Interrupt Request Control Register
 STIC1	equ 07eh ;Serial Transmit Interrupt Request Control Register
 RXB1	equ 070h ;Receive Buffer Register
 TXB1	equ 072h ;Transmit Buffer Register
 WTCL	equ 0e8h ;Wait Control Register Hight
 WTCH	equ 0e9h ;Wait Control Register Low
 RSHM	equ 0e1h ;Refresh mode Register
 PRC	equ 0ebh ;Processor Control Register.


 INTSR1 equ 044h ;Vector interrupci¢n rutina RX
 INTST1 equ 048h ;Vector interrupci¢n rutina TX


 .MODEL small

 fint macro
  db 0fh,92h
 endm


;---------------------- SEGMENTO DE RAM -------------------------------------

.DATA

bufferRx    db 10h dup (0)
bufferTx    db 61h dup (0)

param	    db 20 dup(0)
buffer	    db 30 dup(0)
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
OffBr	    dw 0
SegBr	    dw 0
SegGo	    dw 0
OffGo	    dw 0
RxDada	    db 0
return	    dw 0    ;Indica que han entrado un comando
CanviLin    dw 0
punterTx    dw 0
punterRx    dw 0
FiTx	    db 0    ;Indica que ya ha transmitido el buffer
adrBufferTx dw 0
maxTx	    dw 60

.STACK [256]

.CODE
.STARTUP

    jmp main

;-------------------------- ROM DATA ---------------------------------------

cabecera   db 'Electronica Barcelona S.L.',13,10
	   db 'Monitor ROM.Copyrigth(c) 1994',13,10

ErrMsg	   db 13,10,'Error'

BufferReg  db 'AX=BX=CX=DX=',13,10
	   db 'SP=BP=SI=DI=',13,10
	   db 'DS=ES=SS=CS=IP=',13,10
	   db 'F',32

BufValVirt db 0,0,0,0,0,0,0,0
	   db 255,238,0,0,0,0,0,0
	   db 30,18,30,18,30,18,30,18,1,0

BufFlgVirt db 'DI NZ PO NC',13,10

;-------------------------------------------------------------------------


main:
	 mov PunterTx,0

	 call V25Init
	 call V25Program
	 call TransCabe
	 sti

main1:
	jmp main1

;- - - - - - - - - - - - - TRANSMITE CABECERA - - - - - - - - - - - - - - - -

TransCabe:

	mov di,0
	mov cx,59
pasa:

	mov al,cs:cabecera[di]
	mov BufferTx[di],al
	inc di
	loop pasa

	mov BufferTx[di],0
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
	mov ax,0fff0h
	mov es,ax

	mov al,byte ptr es:scm1
	or al,80h
	mov byte ptr es:scm1,al ;Activa transmisi¢n
	sti
	ret


;- - - - - - - - - - - - - DESACTIVA TRANSMISION - - - - - - - - - - - - - - - -
TxOff:
	mov ax,0fff0h
	mov es,ax

	mov al,byte ptr es:scm1
	and al,7fh
	mov byte ptr es:scm1,al ;Desactiva transmisi¢n
	ret


;- - - - - - - - - - - - - TRANSMITE BUFFER TX - - - - - - - - - - - - - - - -

TxBuffer:
	mov FiTx,0
	call TxOn   ;Activa transmisi¢n.
TxBuf1:
	cmp FiTx,0
	jz TxBuf1   ;Si FiTx es 1 quiere decir buffer transmitido.
	ret

;- - - - - - - - - - - - - INICIALIZACION - - - - - - - - - - - - - - - -

V25Init:
     cli
     mov ax,0fff0h     ; Port SŠrie
     mov es,ax
     ; es pot programar wait states
     mov byte ptr es:wtcl,0ffh
     mov byte ptr es:wtch,0ffh
     ;refresh mode
     mov byte ptr es:rshm,0fch
     ;clock ratio
     mov byte ptr es:prc,0eh
     ret


V25Program:	       ; Programaci¢ V25
     mov ax,0fff0h     ; Port SŠrie
     mov es,ax

     mov byte ptr es:scm1,09h
     mov byte ptr es:scc1,04h
     mov byte ptr es:brg1,52
     mov byte ptr es:stic1,07h
     mov byte ptr es:sric1,07h

     mov ax,0h	      ; Taula Vectors Interrupci¢
     mov es,ax
     mov si,intsr1
     mov ax,OFFSET RSIRx
     mov es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax
     mov ax,OFFSET RSITx
     mov si,intst1
     mov word ptr es:[si],ax
     add si,2
     mov ax,cs
     mov es:[si],ax
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
     mov byte ptr es:txb1,al

     inc punterTX
     cmp al,0
     jnz FiRSITX

     mov punterTx,0

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

     cmp punterRx,10h	  ;Comprueba que el buffer no est‚ lleno.
     jnz NoLLeno
err:
     mov BufferTx,07	  ;Beep de buffer lleno
     mov BufferTx+1,0
     call TxOn
     jmp FiRSIRx

NoLLeno:

     mov di,punterRx
     mov bufferRx[di],cl

     inc punterRx

     cmp cl,13		  ;Mira si pulsan <RETURN>
     jnz NoReturn

     mov return,1
     jmp FiRSIRx

NoReturn:
     mov BufferTx,cl
     mov BufferTx+1,0
     call TxOn		;Transmitir  el buffer recibido (rebote)


FiRSIRx:
     mov al,bufferRx[0]
     mov RxDada,al
     pop ax
     pop di
     pop cx
     pop es
     pop bx
     sti
     fint
     iret
 END
