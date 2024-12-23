/* GPLv2 (c) Airbus */
#include <debug.h>
#include <segmem.h>
#include <pagemem.h>
#include <string.h>
#include <cr.h>
#include <intr.h>
#include <info.h>
#include <asm.h>
#include <io.h>
/* SEGMENTATION */

#define c0_idx  1
#define d0_idx  2
#define c3_idx  3
#define d3_idx  4
#define ts_idx  5

#define c0_sel  gdt_krn_seg_sel(c0_idx)
#define d0_sel  gdt_krn_seg_sel(d0_idx)
#define c3_sel  gdt_usr_seg_sel(c3_idx)
#define d3_sel  gdt_usr_seg_sel(d3_idx)
#define ts_sel  gdt_krn_seg_sel(ts_idx)

seg_desc_t GDT[6];
tss_t      TSS;


#define c0_dsc(_d) gdt_flat_dsc(_d,0,SEG_DESC_CODE_XR)
#define d0_dsc(_d) gdt_flat_dsc(_d,0,SEG_DESC_DATA_RW)
#define c3_dsc(_d) gdt_flat_dsc(_d,3,SEG_DESC_CODE_XR)
#define d3_dsc(_d) gdt_flat_dsc(_d,3,SEG_DESC_DATA_RW)


/* PAGINATION */

pde32_t *pgd = (pde32_t*)0x350000;
pte32_t *pgd_ptb = (pte32_t*)0x351000;

pde32_t *pgd_task_1 = (pde32_t*)0x352000;
pte32_t *pgd_task_1_ptb_0 = (pte32_t*)0x353000;
pte32_t *pgd_task_1_ptb_1 = (pte32_t*)0x354000;
pte32_t *pgd_task_1_ptb_2 = (pte32_t*)0x355000;

pde32_t *pgd_task_2 = (pde32_t*)0x356000;
pte32_t *pgd_task_2_ptb_0 = (pte32_t*)0x357000;
pte32_t *pgd_task_2_ptb_1 = (pte32_t*)0x358000;
pte32_t *pgd_task_2_ptb_2 = (pte32_t*)0x359000;


/* TASKS */
uint32_t current_task_idx = -1;

struct task {
    void *function;
    uint32_t stack;
    uint32_t interrupt_stack;
    pde32_t *pgd;
    int_ctx_t context;
	char context_saved;
};
struct task tasks[2];


#define STACK_SIZE 4096
uint32_t task1_stack[STACK_SIZE] __attribute__((aligned(4), section(".task1_stack")));
uint32_t task2_stack[STACK_SIZE] __attribute__((aligned(4), section(".task2_stack")));

uint32_t  task1_stack_base = (uint32_t)&task1_stack[STACK_SIZE-1];
uint32_t  task2_stack_base = (uint32_t)&task2_stack[STACK_SIZE-1];

#define INTERRUPT_STACK_SIZE 4096
#define INTERRUPT_STACK_TASK_1 0x380000
#define INTERRUPT_STACK_TASK_2 0x390000

__attribute__((section(".task1_code")))
void task1_function()
{
	uint32_t * counter = (uint32_t*)0x800000;
	*counter=0;
	while(1)
	{
		*counter = *counter + 1;
	}
}


__attribute__((section(".task2_code")))
void sys_counter(uint32_t *counter) {
    asm volatile (
        "movl %0, %%eax\n"
        "int $0x80"
        :
        : "r"(counter)
        : "eax"
    );
}

__attribute__((section(".task2_code")))
void task2_function()
{
	uint32_t *counter = (uint32_t *)0x400000;
	while(1)
	{
		sys_counter(counter);
	}
}

void syscall_isr() {
   asm volatile (
	  "cli                \n"
      "leave ; pusha        \n"
      "mov %esp, %eax      \n"
      "call syscall_handler \n"
      "popa ; sti ; iret"
      );
}


void __regparm__(1) syscall_handler(int_ctx_t *ctx) {
	uint32_t eax = ctx->gpr.eax.raw;
	uint32_t * counter = (uint32_t*)eax;
	debug("Counter value: %u\n", *counter);
    outb(0x80, 0x80);
}



