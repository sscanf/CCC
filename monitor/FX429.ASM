;*************************************************************************
; FX429.asm
; Data: 2-12-1994
;************************************************************************

 include Const429.inc   ;Constantes FX429.
 include ConstV25.inc   ;Constantes Inicializaci¢n V25.
 include ConstSer.inc   ;Constantes manupulaci¢n datos serie.

 .MODEL small

 fint macro
  db 0fh,92h
 endm

.STACK [256]

;=============================== RAM ========================================

.DATA
    org 0h

    MiPrefx   db ?  ; Prefijo movil (No volatil)
    MiIndic   dw ?  ; Indicativo movil (No volatil)

    BuffTxRs  db 60h dup (0)
    BuffRxRs  db 60h dup (0)
    NumMecan  db 3 dup (0)   ;Aqu¡ guarda el n£mero de mec nico en ASCII.

    BuffRxFx  db 24 dup (0)  ;1 HEAD con CRC y 2 SLOTS con CRC cada uno.
    BuffTxFx  db 10 dup (0)  ;1 HEAD con SYNCT

    punterTx  dw ?
    punterRx  dw ?
    MdmStat   db ?  ;Memoriza el estado del modem.
    MdmCtrl   db ?  ;Memoriza la palabra de control del modem.
    RadioCtrl db ?  ;Memoriza el se¤ales de control de la radio.
    flag      db ?
    TxMsgL    dw ?  ;Puntero del buffer de transmisi¢n MODEM.
    RxMsgL    dw ?
    RxMsgLn   dw ?
    RxIndic   dw ?
    FiTx      db ?
    MsgFlg    db ?  ;Indica el tipo de memsaje recibido.

;Definici¢n del byte FLAG:
;
; Bit     Descripci¢n
;
;  0   -
;  1   -
;  2   -   Indica que ha recibido todos los slots.
;  3   -   Se ha recibido un byte serie.
;  4   -   0- X_ON/ 1- X_OFF.
;  5   -
;  6   -
;  7   -
;
;

;Definici¢n del byte RadioCtrl:
;
; Bit     Descripci¢n
;
;  0   -   No usado
;  1   -   No usado
;  2   -   R1/ MICRO - 0 = On / 1 = Off
;  3   -   R2/ MODEM - 0 = On / 1 = Off
;  4   -   R3/ SQL
;  5   -   R4
;  6   -   No usado
;  7   -   No usado
;
;
;

;Definici¢n del byte MsgFlg:
;
; Bit     Descripci¢n
;
;  0   -   Mensaje tipo RQC
;  1   -   Mensaje tipo AHYC
;  2   -   Mensaje tipo RQQ
;  3   -   Mensaje tipo NACK
;  4   -   Mensaje tipo ACK
;  5   -
;  6   -
;  7   -



;=============================== ROM ========================================

.CODE
.STARTUP

    cli
    mov flag,0
    mov MsgFlg,0


    call V25Ini     ;Inicializaci¢n CPU
    call ModemOff   ;Desactiva el modem y activa el micro.
    call RxOnRs     ;Activa recepci¢n serie.
    call ModemRst   ;Resetea el modem
    call RxModeFx   ;Pone el modem en recepci¢n
    sti
    mov MiIndic,01
    mov MiPreFx,01
    call TxX_ON     ;Transmite al display X_ON
    mov ah,06
    call ScreenMode ;Pone el display en SCREEN MODE 6

    mov BuffRxRs,0  ;Limpio buffer recepci¢n

NoMecan:            ;Pide n£mero de mecanico.

    mov di,punterRX
    cmp BuffRxRs[di],RETURN
    jnz noMecan

    mov al,BuffRxRs[0] ;Copia los 3 digitos del n£mero a al buffer NumMecan
    mov NumMecan[0],al
    mov al,BuffRxRs[1]
    mov NumMecan[1],al
    mov al,BuffRxRs[2]
    mov NumMecan[2],al

    call TxRQC      ;Transmite RQC por radio.

