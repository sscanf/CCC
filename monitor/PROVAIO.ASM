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


.DATA
.STACK [256]

.CODE
.STARTUP

main:
    mov dx,0f000h
    mov ax,0aaaah
    out dx,ax

    jmp main

 END
