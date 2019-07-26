~A86 will remove error messages if you leave this line in.~
;*************************************************************************
; V25Prova.ASM
; Data: 2-12-1994
;************************************************************************

 fint macro
  db 0fh,92h
 ;endm
 #em
 defseg ~06 Symbol Required~
2000h
  org 0000



scm1    equ 78h
scc1    equ 79h
sric1   equ 7dh; serial receive interrupt request control register
stic1   equ 7eh; serial transmit interrupt request control register
rxb1    equ 70h; receive buffer register
txb1    equ 72h; transmit buffer register
brg1    equ 7ah
intsr1  equ 44h
intst1  equ 48h

  org 0000

param    db 20 dup(0)
buffer   db 30 dup(0)
MaxParam dw 0
NumParam dw 0
posicio  dw 0
FiRx     db 0
OffFunc  dw 0
ErrFlag  dw 0
OffIni   dw 0
OffFi    dw 0
SegIni   dw 0
carac    db 41
dada     db 0ffh
OffBr    dw 0
SegBr    dw 0
SegGo    dw 0
OffGo    dw 0
RxDada   db 0
CanviLin dw 0

        org 00h
start:
        mov     ax, 03000h               ; Init data segment
	mov     ds, ax
        mov     ax, 02000h               ; Init stack segment
	mov     ss,ax

inici:
	 call V25Program
	 call InitVars1
	 sti
main:    
	 call SacaCabe
	 jmp main
	 
	 mov al,prompt
	 mov carac,al
	 call SacaPant
EsperaRx:
	 sti
	 cmp FiRx,0
	 jz EsperaRx
	 cli
	 call Rx
	 jmp main

InitVars1:
	 mov ah,BufValVirt[22] ; per defecte : SegBr = SegGo = cs
	 mov al,BufValVirt[23]
	 mov SegBr,ax
	 mov SegGo,ax
	 mov al,BufValVirt[24]  ;              OffGo = OffBr = ip
	 mov ah,BufValVirt[25]
	 mov OffGo,ax
	 mov OffBr,0
InitVars2:
	 mov ErrFlag,0
	 mov CanviLin,0
	 mov MaxParam,0
	 mov NumParam,0
	 mov posicio,1
	 mov FiRx,0 
	 mov RxDada,13
	 mov cx,20
	 mov bx,0
	 OmpleParam:      ; Inicialitza buffer par…metres.
	  mov param[bx],0 ; Cada par…metre s¢n 4 bytes.
	  inc bx
	  loop OmpleParam
	 mov cx,30
	 mov bx,0
	 OmpleBuf:       ; Inicialitza buffer dades entrada.
	  mov buffer[bx],0ffh
	  inc bx
	 loop OmpleBuf
	 mov carac,13
	 call SacaPant
	 mov carac,10
	 call SacaPant
	 ret

V25Program:            ; Programaci¢ V25
     mov ax,0fff0h     ; Port SŠrie
     mov es,ax
     mov byte ptr es:scm1,49h
     mov byte ptr es:scc1,01h
     mov byte ptr es:brg1,82h
     mov byte ptr es:rxb1,00h
     mov ax,0h        ; Taula Vectors Interrupci¢
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

SacaCabe:       ; Rutina que treu la cap‡alera EB
     push cx
     push bx
     mov cx,59
     mov bx,0

bucle6:
     mov al,cabecera[bx]
     mov carac,al
     call SacaPant
     inc bx
     loop bucle6
     pop bx
     pop cx
     ret


RSITx:
     fint

     iret

RSIRx:
     mov bx,0fff0h
     mov es,bx
     mov cl,es:rxb1 ; recuperem la dada rebuda
     mov RxDada,cl
     mov FiRx,1
     fint

     iret


Rx:
      cmp RxDada,13
      jz ActFiRx
       cmp posicio,29
       jae ActFiRx
	inc posicio
	jmp eco
ActFiRx:
      mov FiRx,1
      mov posicio,0
eco:
      mov bx,posicio
      mov al,RxDada
      mov buffer[bx],al ; carreguem buffer
      mov bx,0fff0h
      mov es,bx
      mov al,RxDada
      mov byte ptr es:txb1,al
      mov bx,0fff0h
      mov es,bx
      mov byte ptr es:scm1,0cdh ; Tx enable
      mov byte ptr es:scm1,4dh        ; Tx disable
FiFRx:
 sti
 ret

SacaPant: ; envia el valor de carac
      push ax
      push es
      push bx
      mov bx,0fff0h
      mov es,bx
      mov al,carac
      mov byte ptr es:txb1,al
      mov bx,0fff0h
      mov es,bx
      mov byte ptr es:scm1,0cdh ; Tx enable
      mov byte ptr es:scm1,4dh ; Tx disable
      pop bx
      pop es
      pop ax
      ret

;- - - - - - - - - - - - - ROM DATA - - - - - - - - - - - - - - - - - - - -

cabecera  db 'Electronica Barcelona S.L.',13,10
	  db 'Monitor ROM.Copyrigth(c) 1994',13,10
prompt db '.'
ErrMsg  db 'Error',13,10

; espai de mem•ria reservat a guardar registres
	BufferReg db 'AX=BX=CX=DX=',13,10
	   db 'SP=BP=SI=DI=',13,10
	   db 'DS=ES=SS=CS=IP=',13,10
	   db 'F',32
	BufValVirt db 0,0,0,0,0,0,0,0
	   db 255,238,0,0,0,0,0,0
	   db 30,18,30,18,30,18,30,18,1,0
	BufFlgVirt db 'DI NZ PO NC',13,10
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      org 0fff0h
      jmp  start

      END