espAHYC:            ;Espera un AHYC
    mov al,MsgFlg   ;El bit 1 de MsgFlg indicar  que ha recibido un AHYC
    and al,02h
    cmp al,02h
    jnz espAHYC


main:
    mov ah,MsgFlg
    cmp ah,0
    jz main


    mov ah,MsgFlg
    and ah,01h
    cmp ah,01h
    jz  RQC
    mov ah,MsgFlg
    and ah,02h
    cmp ah,02h
    jz  AHYC
    mov ah,MsgFlg
    and ah,04h
    cmp ah,04h
    jz  RQQ
    mov ah,MsgFlg
    and ah,08h
    cmp ah,08h
    jz  NACK
    mov ah,MsgFlg
    and ah,10h
    cmp ah,10h
    jz  ACK
    jmp main

RQC:
    jmp main
AHYC:
    jmp main
RQQ:
    jmp main
NACK:
    jmp main
ACK:
    jmp main
;========================== SUBRUTINAS ======================================


;----------- FUERZA AL DISPLAY A GENERAR UNA PANTALLA -----------------------
;
; El numero de pantalla ha de estar en ah
;
ScreenMode:

    mov BuffTxRs[0],10h
    mov BuffTxRs[1],ah
    mov BuffTxRs[2],0
    call TxBufferRs
    ret
;--------------------- TRANSMITE AL DISPLAY XON -----------------------------

TxX_ON:

    mov byte ptr BuffTxRs[0],X_ON
    mov byte ptr BuffTxRs[6],0
    call TxBufferRs
    ret

;--------------------- TRANSMITE AL DISPLAY XOFF -----------------------------
TxX_OFF:

    mov BuffTxRs,X_OFF
    mov BuffTxRs+1,0
    call TxBufferRs
    ret


TxMsgFX:

    call ModemOn      ;Activa salida modem y desactiva el micro.

    mov TxMsgL,0
    mov MdmCtrl,0
    mov MdmStat,0

    call TxModeFx       ;Pone el modem en modo TX
    mov dx,TXDATAFX
    mov al,BuffTxFx
    out dx,al         ;Transmite el primer byte del buffer
                      ;de los demas se encarga el interrupt.
    ret

TxModeFx:

    and MdmCtrl,0fbh  ;Quita el Rx enable
    or MdmCtrl,1      ;Activa Tx enable
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al         ;TX enable
    ret

RxModeFx:

    mov RxMsgL,0
    mov RxMsgLn,8
    mov MdmCtrl,4      ;Activa Rx enable
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al         ;RX enable
    ret


V25Ini:

    mov ax,0fff0h
    mov es,ax

    mov byte ptr es:scm0 ,09h
    mov byte ptr es:scc0 ,04h
    mov byte ptr es:brg0 ,52
    mov byte ptr es:stic0,07h
    mov byte ptr es:sric0,07h

    mov byte ptr es:wtcl ,0ffh     ; es pot programar wait states
    mov byte ptr es:wtch ,0ffh
    mov byte ptr es:rshm ,0fch     ;refresh mode
    mov byte ptr es:prc  ,0ch      ;clock ratio

    mov byte ptr es:PMC0 ,00h      ;Port 0 en port mode
    mov byte ptr es:PM0  ,0f0h     ;4 bits bajos salidas, 4 bits altos entradas.

    mov byte ptr es:PMC1 ,00h      ;Port 1
    mov byte ptr es:PM1  ,0ffh     ;/INTP0,/INTP1,/INTP2

    mov byte ptr es:ISPR ,00h

    mov byte ptr es:EXIC0,00h
    mov byte ptr es:EXIC1,07h
    mov byte ptr es:EXIC2,07h

    mov ax,0h        ; Taula Vectors Interrupci¢
    mov es,ax

    mov si,INTSR0
    mov ax,OFFSET RSIRx
    mov es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax

    mov si,INTST0
    mov ax,OFFSET RSITx
    mov word ptr es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax


    mov si,INTP0
    mov ax,OFFSET HandIntP0
    mov es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax

    mov si,INP1
    mov ax,OFFSET HandIntP0
    mov es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax

    mov si,INP2
    mov ax,OFFSET HandIntP0
    mov es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax

    mov si,NMI
    mov ax,offset IntNMI       ;Rutina de interrupci¢n para el NMI.
    mov word ptr es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax
    ret


