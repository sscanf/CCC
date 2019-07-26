;****************************************************************************
;* NOMBRE      : CONSOLA.ASM                                                *
;* DESCRIPCION : Submodulo del programa RTX21CPU.ASM (rutinas consola)      *
;* LENGUAJE    : MASM 6.0                                                   *
;* EDITOR      : PWB                                                        *
;* FECHA INICIO: 4/01/1995                                                  *
;* FECHA FINAL :                                                            *
;* OBSERV.     : Gestiona la consola                                        *
;****************************************************************************

INCLUDE constRAC.INC

 ;PUBLIC TxOnRs
 ;PUBLIC TxOffRs
 ;PUBLIC TxBufferRs
 ;PUBLIC gets
 ;PUBLIC getc
 ;PUBLIC getch
 ;PUBLIC TxFichero
 ;PUBLIC IniTxFich


 TxFichero  PROTO NumFich : WORD
 TxByteRs   PROTO scr     : BYTE
 TxBufferRs PROTO Lbytes  : word,
                  despla  : dword



.CODE

;------------------ TRANSMITE UN BYTE A LA CONSOLA  -----------------------
;
;En scr est  el byte a transmitir.

TxByteRs PROC scr : BYTE

    mov al,scr

    mov BuffTxRs,al
    lea bx,BuffTxRs
    invoke TxBufferRs,1,ds::bx
    ret
TxByteRs ENDP


;------------------------- ACTIVA RECEPCION SERIE ---------------------------
RxOnRs  PROC USES ax es

    mov KeybHead,0
    mov KeybTail,0
    mov ax,0fff0h
    mov es,ax

    mov al,byte ptr es:SCM0
    or al,40h
    mov byte ptr es:SCM0,al ;Activa recepci¢n

    ret
RxOnRs  ENDP

;------------------------- DESACTIVA RECEPCION SERIE --------------------------
RxOffRs PROC uses ax es

    mov ax,0fff0h
    mov es,ax

    mov al,byte ptr es:SCM0
    and al,0bfh
    mov byte ptr es:SCM0,al ;Desactiva recepci¢n

    ret
RxOffRs ENDP

;------------------------- ACTIVA TRANSMISION SERIE --------------------------
TxOnRs PROC uses ax es

    mov ax,0fff0h
    mov es,ax

    mov al,byte ptr es:SCM0
    or al,80h
    mov es:SCM0,al ;Activa transmisi¢n

    ret
TxOnRs ENDP

;------------------- DESACTIVA TRANSMISION SERIE -----------------------

TxOffRs proc uses ax es

    mov ax,0fff0h
    mov es,ax

    mov al,byte ptr es:SCM0
    and al,7fh
    mov byte ptr es:SCM0,al ;Desactiva transmisi¢n

    ret
TxOffRs endp


;------------------ TRANSMITE BUFFER A LA CONSOLA -------------------------

TxBufferRs proc USES dx es ax bx di,
                Lbytes : word,  ;Largo de bytes a transmitir.
                despla : dword  ;desplazamiento y segmento del buffer a transmitir.

     mov di,0
     mov FiTx,0

     mov bx,word ptr despla[0]
     mov dx,word ptr despla[2]

TxBuf1:
     and flag,0fdh
     mov es,dx
     mov al,es:[bx+di]

;     .if (al<30h)
;        add al,30h
;     .endif

     push ax
     mov ax,0fff0h
     mov es,ax
     pop ax
     mov byte ptr es:TXB0,al


TxBuf:
     mov ah,flag
     and ah,02h
     cmp ah,02
     jnz TxBuf

     inc di
     cmp di,Lbytes
     jb TxBuf1
     ret

TxBufferRs endp


;---------------------- ENTRADA CADENA SERIE -------------------------------
;En cx, ha de estar el n£mero de car cteres a entrar.
;En ds:di ha de estar la direcci¢n del buffer a poner el texto entrado.

gets   PROC uses es di ax

      push ds
      pop es

get1:
    call getch

