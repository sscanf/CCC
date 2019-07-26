;****************************************************************************
;* NOMBRE      : V25INI.ASM                                                 *
;* DESCRIPCION : Submodulo del programa RTX21CPU.ASM                        *
;* LENGUAJE    : MASM 6.0                                                   *
;* EDITOR      : PWB                                                        *
;* FECHA INICIO: 4/01/1995                                                  *
;* FECHA FINAL :                                                            *
;* OBSERV.     : Inicializa el V25                                          *
;****************************************************************************
include ConstRAC.inc

;PUBLIC RstTimer
;PUBLIC TimerOn
;PUBLIC TimerOff
;PUBLIC TMC0
;PUBLIC TMIC0
;PUBLIC V25ini
;PUBLIC PrepTimer
;PUBLIC espera

.CODE

V25ini proc

    cli
    mov ax,0fff0h
    mov es,ax

    mov byte ptr es:WTCL ,0ffh     ; es pot programar wait states
    mov byte ptr es:WTCH ,0ffh
    mov byte ptr es:RSHM ,0fch     ;refresh mode
    mov byte ptr es:PRC  ,0ch      ;clock ratio
                                   ;Divide el clock 1/2
                                   ;Interrupt TBC cada 2**20/fclock,disabled
    mov byte ptr es:SCM0 ,09h
    mov byte ptr es:SCC0 ,04h
    mov byte ptr es:BRG0 ,52
    mov byte ptr es:STIC0,07h
    mov byte ptr es:SRIC0,07h

    mov byte ptr es:PMC0 ,00h      ;Port 0 en port mode
    mov byte ptr es:PM0  ,0f0h     ;4 bits bajos salidas, 4 bits altos entradas.

    mov byte ptr es:PMC1 ,00h      ;Port 1
    mov byte ptr es:PM1  ,0ffh     ;/INTP0,/INTP1,/INTP2

    mov byte ptr es:PMT  ,00h      ;Programa el port T para entradas

    mov byte ptr es:ISPR ,00h

    mov byte ptr es:EXIC0,00h
    mov byte ptr es:EXIC1,07h
    mov byte ptr es:EXIC2,07h

    mov byte ptr es:TMC0 ,060h
    mov byte ptr es:TMC1 ,040h

    mov word ptr es:MD0  ,0f424h   ;Har  una interrupci¢n cada segundo

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

    mov si,INTP1
    mov ax,OFFSET HandIntP0
    mov es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax

    mov si,INTP2
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

    mov si,INTTU0
    mov ax,offset IntTIMER0    ;Rutina de interrupci¢n para el TIMER 0.
    mov word ptr es:[si],ax
    add si,2
    mov ax,cs
    mov es:[si],ax

    ret
V25ini endp

espera  PROC segs:BYTE

    invoke PrepTimer,segs  ;Prepara el timer

 .while (segundos)
 .endw

    ret

espera   ENDP


PrepTimer PROC segs:BYTE

;Prepara el timer para que empiece a contar a partir de que se llama a esta
;rutina poniendo como parametro los segundos a contar.
;

    mov ah,segs
    mov segundos,ah
    call RstTimer   ;Resetea el timer para que empiece a contar desde ahora.
    call TimerOn    ;Pone en marcha el TIMER
    ret
PrepTimer  ENDP

RstTimer PROC uses es

    mov ax,0fff0h
    mov es,ax

    mov byte ptr es:TMC0 ,0c0h
    mov byte ptr es:TMC0 ,0e0h      ;Resetea timer.
    ret
RstTimer ENDP


TimerOff PROC USES ax es

    mov ax,0fff0h
    mov es,ax
    mov byte ptr es:TMC0 ,060h
    ret

TimerOff ENDP


TimerOn PROC USES ax es

    mov ax,0fff0h
    mov es,ax
    mov byte ptr es:TMC0 ,0c0h
    mov byte ptr es:TMC1 ,040h
    mov byte ptr es:TMIC0,07h      ;Activa interrupciones TIMER0

    ret

TimerOn ENDP
END