ModemOff:

    push ax
    push es

    and RadioCtrl,0f7h      ;Activa MICRO
    or RadioCtrl,04h        ;Desactiva MODEM

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    pop es
    pop ax
    ret

ModemOn:

    push ax
    push es

    and RadioCtrl,0fbh      ;Activa MODEM
    or RadioCtrl,08h        ;Desactiva MICRO

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    pop es
    pop ax
    ret


;------------------------- ACTIVA RECEPCION SERIE ---------------------------
RxOnRs:
	mov ax,0fff0h
	mov es,ax

        mov al,byte ptr es:scm0
	or al,40h
        mov byte ptr es:scm0,al ;Activa recepci¢n
        mov punterRx,0
	ret


;------------------------- DESACTIVA RECEPCION SERIE --------------------------
RxOffRs:
	mov ax,0fff0h
	mov es,ax

        mov al,byte ptr es:scm0
	and al,0bfh
        mov byte ptr es:scm0,al ;Desactiva recepci¢n
	ret


;------------------------- ACTIVA TRANSMISION SERIE --------------------------
TxOnRs:
	push ax
	push es

	mov ax,0fff0h
	mov es,ax

        mov PunterTx,0
        mov al,byte ptr es:scm0
	or al,80h
        mov byte ptr es:scm0,al ;Activa transmisi¢n

	pop es
	pop ax
	sti
	ret


;------------------------- DESACTIVA TRANSMISION SERIE -----------------------
TxOffRs:
	push ax
	push es

	mov ax,0fff0h
	mov es,ax

        mov al,byte ptr es:scm0
	and al,7fh
        mov byte ptr es:scm0,al ;Desactiva transmisi¢n

	pop es
	pop ax
	ret


;------------------------- TRANSMITE BUFFER TX SERIE -------------------------

TxBufferRs:

	mov FiTx,0
        call TxOnRs   ;Activa transmisi¢n.
TxBuf1:
	cmp FiTx,0
	jz TxBuf1   ;Si FiTx es 1 quiere decir buffer transmitido.
	ret



;------------------------- RESETEA MODEM -----------------------------------

ModemRst:
        mov dx,CONTROLFX
        mov al,04
        out dx,al

        mov al,00
        out dx,al       ;Reset modem
        ret


;========================== INTERRUPTS ======================================





;----------------------------- INTP0 ----------------------------------------

HandIntP0:

    push ax
    push dx
    push di
    push es


    mov dx,STATUSFX
    in al,dx

    mov MdmStat,al     ;En MdmStat esta el estado del modem.

    and al,08h
    cmp al,08h
    jz  TxBufferFx       ;Transmite siguiente byte al modem.

CntInt1:

    mov al,MdmStat
    and al,10h
    cmp al,10h
    jnz CntInt2
    jmp TxIdle         ;Tx Idle.

CntInt2:

    mov al,MdmStat
    and al,01h
    cmp al,01h
    jnz CntInt3
    jmp RxBuff         ;RxData.

CntInt3:

    mov al,MdmStat
    and al,40h
    cmp al,40h
    jnz CntInt4
    jmp RxSYNC         ;Recibe un SYNC

CntInt4:

    mov al,MdmStat
    and al,80h
    cmp al,80h
    jnz CntInt5
    jmp RxSYNT         ;Recibe un SYNT

CntInt5:
IntP0Fin:

    mov ax,0fff0h
    mov es,ax
    mov byte ptr es:EXIC0,00

    pop es
    pop di
    pop dx
    pop ax
    sti
    fint
    iret



