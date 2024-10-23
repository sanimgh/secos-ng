/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>
#include <info.h>


void bp_handler() {
   uint32_t val;
   asm volatile (
    "pusha\n"
    "mov 4(%%ebp), %0"
    :"=r"(val)
	);

   debug("bp handler called, ebp4:(0x%x)\n",val);

	asm volatile (
					"popa\n"
					"leave\n"
					"iret");
}

void bp_trigger() {
	asm volatile ("int3");
	debug("interruption handled well\n");
}

void tp() {
	idt_reg_t idtr;
	get_idtr(idtr);
	debug("idtr addr: %p\n",idtr.desc);
	build_int_desc(&idtr.desc[3], gdt_krn_seg_sel(1), (offset_t)bp_handler);
	bp_trigger();
}
