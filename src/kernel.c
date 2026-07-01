#include "kernel.h"

void kernel_main(BootInfo* info){
    (void)info;
    for (;;) {
        _asm_ _volatile_("hlt");
    }
    
}