/* GPLv2 (c) Airbus */
#include <debug.h>
#include <segmem.h>

void userland() {
   asm volatile ("mov %eax, %cr0");
}

void print_gdt_content(gdt_reg_t gdtr_ptr) {
    seg_desc_t* gdt_ptr;
    gdt_ptr = (seg_desc_t*)(gdtr_ptr.addr);
    int i=0;
    while ((uint32_t)gdt_ptr < ((gdtr_ptr.addr) + gdtr_ptr.limit)) {
        uint32_t start = gdt_ptr->base_3<<24 | gdt_ptr->base_2<<16 | gdt_ptr->base_1;
        uint32_t end;
        if (gdt_ptr->g) {
            end = start + ( (gdt_ptr->limit_2<<16 | gdt_ptr->limit_1) <<12) + 4095;
        } else {
            end = start + (gdt_ptr->limit_2<<16 | gdt_ptr->limit_1);
        }
        debug("%d ", i);
        debug("[0x%x ", start);
        debug("- 0x%x] ", end);
        debug("seg_t: 0x%x ", gdt_ptr->type);
        debug("desc_t: %d ", gdt_ptr->s);
        debug("priv: %d ", gdt_ptr->dpl);
        debug("present: %d ", gdt_ptr->p);
        debug("avl: %d ", gdt_ptr->avl);
        debug("longmode: %d ", gdt_ptr->l);
        debug("default: %d ", gdt_ptr->d);
        debug("gran: %d ", gdt_ptr->g);
        debug("\n");
        gdt_ptr++;
        i++;
    }
}


void tp() {
    gdt_reg_t gdt;
	get_gdtr(gdt);
    print_gdt_content(gdt);

    seg_sel_t ss;
    seg_sel_t ds;
    seg_sel_t cs;

    ss.raw = get_ss();
    ds.raw = get_ds();
    cs.raw = get_cs();

    debug("ss index :%d\n",ss.index);
    debug("ds index :%d\n",ds.index);
    debug("cs index :%d\n",cs.index);


    seg_desc_t segment_descriptor[8];


    segment_descriptor[0].raw = 0;
    segment_descriptor[1].limit_1 = 0xffff;   //:16;     /* bits 00-15 of the segment limit */
    segment_descriptor[1].base_1 = 0x0000;    //:16;     /* bits 00-15 of the base address */
    segment_descriptor[1].base_2 = 0x00;      //:8;      /* bits 16-23 of the base address */
    segment_descriptor[1].type = 11;//Code,RX //:4;      /* segment type */
    segment_descriptor[1].s = 1;              //:1;      /* descriptor type */
    segment_descriptor[1].dpl = 0; //ring0    //:2;      /* descriptor privilege level */
    segment_descriptor[1].p = 1;              //:1;      /* segment present flag */
    segment_descriptor[1].limit_2 = 0xf;      //:4;      /* bits 16-19 of the segment limit */
    segment_descriptor[1].avl = 1;            //:1;      /* available for fun and profit */
    segment_descriptor[1].l = 0; //32bits     //:1;      /* longmode */
    segment_descriptor[1].d = 1;              //:1;      /* default length, depend on seg type */
    segment_descriptor[1].g = 1;              //:1;      /* granularity */
    segment_descriptor[1].base_3 = 0x00;      //:8;      /* bits 24-31 of the base address */
    segment_descriptor[2].limit_1 = 0xffff;   //:16;     /* bits 00-15 of the segment limit */
    segment_descriptor[2].base_1 = 0x0000; //avant 0x0000    //:16;     /* bits 00-15 of the base address */
    segment_descriptor[2].base_2 = 0x00;      //:8;      /* bits 16-23 of the base address */
    segment_descriptor[2].type = 3; //data,RW //:4;      /* segment type */
    segment_descriptor[2].s = 1;              //:1;      /* descriptor type */
    segment_descriptor[2].dpl = 0; //ring0    //:2;      /* descriptor privilege level */
    segment_descriptor[2].p = 1;              //:1;      /* segment present flag */
    segment_descriptor[2].limit_2 = 0xf;      //:4;      /* bits 16-19 of the segment limit */
    segment_descriptor[2].avl = 1;            //:1;      /* available for fun and profit */
    segment_descriptor[2].l = 0; // 32 bits   //:1;      /* longmode */
    segment_descriptor[2].d = 1;              //:1;      /* default length, depend on seg type */
    segment_descriptor[2].g = 1;              //:1;      /* granularity */
    segment_descriptor[2].base_3 = 0x00;      //:8;      /* bits 24-31 of the base address */
    gdt_reg_t gdtr;
    gdtr.limit=sizeof(segment_descriptor)-1;
    gdtr.desc=segment_descriptor;
    set_gdtr(gdtr);
	get_gdtr(gdt);
    print_gdt_content(gdt);



}
