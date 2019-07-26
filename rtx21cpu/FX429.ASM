;****************************************************************************
;* NOMBRE      : FX429.ASM                                                  *
;* DESCRIPCION : Submodulo del programa RTX21CPU.ASM (rutinas gestion FX429)*
;* LENGUAJE    : MASM 6.0                                                   *
;* EDITOR      : PWB                                                        *
;* FECHA INICIO: 4/01/1995                                                  *
;* FECHA FINAL :                                                            *
;* OBSERV.     :                                                            *
;****************************************************************************
; DEFINICION DEL PORT0 (control emisora)
;
;
;     8 4 2 1 8 4 2 1 8 4 2 1 8 4 2 1
;    ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿
;    ³5³4³3³2³1³0³ ³ ³ ³ ³ ³ ³ ³ ³ ³ ³
;    ³1³1³1³1³1³1³1³8³7³6³5³4³3³2³1³0³
;    ÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÂÁÂÁÂÁÂÁÄÁÂÙ
;                         ³ ³ ³ ³   ³
;                         ³ ³ ³ ³   ÀÄ /AUDIO  - Salida audio RX 0- ON/ 1- OFF
;                         ³ ³ ³ ³
;                         ³ ³ ³ ³
;                         ³ ³ ³ ÀÄÄÄÄÄ /MODEM  - Habilita salida audio modem y PTT OUT.
;                         ³ ³ ÀÄÄÄÄÄÄÄ /MICRO  - Habilita salida audio micro.
;                         ³ ÀÄÄÄÄÄÄÄÄÄ /SQ     - Entrada squelch.
;                         ÀÄÄÄÄÄÄÄÄÄÄÄ /PTT IN - Entrada ptt emisora.
;



include ConstRAC.inc

;PUBLIC TxOffFx
;PUBLIC TxOnFx
;PUBLIC RxOffFx
;PUBLIC RxOnFx
;PUBLIC ModemOff
;PUBLIC ModemOn
;PUBLIC MicroOff
;PUBLIC MicroOn
;PUBLIC ModemRst
;PUBLIC TxMsgFX
;PUBLIC PttIn
;PUBLIC AudioOn
;PUBLIC AudioOff
;PUBLIC GenTest



delay  PROTO tiempo:word

.CODE


;------------------ DESACTIVA TX ENABLE DEL MODEM ---------------------------

TxOffFx PROC

    and MdmCtrl,TX_OFF  ;Desactiva Tx enable
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al
    ret

TxOffFx ENDP

;-------------------- ACTIVA TX ENABLE DEL MODEM -----------------------------

TxOnFx PROC

    or MdmCtrl,TX_ON    ;Activa Tx enable
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al           ;TX enable
    ret

TxOnFx ENDP

;------------------ DESACTIVA RX ENABLE DEL MODEM ----------------------------

RxOffFx proc ;PUBLIC

    mov RxMsgL,0
    mov RxMsgLn,8
    and MdmCtrl,RX_OFF  ;Desactiva Rx enable
    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al           ;RX enable
    ret

RxOffFx endp

;------------------ ACTIVA RX ENABLE DEL MODEM ----------------------------

RxOnFx proc USES dx ax

    mov RxMsgL,0
    mov RxMsgLn,8
    mov CntBytes2,0
    mov CntBytes,0

    or MdmCtrl,RX_ON         ;Activa Rx enable
    and MdmCtrl,RX_FORMAT_0  ;Pone modo de recepci¢n 0

    mov dx,CONTROLFX
    mov al,MdmCtrl
    out dx,al         ;RX enable
    ret

RxOnFx endp

;----------------------------  DESACTIVA MODEM ------------------------

ModemOff PROC USES ax es

    or  RadioCtrl,MODEM_OFF    ;Desactiva /MODEM

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    ret

ModemOff ENDP

;----------------------------- ACTIVA MODEM --------------------------------

ModemOn PROC USES ax es

    and RadioCtrl,MODEM_ON      ;Activa /MODEM

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    ret

ModemOn ENDP


;--------------------------- DESACTIVA MICRO -------------------------------

MicroOff PROC USES ax es

    or  RadioCtrl,MICRO_OFF      ;Desactiva /MICRO

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    ret

MicroOff ENDP

;----------------------------  ACTIVA MICRO --------------------------------

MicroOn PROC USES ax es

    and RadioCtrl,MICRO_ON    ;Activa /MICRO

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    ret
MicroOn ENDP

;----------------------------  ACTIVA PTT --------------------------------

PttOn PROC USES ax es

    call SQL             ;Se asegura que el SQL est‚ cerrado

    and RadioCtrl,PTT_ON ;Activa PTT

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

   ;Cuando el bit 5 de flag est  a 0 quiere decir que ha de hacer un bucle
   ;de 150 milisegundos para dar tiempo al repetidor.
   ;Si el flag est  a 1 es que el bucle ha de ser de 70 milisegundos para evitar
   ;que el repetidor se cierre.

    mov ah,flag
    and ah,020h

   .if ah==20h

     and flag,0dfh       ;Mira SQL, alarga tiempo de PTT
     invoke delay,150     ;Hace un bucle de unos 70 milisegundos

   .elseif

     invoke delay,150    ;Hace un bucle de unos 150 milisegundos

   .endif

FinPtt:

    ret
PttOn ENDP

;----------------------------  DESACTIVA PTT --------------------------------

PttOff PROC USES ax es

    or  RadioCtrl,PTT_OFF    ;Desactiva PTT

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al

    ret