/* clock_interrupt */
void clock_interrupt_isr() {
   asm volatile (
	  "cli                \n"
      "leave ; pusha        \n"
      "mov %esp, %eax      \n"
      "call clock_interrupt_handler \n"
      "popa ; sti ; iret"
      );
}



void __regparm__(1) clock_interrupt_handler(int_ctx_t *ctx) {

	uint32_t next_task = (current_task_idx + 1) % 2;
	if(next_task==1)
	{
		debug("Printing Counter... \n");
	}
	else
	{
		debug("Increasing Counter... \n");
	}

	//if we come from kernel we dont save ctx
	if((int)current_task_idx!=-1)
	{
		memcpy(&tasks[current_task_idx].context,ctx,sizeof(int_ctx_t));
		tasks[current_task_idx].context_saved=1;
	}

	set_ds(d3_sel);
	set_es(d3_sel);
	set_fs(d3_sel);
	set_gs(d3_sel);
	TSS.s0.esp = tasks[next_task].interrupt_stack;
	/* PBL RECONTRER LORS DU CHANGEMENT DE PGD les ebp et esp courrant ne sont plus mappé => faute CPU, creation interrupt stack dans le kernel pour le tss*/
    set_cr3(tasks[next_task].pgd);
	current_task_idx = next_task;

   if (tasks[next_task].context_saved==1)
   {
		memcpy(ctx, &tasks[next_task].context, sizeof(int_ctx_t));
   }
   else
   {

		asm volatile(
			"sti \n"
			"push %0 \n" // ss
			"push %1 \n" // esp
			"pushf   \n" // eflags
			"push %2 \n" // cs
			"push %3 \n" // eip
			::"i"(d3_sel),
			"m"(tasks[next_task].stack),
			"i"(c3_sel),
			"r"(tasks[next_task].function));
		outb(32, 32);
		asm volatile("sti ; iret");
   }
   outb(32, 32);
}

/* --------------------------------------------------------------------  */


void config_segmentation()
{
	gdt_reg_t gdtr;

	GDT[0].raw = 0ULL;

	gdt_flat_dsc(&GDT[c0_idx],0,SEG_DESC_CODE_XR);
	gdt_flat_dsc(&GDT[d0_idx],0,SEG_DESC_DATA_RW);
	gdt_flat_dsc(&GDT[c3_idx],3,SEG_DESC_CODE_XR);
	gdt_flat_dsc(&GDT[d3_idx],3,SEG_DESC_DATA_RW);

	gdtr.desc  = GDT;
   	gdtr.limit = sizeof(GDT) - 1;

	set_gdtr(gdtr);

	set_cs(gdt_krn_seg_sel(c0_idx));

	set_ss(gdt_krn_seg_sel(d0_idx));
	set_ds(gdt_krn_seg_sel(d0_idx));
	set_es(gdt_krn_seg_sel(d0_idx));
	set_fs(gdt_krn_seg_sel(d0_idx));
	set_gs(gdt_krn_seg_sel(d0_idx));

	TSS.s0.esp = get_esp();
    TSS.s0.ss  = d0_sel;
    create_tss_dsc(&GDT[ts_idx], (offset_t)&TSS);
    set_tr(ts_sel);
}

