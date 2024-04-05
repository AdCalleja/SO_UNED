#include "mouse.h"
#include "../../io/interrupts.h"
#include "../../util/string.h"
#include "../../util/printf.h"
#include "../../memory/heap.h"

volatile struct mouse *mouse;
void (*mouse_callback)(uint8_t);


void init_mouse() {

    mouse = (struct mouse *)malloc(sizeof(struct mouse));

    memset((void*)mouse, 0, sizeof(struct mouse));

    mouse->left_click_pressed = 0;
    mouse->right_click_pressed = 0;
    mouse_callback = 0;

}

void handle_mouse(uint8_t scancode) {
    printf(scancode);
    // switch(scancode) {
    //     case Spacebar:
    //         printf(" ");
    //         if (mouse_callback != 0) mouse_callback(' ');
    //         return;
    //     case LeftShift:
    //         keyboard->left_shift_pressed = 1;
    //         return;
    //     case LeftShift+0x80:
    //         keyboard->left_shift_pressed = 0;
    //         return;
    //     case RightShift:
    //         keyboard->right_shift_pressed = 1;
    //         return;
    //     case RightShift+0x80:
    //         keyboard->right_shift_pressed = 0;
    //         return;
    //     case Enter:
    //         keyboard->intro_buffered = 1;
    //         printf("\n");
    //         if (keyboard_callback != 0) keyboard_callback(Enter);
    //         return;
    //     case Backspace:
    //         printf("\b \b");
    //         if (keyboard_callback != 0) keyboard_callback(Backspace);
    //         return;
    // }

    // char ascii = translate(scancode, keyboard->left_shift_pressed || keyboard->right_shift_pressed);
    // if (ascii != 0) {
    //     if (keyboard_callback != 0) keyboard_callback(ascii);
    //     printf("%c", ascii);
    // }
}

void register_mouse_callback(void (*callback)(uint8_t)) {
    mouse_callback = callback;
}

// void halt_until_enter() {
//     keyboard->intro_buffered = 0;
//     printf("Press enter to continue...");
//     while (1) {
//         if (keyboard->intro_buffered) {
//             keyboard->intro_buffered = 0;
//             return;
//         }
//     }
// }