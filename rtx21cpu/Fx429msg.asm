;****************************************************************************
;* NOMBRE      : FX429MSG.ASM                                               *
;* DESCRIPCION : Submodulo del programa RTX21CPU.ASM                        *
;* LENGUAJE    : MASM 6.0                                                   *
;* EDITOR      : PWB                                                        *
;* FECHA INICIO: 4/01/1995                                                  *
;* FECHA FINAL :                                                            *
;* OBSERV.     : Gestiona los mensajes via radio (transmisi¢n y recepci¢n)  *
;****************************************************************************

 INCLUDE constRAC.INC

 ;PUBLIC RxMsg
 ;PUBLIC TxMsg


 TxMsg PROTO NumMsg : WORD
 RxMsg PROTO NumMsg : WORD

.CODE
;-------------------- GESTIONA MENSAJE RECIBIDO  --------------------------
;En ax ha de estar el tipo de mensaje recibido

RxMsg PROC USES es ds ax bx di si NumMsg : WORD
      LOCAL nBlock    :byte


    cmp NumMsg,001h
    jz  rASI
    cmp NumMsg,004h
    jz  rMOD
    cmp NumMsg,008h
    jz  rANU
    cmp NumMsg,010h
    jz  rAHYC
    cmp NumMsg,040h
    jz  rACK
    cmp NumMsg,080h
    jz  rVEU
    cmp NumMsg,100h
    jz  rFIVEU
    cmp NumMsg,200h
    jz  rCAM_IND

;    cmp NumMsg,020h
;    jz  rNACK

FiRxMsg:
     ret


;----------------- RECIBE UN MENSAJE TIPO ANU -------------------------------

rANU:

    and MsgFlg,0f7h       ;Borra flag que indica ANU recibido.
    invoke TxByteRs,P_ANU ;Indica al display anulaci¢n
    mov al,0
    call TxACK            ;Contesta a la central con un ACK

    invoke espera,2       ;Espera 2 segundos antes de continuar.

;  .while (al!=T_SI)       ;Espera que el terminal conteste con SI.
;    call getch
;  .endw

    invoke TxByteRs,P_LLI ;Pone pantalla de LLIURE
    jmp FiRxMsg

;----------------- RECIBE UN MENSAJE TIPO VEU -------------------------------

rVEU:

    and MsgFlg,07fh       ;Borra flag que indica VEU recibido.
    or  flag,01h          ;Permite la pulsaci¢n de PTT
    call AudioOn          ;Permite la entrada audio RX
    jmp FiRxMsg

;---------------- RECIBE UN MENSAJE TIPO FI VEU -----------------------------

rFIVEU:

    and MsgFlg,0feffh     ;Borra flag que indica FI VEU recibido.
    and flag,0feh         ;Deshabilita la pulsaci¢n de PTT
    call AudioOff         ;Deshabilita audio RX
    invoke TxByteRs,P_FIVEU ;Indica al display FI VEU
    jmp FiRxMsg


;----------------RECIBE UN MENSAJE TIPO AHYC --------------------------------

rAHYC:
                     ;Cuando el movil recibe un AHYC, le indica que ha
                     ;de transmitir el n£mero de mec nico.
                     ;El movil transmite el n£mero de mec nico y espera un
                     ;ACK, si en 5 intentos la central no ha contestado,
                     ;reiniciar  todo el sistema partiendo de la pantalla de
                     ;mec nico.

    and MsgFlg,0efh  ;Borra flag que indica rAHYC recibido.
    mov PenTx,T_MEC  ;Har  el resto de intentos con un RQC
    mov EspRx,40h    ;Espera un ACK
    mov ScrMode,02h  ;La pantalla que crear  despu‚s de recibir un ACK
                     ;ha de ser la de LLIURE.

    mov intentos,5   ;5 intentos.
    mov RxIndic,INDI_CENT
    call TxNuMecan   ;Transmite el n£mero de mec nico.
    jmp FiRxMsg

