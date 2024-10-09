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


    segment_descriptor[0].raw=0;

    segment_descriptor[1].s=0;
    segment_descriptor[1].type=0b1010;
    segment_descriptor[1].dpl=0;

    segment_descriptor[2].s=1;
    segment_descriptor[2].type=0b0010;
    segment_descriptor[2].dpl=0;

    gdt_reg_t gdtr;
    gdtr.limit=sizeof(segment_descriptor)-1;
    gdtr.desc=segment_descriptor;
    set_gdtr(gdtr);
	get_gdtr(gdt);
    print_gdt_content(gdt);



}
