;****************************************************************************
;* NOMBRE      : INTERV25.ASM                                               *
;* DESCRIPCION : Submodulo del programa RTX21CPU.ASM                        *
;* LENGUAJE    : MASM 6.0                                                   *
;* EDITOR      : PWB                                                        *
;* FECHA INICIO: 4/01/1995                                                  *
;* FECHA FINAL :                                                            *
;* OBSERV.     : Gestiona las interrupciones:                               *
;*                                          - FX429                         *
;*                                          - TIMER                         *
;*                                          - CANAL 0 SERIE                 *
;*                                                                          *
;****************************************************************************

 include ConstRAC.inc

 ;PUBLIC IntNMI
 ;PUBLIC RSITx
 ;PUBLIC RSIRx
 ;PUBLIC IntTIMER0


fint macro
  db 0fh,92h
 endm

.CODE

;----------------------------- INTP0 ----------------------------------------

HandIntP0 PROC USES ax bx cx dx di si es

    mov dx,STATUSFX
    in al,dx

    mov MdmStat,al      ;En MdmStat est  el estado del modem.

    mov al,MdmStat
    and al,08h
    cmp al,08h
    jnz  CntInt1
    call TxBufferFx     ;Transmite siguiente byte al modem.

CntInt1:

    mov al,MdmStat
    and al,01h
    cmp al,01h
    jnz CntInt3
    call RxBuff         ;RxData.

CntInt3:

    mov al,MdmStat
    and al,40h
    cmp al,40h
    jnz CntInt4
    call RxSYNC         ;Recibe un SYNC

CntInt4:

    mov al,MdmStat
    and al,80h
    cmp al,80h
    jnz CntInt5
    call RxSYNT         ;Recibe un SYNT

CntInt5:
    mov al,MdmStat
    and al,10h
    cmp al,10h
    jnz CntInt2
    call TxIdle         ;Tx Idle.

CntInt2:
    mov al,MdmStat
    and al,20h
    cmp al,20h
    jnz CntInt6
    call TimerInt       ;Timer Interrupt

CntInt6:

IntP0Fin:

    mov ax,0fff0h
    mov es,ax
    mov byte ptr es:EXIC0,00

    sti
    fint
    iret

HandIntP0 ENDP


;- - - - - - - - - Transmite siguiente byte al modem - - - - - - - - - - - - -

TxBufferFx PROC

    mov al,MdmCtrl
    and al,01h
    cmp al,01h
    jz TxOk

    call ModemRst       ;Esta interrupci¢n es erronea porque el bit de TX
                        ;no est  permitido. Reinicializamos el modem por si
                        ;estubiera mal inicializado.
    call RxOnFX         ;Permite recepci¢n.
    jmp  FinTxBuf

TxOk:
    mov di,TxMsgL
    cmp di,TxMsgLn      ;Tope de largo en bytes del mensaje a transmitir.
    jge FinTxBuf        ;Para un mensaje short data  - 0ah

    mov al,BuffTxFX[di]
    mov dx,TXDATAFX
    out dx,al           ;Transmite el siguiente byte al modem.

  .if (di==4)

    or MdmCtrl,02h
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al           ;Activa paridad interna.

  .endif

    inc TxMsgL

FinTxBuf:

    ret
TxBufferFx ENDP


;- - - - - - - - - - - - - - - - RX DATA - - - - - - - - - - - - - - - - - -

RxBuff PROC

    mov dx,RXDATAFX
    in  al,dx           ;Leo el byte recien recibido.

    mov di,CntBytes2
    mov BuffRxFx[di],al ;Guardo el byte recibido en el buffer.

nomem:

    inc CntBytes
    inc CntBytes2       ;Apunto a la siguiente posici¢n del buffer.
    inc RxMsgL

    cmp CntBytes,8      ;Cada 8 bytes(1 slot) miro si el CRC es correcto.
    jnz nosl

    mov al,MdmStat
    and al,02b
    mov StatCRC,al      ;Guardo el status del CRC.
    mov CntBytes,0
    sub CntBytes2,2

nosl:
    mov ax,RxMsgL
    cmp ax,RxMsgLn      ;Compruebo si es final de buffer
    jz No0              ;Ha terminado de recibir el mensaje.
    ret

No0:
    cmp StatCRC,0
    jz  FinRxMsg       ;El CRC no es correcto, rechazo el mensaje.

    mov al,flag
    and al,04
    cmp al,0
    jz  NoSLOTS        ;No est  recibiendo los SLOTS
    jmp NoHEAD         ;Ya ha recibido todos los SLOTS.


