;****************************************************************************
;* NOMBRE      : RTX21CPU.ASM                                               *
;* DESCRIPCION : Modulo principal RTX21-CPU                                 *
;* LENGUAJE    : MASM 6.0                                                   *
;* EDITOR      : PWB                                                        *
;* FECHA INICIO: 4/01/1995                                                  *
;* FECHA FINAL :                                                            *
;* OBSERV.     : Para carga externa LOCATE /h rtx21cpu,rtx21cpu,07000,0800  *
;*             : Para EPROM LOCATE -b -h rtx21cpu (Locate antiguo)          *
;****************************************************************************

;Definici¢n del byte FLAG:
;
; Bit     Descripci¢n
;
;  0   -   PTT 1- enable / 0- disable
;  1   -   Ha transmitido un byte serie.
;  2   -   Indica que ha recibido todos los slots.
;  3   -   El mensaje que ha recibido es de grupo.
;  4   -   Est  en modo TEST
;  5   -   0 - Mira SQL, alarga tiempo de PTT / 1 - No mira SQL y PTT corto.
;  6   -
;  7   -
;
;

;Definici¢n del byte RadioCtrl:

; Bit     Descripci¢n
;
;  0   -   No usado
;  1   -   No usado
;  2   -   R1/ MICRO - 0 = On / 1 = Off
;  3   -   R2/ MODEM - 0 = On / 1 = Off
;  4   -   R3/ SQL
;  5   -   R4/ PTT
;  6   -   No usado
;  7   -   No usado
;
;
;
;
;Definici¢n del byte MsgFlg:
;
; Bit     Descripci¢n
;
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
;  10  -
;  12  -
;  13  -
;  14  -
;  15  -
;
;

 include constRAC.inc

.STACK [STACK_SIZE]
;=============================== RAM ========================================

.FARDATA

    prefx   byte ?  ; Prefijo movil (No volatil)
    indic   dw   ?  ; Indicativo movil (No volatil)

.DATA

    BuffTxFX  byte 016 dup (0)  ;1 HEAD con SYNCT y 1 slot
    BuffRxFx  byte 006 dup (0)  ;1 HEAD
    slot1     byte 006 dup (0)  ;SLOT 1
    slot2     byte 006 dup (0)  ;SLOT 2
    slot3     byte 006 dup (0)  ;SLOT 3
    slot4     byte 006 dup (0)  ;SLOT 4
    slot5     byte 006 dup (0)  ;SLOT 5
    slot6     byte 006 dup (0)  ;SLOT 6
    slot7     byte 006 dup (0)  ;SLOT 7
    slot8     byte 008 dup (0)  ;SLOT 8

    ArrayTemp byte 032 dup (0)  ;Array temporal

    BuffRxRs  byte MAX_MSG_RS  dup (0)
    BuffTxRs  byte 60h dup (0)
    NumMecan  byte 003 dup (0)   ;Aqu¡ guarda el n£mero de mec nico en ASCII.

    KeybHead  word 0
    KeybTail  word 0
    MdmStat   byte 0  ;Memoriza el estado del modem.
    MdmCtrl   byte 0  ;Memoriza la palabra de control del modem.
    RadioCtrl byte 0  ;Memoriza el se¤ales de control de la radio.
    flag      byte 0
    TxMsgL    word 0  ;Puntero del buffer de transmisi¢n MODEM.
    TxMsgLn   word 0
    RxMsgL    word 0
    RxMsgLn   word 0
    RxIndic   word 0
    FiTx      byte 0
    MsgFlg    word 0  ;Indica el tipo de memsaje recibido.
    segundos  byte 0
    EspRx     word 0
    PenTx     word 0
    intentos  byte 0  ;N£mero de intentos para transmitir un mensaje.
    StatCRC   byte 0
    CntBytes  byte 0
    CntBytes2 word 0
    NumBlock  byte 0
    ScrMode   byte 0  ;Modo en que est  el terminal
    PosFicher byte 0  ;N£mero de fichero a transmitir al terminal.
    TotalFich byte 0  ;N£mero de fichero a transmitir al terminal.
    pFichero  word 0  ;Puntero a fichero
    MiIndic   word 0  ;Prefijo movil
    MiPrefx   byte 0  ;Indicativo movil
    random    byte 0

    nBytesASI byte 16 dup (0)
    nBytesMOD byte 07 dup (0)

    FichMod   ficher_mod {} ;Reserva espacio para el fichero de mofificaci¢n

    BuFich    label byte

;=============================== ROM ========================================

.CODE

    byte   'RTX21- CPU '
    byte   'CopyRight (c) Electr¢nica Barcelona S.L '
    byte   'febrero 1995'

.STARTUP

    mov ax,STACK
    mov ss,ax
    mov ax,STACK_SIZE
    mov sp,ax

    cli
    CALL V25ini     ;Inicializaci¢n CPU
    sti

    call AudioOff   ;Desactiva audio RX.
    call ModemOff   ;Desactiva el modem
    call MicroOff   ;Desactiva el micro.
    call ModemRst   ;Resetea el modem

