#include "kernel.h"

void kernel_main(BootInfo* info){
    (void)info;
    for (;;) {
        __asm__ __volatile__("hlt");
    }
    
}