;- - - - - - - - - Transmite siguiente byte al modem - - - - - - - - - - - - -

TxBufferFx:

    cmp TxMsgL,0ah      ;Tope de largo en bytes del mensaje a transmitir.
    jz	CntInt1 	;Para un mensaje short data	- 0ah
			;Para un mensaje con 1 slot  - 0dh
			;Para un mensaje con 2 slots - 13h
    mov di,TxMsgL
    mov al,BuffTxFx[di]
    mov dx,TXDATAFX
    out dx,al           ;Transmite el siguiente byte del modem.

    cmp di,4
    jnz SigTx

    mov al,MdmCtrl
    or  al,02h
    mov MdmCtrl,al
    mov dx,CONTROLFX
    out dx,al           ;Activa paridad interna.

SigTx:

    inc TxMsgL
   jmp CntInt1

;- - - - - - - - - - - - - - - - RX DATA - - - - - - - - - - - - - - - - - -

RxBuff:

    mov dx,RXDATAFX
    in  al,dx          ;Leo el byte recien recibido.

    mov di,RxMsgL

    mov BuffRxFx[di],al  ;Guardo el byte recibido en el buffer.
    inc RxMsgL         ;Apunto a la siguiente posici¢n del buffer.
    inc di

    cmp di,RxMsgLn     ;Compruebo si es final de buffer
    jnz CntInt3        ;Ha terminado de recibir el mensaje.

    mov al,MdmStat
    and al,01
    cmp al,0
    jnz No0
    jmp  FinRxMsg       ;El CRC no es correcto, rechazo el mensaje.

No0:
    mov al,flag
    and al,01
    cmp al,0
    jz  NoSLOTS        ;No esta recibiendo los SLOTS
    jmp NoHEAD         ;Ya ha recibido todos los SLOTS.


NoSLOTS:

    mov al,BuffRxFx+2  ;Miro el tipo de mensaje por si han de venir SLOTS.
    and al,03h         ;Para saber si es un HEAD se ha de leer el 'CAT', este
    mov ah,BuffRxFx+3  ;queda comprendido entre el byte 2 y el 3 del BuffRxFx.
    and ah,07fh        ;'CAT' esta mitad en ah y mitad en al.
    clc
    mov cl,2
    rol ax,cl          ;'CAT' esta en ah
    cmp ah,1
    jnz NoHEAD         ;No es un HEAD

    mov ah,BuffRxFx+3  ;Miro la cantidad de SLOTS que han de venir.
    and ah,30h
    clc
    mov cl,4
    ror ah,cl
    cmp ah,0
    jz  UnSlot

    mov RxMsgLn,24
    jmp cnt1

unSlot:

    mov RxMsgLn,16

cnt1:

    or flag,2         ;Flag para indicar que vienen SLOTS.
    or MdmCtrl,8
    mov dx,STATUSFX
    mov al,MdmCtrl
    out dx,al         ;Indica al modem que ahora viene SLOTS.
    jmp FinRxMsg

NoHEAD:

    mov al,BuffRxFx ;Primero miro si el mensaje es para mi.
    and al,07fh     ;Compruebo si el prefijo coincide con el mio.
    cmp al,MiPrefx
    jnz FinRxMsg

    mov ah,BuffRxFx+1
    mov al,BuffRxFx+2
    mov cl,3
    ror ax,cl
    and ax,0001h
    cmp MiIndic,ax  ;Mi indicativo no es el mismo, rechazo mensaje.
    jnz FinRxMsg

    mov ah,BuffRxFx+5
    and ah,0e0h
    mov bl,BuffRxFx+4
    mov bh,BuffRxFx+3
    and bh,03h

    rcl ah,1
    rcl bx,1
    rcl ah,1
    rcl bx,1
    rcl ah,1
    rcl bx,1

    and bx,0fffh
    mov RxIndic,bx  ;Memorizo el indicativo recibido.

    mov ah,BuffRxFx+3
    and ah,1ch

    cmp ah,01ch
    jz  rRQC         ;El mensaje recibido es un RQC
    cmp ah,14h
    jz  rAHYC        ;El mensaje recibido es un AHYC
    cmp ah,18h
    jz  rRQQ         ;El mensaje recibido es un RQQ
    cmp ah,00
    jz  rNACK        ;El mensaje recibido es un NACK
    cmp ah,0ch
    jz  rACK         ;El mensaje recibido es un ACK
    jmp FinRxMsg