;    .if (al<30h) || (al>39h)   ;S¢lo acepta n£meros en formato ascii
;      xor ah,ah                ;comparando cada entrada si es menor que 30h
;      invoke TxMsg,ax          ;o mayor que 39h
;      jmp get1
;    .endif

      mov es:[di],al
      inc di
      loop get1
      ret

gets   ENDP



;------------- PREPARA LA CONSOLA PARA RECIBIR UN FICHERO -------------------

IniTxFich PROC

    invoke TxByteRs,P_SER ;Indica al display que genere SERVEI
ra:
    call getch       ;Espera que el terminal de devuelva un 11h(XON)
    cmp al,X_ON      ;para poder transmitir el fichero al terminal.
    jnz ra
    ret

IniTxFich ENDP


;--------- TRANSMITE A LA CONSOLA EL NUMERO DE FICHERO INDICADO EN AX ------
;
    T_NumExpe   db 'Num.Expedient:'
    T_NumServ   db 'Servei       :'
    T_client    db 'Client       :'
    T_SubCli    db 'Subclient    :'
    T_nom       db 'Nom i cognom :'
    T_ident     db 'Identificador:'
    T_matric    db 'Matricula    :'
    T_marca     db 'Marca/Color  :'
    T_ubicacio  db 'Ubicacio vehi:'
    T_claus     db 'Claus        :'
    T_contact   db 'Persona cont.:'
    T_situacio  db 'Situacio vehi:'
    T_desti     db 'Desti vehicle:'
    T_codi      db 'Codi producte:'
    T_cost      db 'Cost previst :'


TxFichero PROC  NumFich  : word
          LOCAL posic    : word,
                positit  : word,
                posibyte : word


    mov cx,32
    mov di,0
lp:
    mov ah,' '
    mov ArrayTemp[di],ah
    inc di
    loop lp

    mov ax,NumFich
    dec ax

    mov bx,offset BuFich
    mov posic,bx

    mov bx,431
    mul bx
    add posic,ax ;En posic est  la direcci¢n del fichero.

    mov ax,offset T_NumExpe
    mov positit,ax

    mov ax,offset nBytesASI
    mov posibyte,ax

buc:

    mov cx,14
    mov bx,positit
    invoke TxBufferRs,14,cs::bx  ;Transmite el titulo

    mov di,posibyte
    xor cx,cx
    mov cl,[di]
    dec cl
    mov bx,posic
    invoke TxBufferRs,cx,ds::bx  ;Transmite el contenido del campo

    mov dx,0
    mov ax,cx
    add ax,14
    mov cx,32
    div cx
    sub cx,dx

    lea bx,ArrayTemp
    invoke TxBufferRs,cx,ds::bx  ;Acaba la linea con espacios

    xor cx,cx
    mov di,posibyte
    mov cl,[di]
    add posic,cx
    add positit,14
    inc posibyte
    mov di,posibyte
    mov cl,[di]
    cmp cl,0
    jnz buc

    ret
TxFichero ENDP
;---------------------------- GETCH ----------------------------------------
;Espera un car cter del terminal y lo devuelve en AL
;

getch PROC USES bx di

gec:
    mov ax,KeybTail
    cmp KeybHead,ax
    jz  gec             ;Espera a que entren un car cter

    mov di,KeybHead
    mov al,BuffRxRs[di]
    inc KeybHead       ;Saca el car cter del buffer
    cmp KeybHead,MAX_MSG_RS
    jnz figec
    mov KeybHead,0

figec:
    ret
getch ENDP

;---------------------------- GETC ----------------------------------------

;Devuelve el estado del buffer del teclado.
;Regresa con NZ si no est  vac¡o.
;Regresa con ZF si est  vac¡o.
;No espera pulsaci¢n.
;
;En caso de buffer no vac¡o AL = car cter entrado.

getc PROC uses bx

    mov ax,KeybHead
    cmp KeybTail,ax
    pushf
    jz  figec

    mov di,KeybHead
    mov al,BuffRxRs[di]
    inc KeybHead       ;Saca el car cter del buffer
    cmp KeybHead,MAX_MSG_RS
    jnz figec
    mov KeybHead,0

figec:
    popf
    ret
getc ENDP

END