;------------------------- RECIBE UN ASI ------------------------------------
; Cuando recibe un ASI, indica al display que va a transmitir un fichero, con
; lo cual lo pone en modo SERVEI.
;
; Luego coge el bloque recibido y lo guarda a continuaci¢n
; del £ltimo fichero recibido, el puntero pFichero, apunta donde se ha
; de ubicar este fichero.
; La variable TotalFich tiene el total de ficheros recibidos, a cada fichero
; recibido se incrementa 1.
;
; Los campos que no se completan se rellenan de espacios.
; A cada bloque recibido, se transmite un ACK para pedir el siguiente bloque,
; si en 10 segundos no ha recibido bloque, se aborta el proceso transmitiendo
; al display la pantalla de LLIURE y devolviendo un 0ffffh en ax.
;
; En la variable NumBlock est  el n£mero de bloque que se est  procesando,
; si el bloque recibido no es el que se espera lo ignora.
;
; El bloque recibido est  en SLOT1-SLOT8.
; En nBlock est  el n£mero de bloque que acaba de recibir.
;
; Por £ltimo transmite el fichero recibido a la consola a¤adiendo un 1ah para
; indicar fin de fichero.
; La variable PosFicher se iguala a TotalFich.

rASI:

    call IniTxFich      ;Pone la consola en modo SERVEI y espera a que
                        ;esta transmita un XON
    mov ScrMode,P_SER   ;Actualiza el modo en que est  el terminal.

esASI:
    call TxAHYC         ;Indica al CGC que transmita el HEAD con los datos
    and MsgFlg,0feh     ;Borra flag que indica ASI recibido

    mov di,word ptr pFichero
    mov nBlock,1

    mov bx,offset nBytesASI

buc:
    mov cl,[bx]         ;En cx est  el largo del campo a ubicar.

BlockE:

    mov segundos,TIEMPO_MAX+7 ;Esperar  el tiempo indicado en TIEMPO_MAX
    call RstTimer   ;Resetea el timer para que empiece a contar desde ahora.
    call TimerOn    ;Pone en marcha el TIMER
    and MsgFlg,0fdh ;Borra flag que indica HEAD recibido

 .while (segundos>0);Espera a que entre un HEAD
    mov ax,MsgFlg
    and ax,02h
    cmp ax,02h
    jz  OkRx        ;Ha entrado un HEAD

    mov ax,MsgFlg   ;La ST no ha recibido el AHYC del movil y ha hecho otro
    and ax,01h      ;requerimiento, por lo que el movil ha de contestar con
    cmp ax,01h      ;otro AHYC.
    jz  esASI

 .endw


    invoke TxByteRs,P_LLI   ;Anula el servicio poniendo la pantalla LLIURE
    jmp FiRxMsg

OkRx:
    mov ah,NumBlock
    cmp ah,nBlock    ;El bloque ha de coincidir con el que espera
    jg  BlockE       ;Si el n£mero de bloque es m s grande del que espera rechaza
    jb  BlockE2      ;Si el bloque ya lo tiene, transmite un ok y espera el siguiente.

    mov si,offset slot1
    mov dx,48       ;n£mero total de bytes en el bloque

buc2:
    mov ah,ds:[si]  ;
    inc si
    cmp dx,0
    jz  BlockEnd
    dec dx
    cmp ah,0ah
    jz  buc3
    mov ds:[di],ah  ;slot1 -> fichero
    inc di
    dec cl          ;decrementa n£mero de bytes total
;    cmp dx,0
;    jz  BlockEnd
    jmp buc2

buc3:
    .if (cx>0)
     mov ah,' '
     mov ds:[di],ah  ;rellena los bytes restantes de espacios
     inc di
     loop buc3
    .endif

    inc bx           ;Siguiente campo
    mov al,[bx]
    mov cl,al
    cmp al,0         ;Es final?
    jnz buc2

    call TxACK       ;Fin

    ;Despues de recibir todo el fichero, lo transmite al terminal

    add pFichero,431 ;pFichero apunta al siguiente fichero
    inc TotalFich    ;total ficheros.
    mov al,TotalFich
    mov PosFicher,al ;Ahora el £ltimo fichero mostrado ser  el £ltimo recibido.

    invoke TxFichero,TotalFich ;Transmite el ultimo fichero recibido
    invoke TxByteRs,EOF        ;Transmite final de fichero al terminal.
    jmp FiRxMsg


