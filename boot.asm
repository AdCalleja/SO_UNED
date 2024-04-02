[ORG 0x7c00]	; Zona de memoria real desde la que queremos arrancar
[BITS 16]	; Especifica la arquitectura

mov [MAIN_DISK], dl ; Guardar en MAIN_DISK la dirección del main disk

; Mi código de aranque
mov bp, 0x1000	; bp: base pointer
mov sp, bp	; sp: stack pointer. Iguales al inicio

; Llamada a BIOS para imprimir por TTY
;mov ah, 0x0e	; Configuración para BIOS
;mov al, 'M'	; Mensaje a enviar
;int 0x10	; Llama a la interrupción 10

;mov al, 'H'

mov bx, STRING
call print_string

; Configurar la lectura del disco
mov dl, [MAIN_DISK]
mov ah, 0x02 ; Operacion de lectura
mov al, 0x01 ; Nº de sectores a leer
mov ch, 0x00 ; Cilindor
mov dh, 0x00 ; Cabezal
mov cl, 0x02 ; Sector
mov bx, 0x8000; DIRECCION A LA QUE ESCRIBIMOS. De la dirección 0x8000 hacia abajo está la BIOS


; Acceso al disco usando la bios
int 0x13
; Configure Keyboard
mov ax, keyboardCommandHandler
call install_keyboard
;call install_mouse
call second_stage


jmp $		; while(1) $ == dirección altual



;;;;;;;;;;; TERMINAL ;;;;;;;;;;;
print_string:
	pusha
	xor si, si
.loop:
	mov al, byte [bx, si]
	inc si
	cmp al, 0
	je .end
	call print_char
	jmp .loop
.end:
	popa
	ret

print_char:
	push ax
	mov ah, 0x0e
	int 0x10
	pop ax
	ret

;;;;;;;;;;; KEYBOARD ;;;;;;;;;;;
;Recibe por ax la handler address
install_keyboard:
	push word 0
	pop ds
	cli ; Clear interrupts (las desactiva) para evitar que una interrupción joda todo durante la configuración del teclado

	; Instalar el ISR del teclado
	mov [0 + 4 * KEYBOARD_INTERRUPT], word keyboardHandler ; Tabla IVT empieza en 0
	mov [0 + 4  * KEYBOARD_INTERRUPT + 2], cs ;  Segmento de codigo de la tabla IVT
	mov word [HANDLER], ax	; ?
	sti ; Re-enable interrupts
	ret


keyboardHandler:
    pusha ; Guardar todos los registros
	; PS2 interfacing I/O Programada
	; I/O 2 types: mapeado en memoria o programada (registros o puertos In and Out)
    in al, 0x60
    test al, 0x80
    jnz .end
	; Hacer el cambio de scandcode de ASCII, a través del keymap.inc 
    mov bl, al
    xor bh, bh ; Asegurarse de que bh es 0
    mov al, [cs:bx + keymap]	; al = offset + keymap_base / Desde 
    cmp al, 13
    je .enter
    mov bl, [WORD_SIZE]
    mov [WRD+bx], al
    inc bx
    mov [WORD_SIZE], bl
    mov ah, 0x0e
    int 0x10
.end:
    mov al, 0x61
    out 0x20, al
    popa
    iret ; Interrupt return. Recupera los stacks de fuera d ela interrupción
.enter:
    mov bx, WRD
    mov cl, [WORD_SIZE]
    mov dx, [HANDLER]
    call dx
    mov byte [WORD_SIZE], 0
    jmp .end


keyboardCommandHandler:
    mov al, [bx]
    cmp al, 'h'
    je .hola
    cmp al, 'a'
    je .adios
    mov bx, INVALID
    call print_string
    ret
.hola:
    mov bx, STRING
    call print_string
    ret
.adios:
    mov bx, STRONG
    call print_string
    ret

;;;;;;;;;;; MOUSE ;;;;;;;;;;;
install_mouse:
	push word 0
	pop ds
	cli ; Clear interrupts (las desactiva) para evitar que una interrupción joda todo durante la configuración del teclado

	; Instalar el ISR del teclado
	mov [0 + 4 * MOUSE_INTERRUPT], word mouseHandler ; Tabla IVT empieza en 0
	mov [0 + 4  * MOUSE_INTERRUPT + 2], cs ;  Segmento de codigo de la tabla IVT
	mov word [HANDLER], ax	; ?
	sti ; Re-enable interrupts
	ret

mouseHandler:
    pusha
    mov bx, MOSE
    call print_string
    popa
    iret


WORD_SIZE: db 0
WRD: times 64 db 0 ; Buffer para almacenar la palabra
STRING: db "Hola Mundo", 0
STRONG: db "Adios Mundo", 0
MOSE: db "Mouse handler", 0
INVALID: db "Comando invalido", 0
MAIN_DISK: db 0 ; Label, referencia memoria Estamos reservando un byte, pero no tiene valor, no lo estamos guardando
IVTR EQU 0	; Registro que tiene la posición base de la IVT
KEYBOARD_INTERRUPT EQU 9
MOUSE_INTERRUPT EQU 74
HANDLER: dw 0

keymap:
%include "keymap.inc"

times 510-($-$$) db 0	; times: directiva de compilación que repite el commando. $$: Inicio del prorama

dw 0xaa55

second_stage:
	; mov ah, 0x0e
	; mov al, 'H'
	; int 0x10
	jmp $




; Necesario el padding hasta 512 bytes para cargar el sector entero
times 1024-($-$$) db 0

