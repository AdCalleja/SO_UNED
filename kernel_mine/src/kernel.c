#include "util/string.h"
#include "bootservices/bootservices.h"
#include "util/printf.h"
#include "memory/memory.h"

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
    init_memory();
    //init_interrupts(1);

    printf("Hola Mundo\n");

    // Memory check
    int * buffer = (int*)request_page();

    for (int i = 0; i < 100; i++) {
        buffer[i] = i;
    }
    for (int i = 0; i < 100; i++) {
        printf("buffer[%d] at %p = %d\n", i, &(buffer[i]), buffer[i]);
    }

    // //Trigger a page fault
    // int a = 5;
    // int b = a / 0;
    // printf("The result is %d\n", b);



    while(1);
}