BlockEnd:
    inc nBlock

BlockE2:

    call TxACK       ;Pide siguiente bloque
    jmp BlockE


;----------------------------- RECIBE UN MOD -------------------------------

rMOD:

    ;Si el terminal est  en LLIURE o ATURAT, rechaza el MOD.

    .if (ScrMode==P_LLI || ScrMode==P_ATU)
     jmp FiRxMsg
    .endif

    call IniTxFich  ;Avisa a la consola de que va a transmitir un MOD y
                    ;espera a que esta transmita un XON
esMOD:

    call TxAHYC     ;Indica al CGC que transmita el HEAD con los datos
    and MsgFlg,0fbh  ;Borra flag que indica MOD recibido.


    mov segundos,TIEMPO_MAX+7 ;Esperar  el tiempo indicado en TIEMPO_MAX.
    call RstTimer   ;Resetea el timer para que empiece a contar desde ahora.
    call TimerOn    ;Pone en marcha el TIMER
    and MsgFlg,0fdh ;Borra flag que indica HEAD recibido

 .while (segundos>0);Espera a que entre un HEAD
    mov ax,MsgFlg
    and ax,02h
    cmp ax,02h
    jz  Ox          ;Ha entrado un HEAD

    mov ax,MsgFlg   ;La ST no ha recibido el AHYC del movil y ha hecho otro
    and ax,04h      ;requerimiento, por lo que el movil ha de contestar con
    cmp ax,04h      ;otro AHYC.
    jz  esMOD
 .endw

    invoke TxByteRs,P_LLI   ;Anula el servicio poniendo la pantalla LLIURE
    jmp FiRxMsg

Ox:
    mov di,offset FichMod
    mov bx,offset nBytesMOD
    xor cx,cx

bucM:
    mov cl,[bx]     ;En cx est  el largo del campo a ubicar.

bcM:
    lea si,slot1
    mov dx,48       ;n£mero total de bytes en el bloque

bu2:
    mov ah,ds:[si]  ;
    inc si
    dec dx
    cmp ah,0ah
    jz  bu3
    mov ds:[di],ah  ;slot1 -> fichero MOD
    inc di
    dec cl          ;decrementa n£mero de bytes total
    cmp dx,0
    jz  BlockEnd
    jmp bu2

bu3:
    .if (cx>0)
     mov ah,' '
     mov ds:[di],ah  ;rellena los bytes restantes de espacios
     inc di
     loop bu3
    .endif

    inc bx           ;Siguiente campo
    mov al,[bx]
    mov cl,al
    cmp al,0         ;Es final?
    jnz bu2

    call TxACK       ;Fin

    push ds
    pop es

    mov si,offset FichMod
    mov bx,word ptr pFichero
    sub bx,431
    mov di,bx        ;Cambia los campos No. d'expedient
    mov cx,21        ;                  No. del servei
    cld              ;                  Client
    rep movsb        ;                  Subclident

    mov di,pFichero  ;Cambia el campo Codi.
    sub di,431
    add di,420
    mov cx,4
    rep movsb

    mov di,pFichero  ;Cambia el campo Cost.
    sub di,431
    add di,424
    mov cx,7
    rep movsb

    mov al,TotalFich
    mov PosFicher,al ;Ahora el £ltimo fichero mostrado ser  el £ltimo recibido.
    invoke TxFichero,TotalFich
    invoke TxByteRs,EOF        ;Transmite final de fichero al terminal.
    invoke TxByteRs,ScrMode    ;Pone el display en el modo en que estaba antes
                               ;de transmitir el MOD.
    ret

