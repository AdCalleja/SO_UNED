OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

/* 3 bits de propiedades XWR*/
PHDRS
{
    text    PT_LOAD FLAGS((1 << 0) | (1 << 2)); /* Ejecutar y leer*/
    rodata  PT_LOAD FLAGS((1 << 2));            /* Leer*/
    data    PT_LOAD FLAGS((1 << 1) | (1 << 2)); /*Leer y Escribir*/
}

SECTIONS
{
    USER_START = 0x0000000000000000;
    USER_END = 0x00007fffffffffff;

    . = 0xffffffff80000000;
    KERNEL_START = .;

    .text : ALIGN(CONSTANT(MAXPAGESIZE)) {
        *(.text .text.*)
    } :text

    .data : ALIGN(CONSTANT(MAXPAGESIZE)){
        *(.data .data.*)
    } :data

    .bss : ALIGN(CONSTANT(MAXPAGESIZE)){
        *(COMMON)
        *(.bss .bss.*)
    } :data

    /DISCARD/ : {
        *(.note .note.*)
    }

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    KERNEL_END = .;
}