; PARA ENTRAR EN EL MONITOR AL HACER UN RESET SE HA DE PONER EL BIT 7 DEL
; PORT 'T' A +5V.

  if RELEASE

    mov ax,0fff0h
    mov es,ax
    mov al,byte ptr es:PT
    and al,80h
    cmp al,80h
    jnz  begin

    ;PARA ENTRAR EN MODO TEST, AL HACER UN RESET SE HA DE PULSAR EL PTT

    mov al,byte ptr es:P0
    and al,20h
    cmp al,0h
    jz  mon

    call GenTest

mon:
    mov ax,0f000h
    push ax
    mov ax,0
    push ax
    retf

  endif

begin:

    call RxOnFx     ;Pone el modem en recepci¢n
    call RxOnRs     ;Activa recepci¢n serie.
    call TxOnRs     ;Activa transmisi¢n serie.
    call PttOff

    mov flag     ,0
    mov MdmCtrl  ,0
    mov CntBytes ,0
    mov CntBytes2,0
    mov NumBlock ,1
    mov PosFicher,1
    mov TotalFich,0
    mov ScrMode  ,0
    mov EspRx    ,0

    ;Pasa el indicativo y el prefijo que est n en FARDATA a NEARDATA

    mov ax,70h
    mov es,ax
    mov di,0

    mov al,byte ptr es:[di]
    mov MiPrefx,al
    inc di
    mov ax,word ptr es:[di]
    mov MiIndic,ax

    mov pFichero ,offset BuFich   ;Apunta al primer fichero a poner en memoria.

    mov di,offset RomASI
    mov si,offset nBytesASI
    mov cx,23

tras:
    mov al,cs:[di]
    mov ds:[si],al
    inc di
    inc si
    loop tras

    invoke espera,2    ;Antes de nada espera 2 segundos para que el display
                       ;se inicialize bien.
beg:
    mov PenTx,0
    mov NumMecan[0],0
    invoke TxByteRs,P_MEC ;Pone el display en MECANIC.

reint:

    invoke PrepTimer,TIEMPO_MAX   ;Esperar  el tiempo indicado en TIEMPO_MAX

main1:

    mov MsgFlg,0    ;Resetea el flag de mensajes recibidos.

main:

 .while (segundos>0)

   .if EspRx==0      ;Solo mirar  si hay una tecla pendiente cuando haya acabado
     call getc       ;de realizar la tarea.
     jnz teclado     ;NZ = 1 hay pulsaci¢n.
   .endif

    call PttIn      ;pulsan PTT?

   .if MsgFlg
    jmp DoRxMsg     ;Ha recibido un mensaje
   .endif

 .endw
    mov EspRx,0
    cmp PenTx,0     ;Si no tiene ning£n mensaje pendiente, regresa a main
    jz  reint

 .if (intentos>0)

    invoke TxMsg,PenTx ;Gestiona el mensaje pendiente.
    dec intentos
    jmp reint

 .endif

  .if (PenTx==P_MEC)
    jmp beg
  .endif

    mov PenTx,0
    jmp reint

DoRxMsg:

    mov al,flag
    and al,08h

    ;No est  esperando alg£n mensaje y el mensaje que ha recibido
    ;es de grupo, gestiona el mensaje.

    .if (EspRx==0) && (al==00h)
      invoke RxMsg,MsgFlg ;gestiono el mensaje recibido.
      jmp reint
    .endif

    mov ax,MsgFlg
    and ax,EspRx
    cmp ax,EspRx    ;El mensaje recibido no es el que espera, lo rechaza.
    jnz reint
    invoke RxMsg,MsgFlg     ;Gestiona el mensaje recibido.

;    mov PenTx,0     ;Ya no espera ning£n mensaje.
;    mov EspRx,0     ;Ya no espera ning£n mensaje.

    jmp reint


teclado:
                    ;AL = Tecla pulsada
    mov intentos,5

 .if al==C_FICHANT

    .if (PosFicher==1)
      invoke TxByteRs,X_OFF     ;Transmite un error
      jmp main1
    .endif

    call IniTxFich             ;Prepara la consola para que reciba un fichero
    dec PosFicher
    invoke TxFichero,PosFicher
    jmp main1
 .endif

 .if al==C_FICHSIG

    mov al,ToTalFich

    .if (PosFicher==al)
      invoke TxByteRs,X_OFF     ;Transmite un error
      jmp main1
    .endif

    call IniTxFich             ;Prepara la consola para que reciba un fichero
    inc PosFicher
    invoke TxFichero,PosFicher
    jmp main1
 .endif

    xor ah,ah
    invoke TxMsg,ax
    jmp reint


;============================ ROM DATA ====================================

RomASI byte 8,3,5,5,51,16,11,14,5,41,31,155,75,4,7,0
RomMOD byte 8,3,5,5,4,7,0

       byte 'NUMERO DE SERIE:'
NSerie word 65535

END
