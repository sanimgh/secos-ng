/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>

void bp_handler() {
   asm volatile ("pusha");
   uint32_t eip;
   asm volatile ("mov 4(%%ebp), %0":"=r"(eip));
   debug("EIP = %x\n", (unsigned int) eip);
   asm volatile ("popa");
   asm volatile ("leave; iret");
}

void bp_trigger() {
    // Q4
    // asm volatile ("int3");
    // end Q4

    // Q10
    asm volatile ("int3");
    debug("after bp triggered\n");
    // end Q10
}

void tp() {
   // Q1
   idt_reg_t idtr;
   get_idtr(idtr);
   debug("IDT @ 0x%x\n", (unsigned int) idtr.addr);
   // end Q1
   // Q3
   int_desc_t *bp_dsc = &idtr.desc[3];
   bp_dsc->offset_1 = (uint16_t)((uint32_t)bp_handler);
   bp_dsc->offset_2 = (uint16_t)(((uint32_t)bp_handler)>>16);
   // end Q3
   // Q4
   bp_trigger();
   // end Q4
}
