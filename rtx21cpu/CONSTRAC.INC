.MODEL small,pascal

;RUTINAS PUBLICAS

EXTERNDEF TxMsg     :PROTO NumMsg  :WORD
EXTERNDEF RxMsg     :PROTO numMsg  :WORD
EXTERNDEF TxFichero :PROTO NumFich :WORD
EXTERNDEF TxByteRs  :PROTO scr     :BYTE
EXTERNDEF TxBufferRs:PROTO Lbytes  :WORD, despla  : DWORD
EXTERNDEF delay     :PROTO tiempo  :WORD
EXTERNDEF espera    :PROTO segs    :BYTE
EXTERNDEF PrepTimer :PROTO segs    :BYTE

EXTERNDEF V25ini    :near
EXTERNDEF RstTimer  :near
EXTERNDEF TimerOn   :near
EXTERNDEF TimerOff  :near
EXTERNDEF TxRQQ     :near
EXTERNDEF TxOffFx   :near
EXTERNDEF TxOnFx    :near
EXTERNDEF RxOffFx   :near
EXTERNDEF RxOnFx    :near
EXTERNDEF ModemOff  :near
EXTERNDEF ModemOn   :near
EXTERNDEF MicroOn   :near
EXTERNDEF MicroOff  :near
EXTERNDEF ModemRst  :near
EXTERNDEF TxOnRs    :near
EXTERNDEF TxOffRs   :near
EXTERNDEF RxOnRs    :near
EXTERNDEF getc      :near
EXTERNDEF getch     :near
EXTERNDEF gets      :near
EXTERNDEF PttIn     :near
EXTERNDEF IniTxFich :near
EXTERNDEF AudioOn   :near
EXTERNDEF AudioOff  :near
EXTERNDEF GenTest   :near
EXTERNDEF PttOff    :near
EXTERNDEF TxMsgFX   :near
EXTERNDEF HandIntP0 :near
EXTERNDEF IntNMI    :near
EXTERNDEF RSITx     :near
EXTERNDEF RSIRx     :near
EXTERNDEF IntTIMER0 :near


;VARIABLES PUBLICAS

