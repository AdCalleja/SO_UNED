#include "arch/simd.h"
#include "io/interrupts.h"
#include "devices/pit/pit.h"
#include "devices/keyboard/keyboard.h"
#include "devices/mouse/mouse.h"
#include "util/string.h"
#include "bootservices/bootservices.h"
#include "util/printf.h"
#include "memory/memory.h"
#include "memory/heap.h"
#include "memory/paging.h"
#include "arch/cpu.h"
#include "arch/gdt.h"
#include "debug/shell.h"

/* 
///// COMANDOS POR SI ACASO /////
sudo rm -rf /mnt/bloodmoon
sudo losetup -l
sudo losetup -d /dev/loop*

///// COMANDOS DE MAKE /////
make setup
make cleansetup
make gpt
*/

// No hacemos el cargador de arranque (Como sería GRUB UBOOT etc).
// Xabier tiene hecho el suyo con LIMINE y lo usamos directamente
// OVMFbin es la UEFI que usamos para simular la bios de los fabricantes
// QEMU carga la UEFI, la UEFI carga LIMINE, y LIMINE carga nuestro KERNEL. QEMU->UEFI->LIMINE->KERNEL
// Códgio de LIMINE en src/bootservices
void _start(void) {
    // Inicial
    // void (*writer)(const char*, uint64_t) = get_terminal_writer();
    // char string[] = "Hello World!\n";
    // writer(string, strlen(string));

    // INITS
    //init_simd();
    init_simd();
    init_memory();
    init_paging();
    init_heap();
    create_gdt();
    init_interrupts(0);
    init_keyboard();
    init_mouse();
    init_cpus();
    enable_interrupts();
    
    printf("Hey Mundo\n");

    char * buffer = (char*)malloc(1000);
    sprintf(buffer, "Hola Mundo %d\n", 5);
    printf("String %s\n", buffer);
    free(buffer);

    run_shell();
    
    printf("Máscara del PIC: %d\n", inb(PIC1_DATA));
    while(1){
        if(((inb(0x64) & 0x1))==1)
        printf("Interrupción de ratón detectada %d", inb(0x60));
    }
}