void config_pgds()
{
	memset((void*)pgd, 0, PAGE_SIZE);
	memset((void*)pgd_ptb, 0, PAGE_SIZE);

	memset((void*)pgd_task_1, 0, PAGE_SIZE);
	memset((void*)pgd_task_1_ptb_0, 0, PAGE_SIZE);
	memset((void*)pgd_task_1_ptb_1, 0, PAGE_SIZE);
	memset((void*)pgd_task_1_ptb_2, 0, PAGE_SIZE);

	memset((void*)pgd_task_2, 0, PAGE_SIZE);
	memset((void*)pgd_task_2_ptb_0, 0, PAGE_SIZE);
	memset((void*)pgd_task_2_ptb_1, 0, PAGE_SIZE);
	memset((void*)pgd_task_2_ptb_2, 0, PAGE_SIZE);

	for(int i=0;i<1024;i++) // 0x0 => 0x400
	{
		/* identity map kernel from 0x0 to 0x400000*/
	 	pg_set_entry(&pgd_ptb[i], PG_KRN|PG_RW, i);
	 	pg_set_entry(&pgd_task_1_ptb_0[i], PG_KRN|PG_RW, i);
	 	pg_set_entry(&pgd_task_2_ptb_0[i], PG_KRN|PG_RW, i);

		/* map tasks own memory */
		pg_set_entry(&pgd_task_1_ptb_1[i], PG_USR|PG_RW, 1024 + i); // from physcially 0x400000 to 0x800000
	 	pg_set_entry(&pgd_task_2_ptb_1[i], PG_USR|PG_RW, 2* 1024 + i); // from  physically 0x800000 to 0xc00000
	}



	pg_set_entry(&pgd_task_1_ptb_2[0], PG_USR|PG_RW, 0xe00000 >> 12);
	pg_set_entry(&pgd_task_2_ptb_2[0], PG_USR|PG_RW, 0xe00000 >> 12);

	pg_set_entry(&pgd[0], PG_KRN|PG_RW, page_get_nr(pgd_ptb));

	pg_set_entry(&pgd_task_1[0], PG_KRN|PG_RW, page_get_nr(pgd_task_1_ptb_0));
	pg_set_entry(&pgd_task_2[0], PG_KRN|PG_RW, page_get_nr(pgd_task_2_ptb_0));

	pg_set_entry(&pgd_task_1[1], PG_USR|PG_RW, page_get_nr(pgd_task_1_ptb_1)); // set page table at [1] in order to identity map task space
	pg_set_entry(&pgd_task_2[2], PG_USR|PG_RW, page_get_nr(pgd_task_2_ptb_1)); // set page table at [2] in order to identity map task space

	pg_set_entry(&pgd_task_2[1], PG_USR|PG_RW, page_get_nr(pgd_task_2_ptb_2)); // will be virtually at 0x400000 for task 2
	pg_set_entry(&pgd_task_1[2], PG_USR|PG_RW, page_get_nr(pgd_task_1_ptb_2)); // will be virtually at 0x800000 for task 1


	set_cr3((uint32_t)pgd);

	uint32_t cr0 = get_cr0();
	set_cr0(cr0|CR0_PG);

	debug("pagination enabled\n");
}

void config_interrupt()
{
  	int_desc_t *dsc;
	idt_reg_t  idtr;
	get_idtr(idtr);

	dsc = &idtr.desc[32];
	dsc->offset_1 = (uint16_t)((uint32_t)clock_interrupt_isr);
	dsc->offset_2 = (uint16_t)(((uint32_t)clock_interrupt_isr)>>16);
	dsc->dpl = 3;

	dsc = &idtr.desc[0x80];
    dsc->offset_1 = (uint16_t)((uint32_t)syscall_isr);
    dsc->offset_2 = (uint16_t)(((uint32_t)syscall_isr) >> 16);
    dsc->dpl = 3;
}

void config_task(struct task *task, void *func, uint32_t stack,uint32_t interrupt_stack, pde32_t *pgd) {
    task->function = func;
    task->stack = stack;
    task->pgd = pgd;
	task->interrupt_stack=interrupt_stack;
    memset(&task->context, 0, sizeof(int_ctx_t));
	task->context_saved = false;
}


void config() {
	config_segmentation();
	config_pgds();
	config_interrupt();
    config_task(&tasks[0], task1_function, task1_stack_base,INTERRUPT_STACK_TASK_1+INTERRUPT_STACK_SIZE ,pgd_task_1);
    config_task(&tasks[1], task2_function, task2_stack_base,INTERRUPT_STACK_TASK_2+INTERRUPT_STACK_SIZE ,pgd_task_2);

	set_ds(d3_sel);
    set_es(d3_sel);
    set_fs(d3_sel);
    set_gs(d3_sel);
	TSS.s0.esp = tasks[0].interrupt_stack;
	set_cr3((uint32_t)tasks[0].pgd);
	force_interrupts_on();

	while(1);
}