NoSLOTS:

    mov ah,BuffRxFx+2  ;Miro el tipo de mensaje por si han de venir SLOTS.
    and ah,07h         ;Para saber si es un HEAD se ha de leer el 'CAT', este
    mov al,BuffRxFx+3  ;queda comprendido entre el byte 2 y el 3 del BuffRxFx.
    and al,080h        ;'CAT' esta mitad en ah y mitad en al.
    clc
    rol ax,1           ;'CAT' esta en ah
    cmp ah,0           ;Siempre que 'CAT' sea 0 es que no es un HEAD.
    jz  NoHEAD         ;No es un HEAD


    mov NumBlock,ah    ;se apunta el n£mero de bloque recibido.

    mov ah,BuffRxFx+3  ;Miro la cantidad de SLOTS que han de venir.
    and ah,70h
    clc
    mov cl,4
    ror ah,cl

    inc ah

    mov cl,3
    shl ah,cl          ;multiplica ahx8

    mov al,ah
    xor ah,ah
    add RxMsgLn,ax

cnt1:

    or flag,04         ;Flag para indicar que vienen SLOTS.

    mov dx,CONTROLFX
    or MdmCtrl,RX_FORMAT_1
    mov al,MdmCtrl
    out dx,al         ;Indica al modem que ahora viene SLOTS.
    ret

NoHEAD:

    mov al,BuffRxFx ;Primero miro si el mensaje es para mi.
    and al,07fh
    cmp al,MiPrefx  ;Compruebo si el prefijo coincide con el mio.
    jnz FinRxMsg

    mov ah,BuffRxFx+1
    mov al,BuffRxFx+2
    and al,0fch     ;descartamos el n£mero de bloque o el 'CAT'
    mov cl,3
    ror ax,cl
    ;and ax,0001h
    cmp ax,00       ;Si el indicativo es 00 indica que es mensaje para
    jnz  nogrup     ;grupo, por lo que s¢lo har  caso del prefijo.
    or  flag,08h    ;Indica que el mensaje que ha recibido es de grupo
    jmp sigr        ;para que la rutina de transmisi¢n de mensajes no
                    ;transmita.
nogrup:

    cmp MiIndic,ax
    jnz FinRxMsg    ;Mi indicativo no es el mismo, rechazo mensaje.
    and flag,0f7h   ;Indica que el mensaje que ha recibido es individual.

sigr:
    mov al,flag
    and al,04

  .if al==04

    mov ah,BuffRxFx+4
    and ah,01fh
    mov al,BuffRxFx+5
    jmp skp

  .endif

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

    mov cl,3
    shl bx,cl

    and bx,0fff8h
    mov RxIndic,bx  ;Memorizo el indicativo recibido.

skp:

    mov al,BuffRxFx+3
    and al,80h
    mov ah,BuffRxFx+2
    and ah,07h
    mov cl,7
    shr ax,cl

;  0   -   Recibido Mensaje ASI
;  1   -   Recibido Mensaje HEAD
;  2   -   Recibido Mensaje MOD
;  3   -   Recibido Mensaje ANU
;  4   -   Recibido Mensaje AHYC
;  5   -   Recibido Mensaje NACK
;  6   -   Recibido Mensaje ACK
;  7   -   Recibido Mensaje VEU
;  8   -   Recibido Mensaje FI_VEU
;  9   -   Recibido Mensaje CAM_IND

     or flag,20h      ;Indica que no ha de mirar el SQL u PTT corto


    .if ax>0          ;El mensaje recibido es un HEAD

     or MsgFlg,02h
     jmp FinRxMsg
    .endif

    mov ah,BuffRxFx+3
    and ah,1ch

 .if (ah==01ch)       ;El mensaje recibido es un RQC

    mov ah,BuffRxFx[5]
    and ah,02
   .if ah==00
     or MsgFlg,01h    ;El mensaje recibido es un ASI
     jmp FinRxMsg
   .endif

   .if ah==02
     or MsgFlg,04h    ;El mensaje recibido es un MOD
     jmp FinRxMsg
   .endif
 .endif


 .if (ah==14h)        ;El mensaje recibido es un AHYC

    or MsgFlg,10h
    jmp FinRxMsg
 .endif

 .if (ah==18h)       ;El mensaje recibido es un RQQ
    mov ah,BuffRxFx[5]
    and ah,1fh

   .if (ah==09h)
     or MsgFlg,08h   ;Recibe un ANU
     jmp FinRxMsg
   .endif

   .if (ah==0ah)
     or MsgFlg,80h   ;Recibe un VEU
     and flag,0dfh   ;Mira SQL, alarga tiempo de PTT
     jmp FinRxMsg
   .endif
   .if (ah==0bh)
     or MsgFlg,100h  ;Recibe un FI_VEU
     jmp FinRxMsg
   .endif
   .if (ah==0ch)

     or MsgFlg,200h  ;Recibe un CAM_IND
     jmp FinRxMsg
   .endif
 .endif

 .if (ah==0ch)        ;El mensaje recibido es un NACK
     and flag,0dfh   ;Mira SQL, alrga tiempo de PTT
     or MsgFlg,20h
     jmp FinRxMsg
 .endif

 .if (ah==00h)        ;El mensaje recibido es un ACK
     and flag,0dfh   ;Mira SQL, alrga tiempo de PTT
     or MsgFlg,40h
     jmp FinRxMsg
 .endif


