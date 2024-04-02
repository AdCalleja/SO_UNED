#include "printf.h"
void panic(const char * str){
    printf("PANIC: %s\n", str);
    while (1); //Si entra en panic no queremos que haga nada más
}