PttOff ENDP

;------------------ DESACTIVA SALIDA AUDIO RX ---------------------------

AudioOff PROC USES ax es

    and RadioCtrl,AUDIO_ON   ;Activa audio RX

    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al
    ret

AudioOff ENDP


;------------------ ACTIVA SALIDA AUDIO RX ---------------------------

AudioOn PROC USES ax es

    or  RadioCtrl,AUDIO_OFF  ;Desactiva audio RX
    mov ax,0fff0h
    mov es,ax
    mov al,RadioCtrl
    mov byte ptr es:P0,al
    ret

AudioOn ENDP

;---------- COMPRUEBA SI EL CANAL ESTA OCUPADO ---------------------------

SQL PROC USES ax es

    mov ah,flag
    and ah,20h

    .if ah==0

      mov ax,0fff0h
      mov es,ax

     .while (al!=0)      ;Mientras el SQL est‚ cerrado

       mov al,byte ptr es:P0
       and al,10h
     .endw

     invoke delay,random ;Antes de ponerse en transmision espera los
                         ;milisegundos indicados en random, para evitar
                         ;colisiones.
    .endif

   ret

SQL ENDP


;---------------------- Mira si pulsan PTT ----------------------------------
;
;Si pulsan PTT y el bit 0 de FLAG est  a 1 permite PTT.
;Esto quiere decir que ha recibido un mensaje de VEU y el ptt est  permitido.
;
PttIn PROC USES ax es

    mov al,flag
    and al,01h
    cmp al,01h
    jnz FinPttIn            ;El ptt no est  permitido.

    mov ax,0fff0h
    mov es,ax

    mov al,byte ptr es:P0
    xor al,RadioCtrl
    jz  FinPttIn            ;El estado de PTT no ha cambiado.

    mov al,byte ptr es:P0
    and al,20h

 .if al==20h                 ;Si el Ptt est  pulsado

    or flag,10h
;    or flag,20h
    call PttOn              ;activa el ptt de la emisora.
    invoke TxMsg,T_VEU      ;Indica a la central que es mensaje voz.

wat:
    mov al,MdmStat
    and al,10h
    cmp al,10h
    jnz wat                 ;Espera a transmitir el mensaje


    call MicroOn            ;Activa microfono.

    mov al,20h

   .while (al==20h)         ;espera que suelten el PTT
    mov al,byte ptr es:P0
    and al,20h
   .endw

    and flag,0efh
    call PttOff             ;desactiva el ptt de la emisora.
    call MicroOff           ;Desactiva microfono
    call RxOnFx             ;Pone el modem en recepci¢n

 .endif

FinPttIn:

    ret
PttIn ENDP

;---------------- ACTIVA PROCESO TRANSMISION MENSAJE ------------------------

TxMsgFX PROC

;    cli
    mov al,flag
    and al,08h

  .if al==00h          ;s¢lo transmitir  el mensaje si est  permitido

    mov al,RadioCtrl
    and al,04h

  .if (al==04h)        ;Si el ptt no est  puesto lo pone
    call PttOn         ;Activa ptt de la emisora
  .endif

    call ModemOn       ;Activa salida modem y desactiva el micro.

    mov TxMsgL,0
    mov MdmStat,0

    call TxOnFx        ;Pone el modem en modo TX
    mov dx,TXDATAFX
    mov al,BuffTxFX[0]
    out dx,al          ;Transmite el primer byte del buffer
                       ;de los demas se encarga el interrupt.
    inc TxMsgL

  .endif

FinMsg:

;    sti
    ret

TxMsgFX ENDP

;--------------------------- GENERA EL TEST --------------------------------

GenTest PROC

.while (1)

   mov BuffTxFX[0],0aah
   mov BuffTxFX[1],0aah
   mov BuffTxFX[2],0aah
   mov BuffTxFX[3],0aah
   mov BuffTxFX[4],0aah
   mov BuffTxFX[5],0aah
   mov BuffTxFX[6],0aah
   mov BuffTxFX[7],0aah
   mov BuffTxFX[9],0aah
   mov BuffTxFX[10],0aah

   mov flag,10h
   and RadioCtrl,11111011b    ;Activa PTT

   mov ax,0fff0h
   mov es,ax
   mov al,RadioCtrl
   mov byte ptr es:P0,al

   call TxOnFx        ;Pone el modem en modo TX

   mov TxMsgL,0
   mov MdmStat,0

   mov TxMsgLn,0ah

   mov dx,TXDATAFX
   mov al,BuffTxFX[0]
   out dx,al          ;Transmite el primer byte del buffer
                       ;de los demas se encarga el interrupt.
   inc TxMsgL


  mov al,01

 .while (al==01)
  mov al,MdmCtrl
  and al,01h
 .endw

.endw

  ret
GenTest ENDP
;------------------------- RESETEA MODEM -----------------------------------

ModemRst proc USES dx ax

    mov dx,CONTROLFX
    mov al,04
    out dx,al

    mov al,00
    out dx,al       ;Reset modem

    mov dx,STATUSFX
    in al,dx        ;Resetea las interrupciones del modem

    ret

ModemRst endp


;------------------------- BUCLE DE X MILISEGUNDOS --------------------------
;
;En tiempo han de estar los milisegundos.
;
delay PROC tiempo:word

    mov cx,tiempo

bc:
    mov ax,364

buc:

    dec ax
    nop
    nop
    cmp ax,0
    jnz buc
    loop bc

    ret
delay ENDP


END