FinRxMsg:

    and flag,0fbh   ;no slots.
    mov MdmCtrl,0
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al

    call RxOnFx      ;Reinicializa la recepci¢n.
    ret

RxBuff ENDP


;- - - - - - - - - - - - - - - - TX IDLE - - - - - - - - - - - - - - - - - -

TxIdle PROC

    mov TxMsgL,0
    mov MdmCtrl,0

    mov ah,flag
    and ah,10h

   .if ah==0

    mov al,MdmCtrl
    mov dx,CONTROLFX
    out dx,al           ;No TX
    call ModemOff       ;Desactiva el modem

    call RxOnFx         ;Pone el modem en recepci¢n


    invoke delay,50
    call PttOff         ;Desactiva el PTT
   .endif

    ret

TxIdle ENDP


;- - - - - - - - - - - - - - - - - SYNC - - - - - - - - - - - - - - - - - -
RxSYNC proc

    mov RxMsgL,0
    mov RxMsgLn,8
    mov CntBytes2,0
    mov CntBytes,0
    ret
RxSYNC endp


;- - - - - - - - - - - - - - - - - SYNT - - - - - - - - - - - - - - - - - -
RxSYNT PROC

    mov RxMsgL,0
    mov RxMsgLn,8
    mov CntBytes2,0
    mov CntBytes,0
    ret
RxSYNT ENDP

;- - - - - - - - - - - - TIMER INTERRUPT - - - - - - - - - - - - - - - - - -

TimerInt PROC

    and MdmCtrl,0fh     ;Desactiva las interrupciones del TIMER
    mov al,MdmCtrl
    mov dx,CONTROLFX
    out dx,al
    ret

TimerInt ENDP


;------------------------- TRANSMISION CANAL 0 SERIE --------------------------
RSITx PROC USES cx bx es ax di dx

     mov bx,0fff0h
     mov es,bx
     mov byte ptr es:STIC0,07h

     or  flag,2
     sti
     fint
     iret
RSITx ENDP


;------------------------- RECEPCION CANAL 0 SERIE ------------------------

RSIRx PROC USES ax bx cx dx es si di

     mov bx,0fff0h
     mov es,bx
     mov byte ptr es:SRIC0,07h      ;Resetea flag de RX

     mov cl,es:RXB0        ;El dato recibido queda en CL.

     mov ax,KeybTail
     inc ax
     cmp KeybHead,ax       ;Comprueba que el buffer no est‚ lleno.
     jz  FiRSIRx           ;Si el buffer est  lleno, no acepta m s car cteres

NoLLeno:

     mov di,KeybTail
     mov BuffRxRs[di],cl

     inc KeybTail
     cmp KeybTail,MAX_MSG_RS

     jnz FiRSIRx
     mov KeybTail,0

FiRSIRx:

     sti
     fint
     iret
RSIRx ENDP

IntTIMER0 PROC uses ax es

     mov ax,0fff0h
     mov es,ax

     call RstTimer
     mov byte ptr es:TMIC0,07h      ;Activa interrupciones TIMER0
     dec segundos

     .if (random>100)
        mov random,100
     .endif

     dec random

     fint
     sti
     iret

IntTIMER0 ENDP


IntNMI PROC ;PUBLIC
     sti
     iret
IntNMI ENDP
END