;----------------------- RECIBE UN CAM_IND ----------------------------------
rCAM_IND:

    and Msgflg,0fdffh ;Resetea flag que indica que ha recibido un CAM_IND

    call TxAHYC     ;Pide a la central el HEAD

    mov segundos,TIEMPO_MAX ;Esperar  el tiempo indicado en TIEMPO_MAX.
    call RstTimer   ;Resetea el timer para que empiece a contar desde ahora.
    call TimerOn    ;Pone en marcha el TIMER
    and MsgFlg,0fdh ;Borra flag que indica HEAD recibido

 .while (segundos>0);Espera a que entre un HEAD
    mov ax,MsgFlg
    and ax,0002
    cmp ax,0002
    jz  OC           ;Ha entrado un HEAD
 .endw

    jmp FiRxMsg

OC:
    call TxACK       ;Transmite un ACK para indicar que est  conforme

    mov ax,70h
    mov es,ax
    mov di,0


    mov al,slot1

    mov MiPrefx,al
    mov es:[di],al
    inc di

    mov ah,slot1+1
    mov al,slot1+2

    mov MiIndic,ax
    mov es:[di],word ptr ax
    jmp FiRxMsg


;---------------- RECIBE UN MENSAJE TIPO ACK --------------------------------

rACK:

  ;Siempre que recibe un ACK del mensaje SOS, permite audio y pulsaci¢n de
  ;PTT.

  .if (PenTx==T_EME)
    or  flag,01h          ;Permite la pulsaci¢n de PTT
    call AudioOn          ;Permite la entrada audio RX
  .endif

    mov PenTx,0
    mov EspRx,0

    and MsgFlg,0bfh  ;Borra flag que indica ACK recibido.
    cmp ScrMode,0
    jnz pScreen

    ret

pScreen:
    invoke TxByteRs,ScrMode
    ret
RxMsg ENDP


;================= SECCION TRANSMISION DE MENSAJES ==========================



;------------------------- TRANSMISION DE MENSAJES --------------------------

;En ax ha de estar el tipo de mensaje a transmitir

TxMsg PROC NumMsg : WORD

    cmp NumMsg,T_MEC
    jz  pRQC
    cmp NumMsg,T_DES
    jz  pDES
    cmp NumMsg,T_OCU
    jz  pOCU
    cmp NumMsg,T_PAR
    jz  pPAR
    cmp NumMsg,T_FIN
    jz  pFIN
    cmp NumMsg,T_EME
    jz  pEME
    cmp NumMsg,T_VEU
    jz  pVEU
    cmp NumMsg,T_TAN
    jz  pTAN
    cmp NumMsg,T_LLI
    jz  pLLI

EndTxMsg:

    ret

pRQC:

    mov al,NumMecan[0] ;Siempre que el primer byte del n£mero de mec nico sea
                       ;00 preguntar  el n£mero de mec nico.
  .if al==0

    mov di,offset NumMecan
    mov cx,3           ;M ximo 3 car cteres.
    call gets          ;Pide n£mero de mec nico.
    mov intentos,5
  .endif

    mov RxIndic,INDI_CENT
    mov PenTx,T_MEC
    mov EspRx,10h      ;Espera un AHYC
    call TxRQC
    jmp EndTxMsg


pDES:

  .if (ScrMode==P_DES || ScrMode==P_SER || ScrMode==0)

    mov RxIndic,INDI_CENT
    mov PenTx,T_DES
    mov EspRx,40h     ;Espera un ACK
    mov ScrMode,P_DES ;pantalla a generar DESPLA€AMENT
    mov ah,DES
    call TxRQQ        ;Transmite un RQQ con info. DES

  .endif

    jmp EndTxMsg

pOCU:

  .if (ScrMode==P_OCU || ScrMode==P_DES || ScrMode==0)

    mov RxIndic,INDI_CENT
    mov PenTx,T_OCU
    mov EspRx,40h     ;Espera un ACK
    mov ScrMode,P_OCU ;pantalla a generar OCUPAT
    mov ah,OCU
    call TxRQQ        ;Transmite un RQQ con info. OCU

  .endif

    jmp EndTxMsg