EXTERNDEF segundos  :byte
EXTERNDEF pFichero  :word
EXTERNDEF BuffTxFX  :byte
EXTERNDEF MiIndic   :word
EXTERNDEF TxMsgLn   :word
EXTERNDEF MiPrefx   :byte
EXTERNDEF RxIndic   :word
EXTERNDEF PenTx     :word
EXTERNDEF NumMecan  :byte
EXTERNDEF slot1     :byte
EXTERNDEF EspRx     :word
EXTERNDEF intentos  :byte
EXTERNDEF MsgFlg    :word
EXTERNDEF NumBlock  :byte
EXTERNDEF BuFich    :byte
EXTERNDEF ScrMode   :byte
EXTERNDEF nBytesASI :byte
EXTERNDEF nBytesMOD :byte
EXTERNDEF TotalFich :byte
EXTERNDEF FichMod   :byte
EXTERNDEF flag      :byte
EXTERNDEF PosFicher :byte
EXTERNDEF NSerie    :word
EXTERNDEF MdmCtrl   :byte
EXTERNDEF RxMsgL    :word
EXTERNDEF RxMsgLn   :word
EXTERNDEF TxMsgL    :word
EXTERNDEF RadioCtrl :byte
EXTERNDEF MdmStat   :byte
EXTERNDEF CntBytes2 :word
EXTERNDEF CntBytes  :byte
EXTERNDEF BuffTxRs  :byte
EXTERNDEF KeybHead  :word
EXTERNDEF KeybTail  :word
EXTERNDEF FiTx      :byte
EXTERNDEF BuffRxRs  :byte
EXTERNDEF ArrayTemp :byte
EXTERNDEF BuffRxFx  :byte
EXTERNDEF BuffTxFX  :byte
EXTERNDEF StatCRC   :byte
EXTERNDEF random    :byte


 ;DEBUGER

 TRUE       equ 0ffffh
 FALSE      equ 0h

 RELEASE    equ TRUE

 ;TIEMPOS

 TIEMPO_MAX equ 03h    ;Tiempo m ximo de espera entre intentos de comunicaci¢n


 ;COMANDOS RS-232

 X_ON       equ 11h
 X_OFF      equ 13h
 EndChar    equ 0
 EOF        equ 1ah  ;End Off File

 ;DISPLAY->V25

 T_MEC      equ 01h
 T_LLI      equ 02h
 T_DES      equ 08h
 T_OCU      equ 10h
 T_FIN      equ 12h
 T_VEU      equ 06h
 T_EME      equ 07h
 T_SI       equ 0dh
 T_NO       equ 0eh
 T_TAN      equ 0fh
 T_PAR      equ 14h

 C_FICHSIG  equ 03h
 C_FICHANT  equ 05h


 ;V25->DISPLAY

 P_MEC      equ 01h
 P_LLI      equ 02h
 P_SER      equ 04h
 P_DES      equ 08h
 P_OCU      equ 10h
 P_ATU      equ 0ah
 P_ANU      equ 0bh
 P_FIVEU    equ 09h


 ;MENSAJES PROTOCOLO RACC

 DES        equ 01h
 OCU        equ 02h
 PAR        equ 03h
 FIN        equ 04h
 OBE        equ 05h
 EME        equ 06h
 VEU        equ 07h
 TAN        equ 08h
 MEC        equ 09h
 LLI        equ 10h


 ;CONSTANTES PARA FX429

 RXDATAFX   equ 06h
 TXDATAFX   equ 02h
 CONTROLFX  equ 03h
 STATUSFX   equ 07h
 INDI_CENT  equ 0ffffh

 TX_OFF      equ 0feh
 TX_ON       equ 01h
 RX_OFF      equ 0fbh
 RX_ON       equ 04h
 RX_FORMAT_0 equ 0f7h
 RX_FORMAT_1 equ 08h


 ;CONTROL EMISORA

 MODEM_OFF  equ 00000100b
 MODEM_ON   equ 11111011b
 MICRO_OFF  equ 00001000b
 MICRO_ON   equ 11110111b
 PTT_ON     equ 11111011b
 PTT_OFF    equ 00000100b
 AUDIO_ON   equ 11111110b
 AUDIO_OFF  equ 00000001b



 ;CONSTANTES V25

 P0         equ 000h    ;Port 0 datos
 PM0        equ 001h    ;Port 0 Mode Register
 PMC0       equ 002h    ;Port 0 Mode Control Register
 PM1        equ 009h    ;Port 1 Mode Register
 PMC1       equ 00ah    ;Port 1 Mode Control Register
 PM1        equ 009h    ;Port 1 Mode Register
 PMC1       equ 00ah    ;Port 1 Mode Control Register
 PT         equ 038h    ;Port T datos
 PMT        equ 03bh    ;Port T Mode Register
 WTCL       equ 0e8h    ;Wait Control Register Hight
 WTCH       equ 0e9h    ;Wait Control Register Low
 RSHM       equ 0e1h    ;Refresh mode Register
 PRC        equ 0ebh    ;Processor Control Register.
 TM0        equ 080h    ;Timer Register 0
 MD0        equ 082h    ;Modulo/Time Register 0
 TMC0       equ 090h    ;Timer Control Register 0
 TMC1       equ 092h    ;Timer Control Register 1


 SCM0       equ 068h    ;Serial Mode Register canal 0
 SCC0       equ 069h    ;Serial Control Register canal 0
 BRG0       equ 06ah    ;Baud Rate Generator Register canal 0
 SRIC0      equ 06dh    ;Serial Receive Interrupt Request Control Register canal 0
 STIC0      equ 06eh    ;Serial Transmit Interrupt Request Control Register canal 0
 RXB0       equ 060h    ;Receive Buffer Register canal 0
 TXB0       equ 062h    ;Transmit Buffer Register canal 0

 EXIC0      equ 4Ch     ;EXTERNDEFal Interrupt Macro Service Control Register 0.
 EXIC1      equ 4Dh     ;EXTERNDEFal Interrupt Macro Service Control Register 0.
 EXIC2      equ 4Eh     ;EXTERNDEFal Interrupt Macro Service Control Register 0.
 NMI        equ 08h
 ISPR       equ 0fch    ;Interrupt Priority Register

 INTP0       equ 60h
 INTP1       equ 64h
 INTP2       equ 68h
 INTSR0      equ 34h    ;Vector interrupci¢n rutina RX
 INTST0      equ 38h    ;Vector interrupci¢n rutina TX
 TMIC0       equ 9ch
 INTTU0      equ 70h
 BREAK       equ 03h
 MAX_MSG_RS  equ 16
 STACK_SIZE  equ 256

 ;ESTRUCTURAS

 fichero STRUCT
    NumExpe  byte 008 dup (0)
    NumServ  byte 003 dup (0)
    client   byte 005 dup (0)
    SubCli   byte 005 dup (0)
    nom      byte 051 dup (0)
    ident    byte 016 dup (0)
    matric   byte 011 dup (0)
    marca    byte 014 dup (0)
    ubicacio byte 005 dup (0)
    claus    byte 041 dup (0)
    contact  byte 031 dup (0)
    situacio byte 155 dup (0)
    desti    byte 075 dup (0)
    codi     byte 004 dup (0)
    cost     byte 007 dup (0)
fichero ENDS

ficher_mod STRUCT
    M_NumExpe  byte 008 dup (0)
    M_NumServ  byte 003 dup (0)
    M_client   byte 005 dup (0)
    M_SubCli   byte 005 dup (0)
    M_codi     byte 004 dup (0)
    M_cost     byte 007 dup (0)
ficher_mod ENDS
