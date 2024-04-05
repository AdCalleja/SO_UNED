#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <stdint.h>
#define ASCII_SIZE 56

#define LeftShift 0x2A
#define RightShift 0x36
#define Enter 0x1C
#define Backspace 0x0E
#define Spacebar 0x39
#define MOUSE_IRQ 0x2C //0x20 +12

struct mouse {
    uint8_t left_click_pressed;
    uint8_t right_click_pressed;
};

void init_mouse();
void handle_mouse(uint8_t);
//void halt_until_enter();
void register_mouse_callback(void (*callback)(uint8_t));
#endif