pPAR:

  .if (ScrMode==P_ATU || ScrMode==P_LLI || ScrMode==0)

    mov RxIndic,INDI_CENT
    mov PenTx,T_PAR
    mov EspRx,40h     ;Espera un ACK
    mov ScrMode,P_ATU ;pantalla a generar ATURAT
    mov ah,PAR
    call TxRQQ        ;Transmite un RQQ con info. PAR

  .endif

    jmp EndTxMsg

pFIN:

  .if (ScrMode==P_LLI || ScrMode==P_OCU || ScrMode==0)

    mov RxIndic,INDI_CENT
    mov PenTx,T_FIN
    mov EspRx,40h     ;Espera un ACK
    mov ScrMode,P_LLI ;pantalla a generar LLIURE
    mov ah,FIN
    call TxRQQ        ;Transmite un RQQ con info. FIN

  .endif

    jmp EndTxMsg

pEME:
    mov RxIndic,INDI_CENT
    mov PenTx,T_EME
    mov ScrMode,0     ;no generar  pantalla
    mov EspRx,40h     ;Espera un ACK
    mov ah,EME
    call TxRQQ        ;Transmite un RQQ con info. EME
    jmp EndTxMsg

pVEU:
    mov RxIndic,INDI_CENT
    mov PenTx,0       ;No espera ACK
    mov ScrMode,0     ;no generar  pantalla
    mov ah,VEU
    call TxRQQ        ;Transmite un RQQ con info. VEU
    jmp EndTxMsg

pTAN:

  .if (ScrMode==P_MEC || ScrMode==P_LLI || ScrMode==0)

    mov NumMecan[0],0 ;Indica que ha de preguntar el n£mero de mec nico.

    mov RxIndic,INDI_CENT
    mov PenTx,T_TAN
    mov EspRx,40h     ;Espera un ACK
    mov ScrMode,P_MEC ;pantalla a generar MECANIC
    mov ah,TAN
    call TxRQQ        ;Transmite un RQQ con info. TAN

  .endif

    jmp EndTxMsg

pLLI:

   .if (ScrMode==P_LLI || ScrMode==P_ATU || ScrMode==P_OCU || ScrMode==P_MEC || ScrMode==0)

    mov RxIndic,INDI_CENT
    mov PenTx,T_LLI
    mov EspRx,40h     ;Espera un ACK
    mov ScrMode,P_LLI ;pantalla a generar LLIURE
    mov ah,LLI
    call TxRQQ        ;Transmite un RQQ con info. LLI

   .endif

    jmp EndTxMsg


TxMsg ENDP

;----------------------------- TRANSMITE UN RQC ---------------------------
TxRQC PROC  USES AX ES

    call IniCab
    mov ax,MiIndic
    mov BuffTxFX[9],al
    mov BuffTxFX[8],ah
    mov BuffTxFX[7],0

    mov cx,5
rr:
    rcl BuffTxFX[9],1
    rcl BuffTxFX[8],1
    rcl BuffTxFX[7],1
    loop rr

    and BuffTxFX[9],0f0h
    or  BuffTxFX[9],10h
    or  BuffTxFX[7],5ch
    mov TxMsgLn,0ah
    call TxMsgFX
    ret

TxRQC ENDP


;----------------------------- TRANSMITE UN RQQ ---------------------------
TxRQQ PROC uses ax cx es

    push ax
    call IniCab

    mov ax,MiIndic

    mov BuffTxFX[9],al
    mov BuffTxFX[8],ah
    mov BuffTxFX[7],0

    mov cx,5
rr:
    rcl BuffTxFX[9],1
    rcl BuffTxFX[8],1
    rcl BuffTxFX[7],1
    loop rr

    and BuffTxFX[9],0f0h
    or  BuffTxFX[7],58h

    pop ax
    or  BuffTxFX[9],ah
    mov TxMsgLn,0ah
    call TxMsgFX
    ret

