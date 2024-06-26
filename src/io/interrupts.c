#include "interrupts.h"
#include "idt.h"
#include "io.h"

#include "../arch/cpu.h"
#include "../arch/gdt.h"
#include "../arch/tss.h"
#include "../arch/getcpuid.h"
#include "../memory/memory.h"
#include "../memory/heap.h"
#include "../memory/paging.h"
#include "../util/string.h"
#include "../util/printf.h"
#include "../util/panic.h"
#include "../devices/keyboard/keyboard.h"
#include "../devices/mouse/mouse.h"

#define __UNDEFINED_HANDLER  __asm__ ("cli"); (void)frame; panic("Undefined interrupt handler");

extern void* interrupt_vector[IDT_ENTRY_COUNT];

uint8_t interrupts_ready = 0;
struct idtr idtr;
volatile int dynamic_interrupt = -1;

void (*dynamic_interrupt_handlers[256])(struct cpu_context* ctx, uint8_t cpuid) = {0};

// Final de transación con el PIC, si el que había enviado la interrupción era un esclavo (irq > 12) hay que desactivar también el esclavo
// Outb: escribe a PIC1_command (0x20) el EOI (0x20)
void pic_eoi(unsigned char irq) {
    if (irq >= 12) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_end_master() {
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_end_slave() {
    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void remap_pic() {
    uint8_t a1, a2;

    a1 = inb(PIC1_DATA);
    io_wait();  //Note the presence of io_wait() calls, on older machines its necessary to give the PIC some time to react to commands as they might not be processed quickly
    a2 = inb(PIC2_DATA);
    io_wait();

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // 0x10 | 0x01 = 0x11 Starts the initialization sequence (in cascade mode)
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, 0x20);  //ICW2: Master PIC vector offset = 0x20
    io_wait();
    outb(PIC2_DATA, 0x28);  // PIC2 a 0x28
    io_wait();

    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);  // restore saved masks.
    io_wait();
    outb(PIC2_DATA, a2);
    io_wait();
    printf("Máscara del PIC: %d\n", a1);
}

void PageFault_Handler(struct cpu_context* ctx, uint8_t cpuid) {
    (void)ctx;
    (void)cpuid;
    uint64_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));
    printf("Page Fault Address: %x\n", faulting_address);
    panic("Page fault\n");
}

void DoubleFault_Handler(struct cpu_context* ctx, uint8_t cpuid) {
    (void)ctx;
    (void)cpuid;
    panic("Double fault\n");
}

void GPFault_Handler(struct cpu_context* ctx, uint8_t cpuid) {
    (void)ctx;
    (void)cpuid;
    panic("General protection fault\n");
}

void KeyboardInt_Handler(struct cpu_context* ctx, uint8_t cpuid) {
    (void)ctx;
    (void)cpuid;
    uint8_t scancode = inb(0x60);
    handle_keyboard(scancode);
    pic_end_master();
}

void MouseInt_Handler(struct cpu_context* ctx, uint8_t cpuid) {
    (void)ctx;
    (void)cpuid;
    printf("Inside of MouseInt Handler");
    uint8_t scancode = inb(0x60);
    handle_mouse(scancode);
    pic_end_slave();
}

static void interrupt_exception_handler(struct cpu_context* ctx, uint8_t cpu_id) {
    printf("GENERIC EXCEPTION %d ON CPU %d\n", ctx->interrupt_number, cpu_id);
    panic("Exception\n");
}

struct idtdescentry * get_idt_gate(uint8_t entry_offset) {
    return (struct idtdescentry*)(idtr.offset + (entry_offset * sizeof(struct idtdescentry)));
}

void set_idt_gate(uint64_t handler, uint8_t entry_offset, uint8_t type_attr, uint8_t ist, uint16_t selector) {
    struct idtdescentry* interrupt = (struct idtdescentry*)(idtr.offset + (entry_offset * sizeof(struct idtdescentry)));
    set_offset(interrupt, handler);
    interrupt->type_attr.raw = type_attr;
    interrupt->selector = selector;
    interrupt->ist = ist;
}

void hook_interrupt(uint8_t interrupt, void* handler) {
    if (!interrupts_ready) panic("Interrupts not ready\n");
    __asm__("cli");
    dynamic_interrupt_handlers[interrupt] = handler;
    __asm__("sti");
}

void unhook_interrupt(uint8_t interrupt) {
    if (!interrupts_ready) panic("Interrupts not ready\n");
    __asm__("cli");
    dynamic_interrupt_handlers[interrupt] = (void*)interrupt_exception_handler;
    __asm__("sti");
}

void enable_interrupts() {
    if (!interrupts_ready) panic("Interrupts not ready\n");
    __asm__("sti");
}

void load_interrupts_for_local_cpu() {
    if (interrupts_ready) {
        __asm__("cli");
        __asm__("lidt %0" : : "m"(idtr));
    } else {
        panic("Interrupts not ready\n");
    }
}

void init_interrupts(uint8_t pit_disable) {
    __asm__("cli");
    
    idtr.limit = 256 * sizeof(struct idtdescentry) - 1;
    idtr.offset = (uint64_t)request_current_page_identity();
    memset((void*)idtr.offset, 0, 256 * sizeof(struct idtdescentry));
    
    for (int i = 0; i < 256; i++) {
        set_idt_gate((uint64_t)interrupt_vector[i], i, IDT_TA_InterruptGate, 0, get_kernel_code_selector());
    }

    for (int i = 0; i < 32; i++) {
        dynamic_interrupt_handlers[i] = interrupt_exception_handler;
    }

    dynamic_interrupt_handlers[0x8] = DoubleFault_Handler;    
    dynamic_interrupt_handlers[0xD] = GPFault_Handler;
    dynamic_interrupt_handlers[0xE] = PageFault_Handler;
    dynamic_interrupt_handlers[KBD_IRQ] = KeyboardInt_Handler;
    dynamic_interrupt_handlers[MSE_IRQ] = MouseInt_Handler;
    remap_pic();

    outb(PIC1_DATA, 0xe1);  // Máscara del PIC
    outb(PIC2_DATA, 0xef);
    interrupts_ready = 1;
    return;
}

void raise_interrupt(uint8_t interrupt) {
    if (!interrupts_ready) panic("Interrupts not ready\n");
    __asm__("cli");
    dynamic_interrupt = interrupt;
    __asm__("int %0" : : "i"(DYNAMIC_HANDLER));
}

void global_interrupt_handler(struct cpu_context* ctx, uint8_t cpu_id) {
    __asm__("cli");
    void (*handler)(struct cpu_context* ctx, uint8_t cpu_id) = (void*)dynamic_interrupt_handlers[ctx->interrupt_number];
    
    if (ctx->interrupt_number == DYNAMIC_HANDLER) {
        if (dynamic_interrupt != 0 && dynamic_interrupt != DYNAMIC_HANDLER) {   
            handler = (void*)dynamic_interrupt_handlers[dynamic_interrupt];
            dynamic_interrupt = 0;
        } else {
            panic("Invalid dynamic interrupt\n");
        }
    }

    //printf("Interrupt %d received on CPU %d\n", ctx->interrupt_number, cpu_id);

    if (handler == 0) {
        panic("No handler for interrupt\n");
    }

    handler(ctx, cpu_id);

    pic_eoi(ctx->interrupt_number);
    __asm__("sti");
}