rRQC:
    or MsgFlg,01
    jmp FinRxMsg

rAHYC:
    or MsgFlg,02
    jmp FinRxMsg

rRQQ:
    or MsgFlg,04
    jmp FinRxMsg

rNACK:
    or MsgFlg,08
    jmp FinRxMsg

rACK:
    or MsgFlg,10h

FinRxMsg:

    mov MdmCtrl,0
    mov dx,STATUSFX
    mov al,MdmCtrl
    out dx,al

    call RxModeFx      ;Reinicializa la recepci¢n.
    jmp CntInt3


;- - - - - - - - - - - - - - - - - SYNC - - - - - - - - - - - - - - - - - -
RxSYNC:

    jmp CntInt4

;- - - - - - - - - - - - - - - - - SYNT - - - - - - - - - - - - - - - - - -
RxSYNT:

    jmp CntInt5
;- - - - - - - - - - - - - - - - TX IDLE - - - - - - - - - - - - - - - - - -

TxIdle:

    mov TxMsgL,0

    and MdmCtrl,0feh
    mov al,MdmCtrl

    mov dx,CONTROLFX
    out dx,al           ;No TX
    call ModemOff       ;Desactiva el modem y activa el microfono.
    jmp CntInt2


;------------------------- TRANSMISION CANAL 0 SERIE --------------------------
RSITx:
     push cx
     push bx
     push es
     push ax
     push di
     push dx

     mov ah,flag
     and ah,0efh
     cmp ah,10h
     jz  FiRSITx        ;Si el display est  ocupado, no transmite bytes


     mov di,word ptr punterTx
     mov al,byte ptr ds:BuffTxRs[di]
     cmp al,EndChar
     jz FinTX

     mov bx,0fff0h
     mov es,bx
     inc punterTX
     mov byte ptr es:txb0,al
     jmp FiRSITx

FinTx:
     mov punterTx,0
     mov FiTx,1
     call TxOffRs     ;Desactiva la transmisi¢n

FiRSITx:

     mov dx,0fff0h
     mov es,dx
     mov byte ptr es:stic0,07h  ;Resetea flag de interrupci¢n

     pop dx
     pop di
     pop ax
     pop es
     pop bx
     pop cx
     sti
     fint
     iret

;------------------------- RECEPCION CANAL 0 SERIE ------------------------

RSIRx:
     push bx
     push es
     push cx
     push di
     push ax

     mov bx,0fff0h
     mov es,bx
     mov byte ptr es:sric0,07h      ;Resetea flag de RX

     or flag,8             ;Indica que se ha recibido un byte.

     mov cl,es:rxb0        ;El dato recibido queda en CL.
     cmp cl,X_ON
     jnz noxon
     and flag,0efh         ;Indica que el display est  libre.
     jmp FiRSIRx

noxon:
     cmp cl,X_OFF
     jnz noxoff
     or flag,10h           ;Indica que el display est  ocupado.
     jmp NoLleno

noxoff:

     cmp punterRx,404h     ;Comprueba que el buffer no est‚ lleno.
     jnz  NoLLeno

NoLleno:

     mov di,punterRx
     mov BuffRxRs[di],cl
     inc punterRx

FiRSIRx:
     pop ax
     pop di
     pop cx
     pop es
     pop bx
     sti
     fint
     iret


intNMI:
     sti
     iret
END