TxRQQ ENDP


;---------------------------- TRANSMITE UN AHYC -----------------------------

TxAHYC PROC USES ax cx es

    call IniCab     ;Compone cabecera mensaje

    mov ax,MiIndic
    mov BuffTxFX[9],al
    mov BuffTxFX[8],ah
    mov BuffTxFX[7],0

    mov cx,5
rr:
    rcl BuffTxFX[9],1
    rcl BuffTxFX[8],1
    rcl BuffTxFX[7],1
    loop rr

    and BuffTxFX[9],0f0h
    or  BuffTxFX[9],10h
    or  BuffTxFX[7],54h
    mov TxMsgLn,0ah
    call TxMsgFX
    ret

TxAHYC ENDP

;---------------------------- TRANSMITE UN ACK -----------------------------

TxACK PROC USES ax cx es

    call IniCab     ;Compone cabecera mensaje

    mov ax,MiIndic
    mov BuffTxFX[9],al
    mov BuffTxFX[8],ah
    mov BuffTxFX[7],0

    mov cx,5
rr:
    rcl BuffTxFX[9],1
    rcl BuffTxFX[8],1
    rcl BuffTxFX[7],1
    loop rr

    or BuffTxFX[7],04h

    and BuffTxFX[9],0f0h
    or  BuffTxFX[9],10h
    or  BuffTxFX[7],20h
    mov TxMsgLn,0ah
    call TxMsgFX
    ret

TxACK ENDP

;------------------ TRANSMITE EL NUMERO DE MECANICO  ------------------------

TxNuMecan PROC uses es ax cx


    call IniCab       ;Compone cabecera mensaje

    mov BuffTxFX[9],0
    mov BuffTxFX[8],0
    mov BuffTxFX[7],0

    mov al,MiPrefx
    mov BuffTxFX[8],al

    mov al,0
    mov cx,5
lp:
    rcl BuffTxFX[8],1
    rcl BuffTxFX[7],1
    loop lp

    mov ax,MiIndic

    or  BuffTxFX[8],ah
    mov BuffTxFX[9],al

    or  BuffTxFX[7],80h


    mov BuffTxFX[10],0

    mov ax,cs:NSerie
    mov BuffTxFX[11],ah         ;En el SLOT1 bytes 1,2 est  en n£mero de serie.
    mov BuffTxFX[12],ah

    mov ah,NumMecan[0]
    mov BuffTxFX[13],ah
    mov ah,NumMecan[1]
    mov BuffTxFX[14],ah
    mov ah,NumMecan[2]  ;En el SLOT 1 bytes 3,4,5 est  el n£mero de mec nico
    mov BuffTxFX[15],ah

    mov TxMsgLn,10h     ;Largo mensaje a transmitir para un HEAD y un SLOT.
    call TxMsgFX
    ret

TxNuMecan ENDP




IniCab PROC uses es     ;Prepara el encabezado de los mensajes

; Compone el preambulo y el SYNC

    mov BuffTxFX[0],0aah        ;    10101010   PREAM
    mov BuffTxFX[1],0aah        ;    10101010   PREAM
    mov BuffTxFX[2],03bh        ;    00111011   SYNC
    mov BuffTxFX[3],028h        ;    00101000   SYNC

; Compone el prefijo y el indicativo fuente.

    mov al,MiPrefx              ;    00000001  (en caso de 01)
    or  al,080h                 ;or  10000000
                                ;    --------
    mov BuffTxFX[4],al          ;    10000001  - 81h


    mov ax,RxIndic              ;    00000001 (en caso de 01)
;    mov cl,04h
;    rol ax,cl
    and ax,0fff8h

    mov BuffTxFX[5],ah          ;rol 00001000
                                ;    --------
    mov BuffTxFX[6],al          ;    00001000  - 08h
    ret

IniCab ENDP

    END
