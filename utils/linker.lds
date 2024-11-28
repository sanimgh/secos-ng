/* GPLv2 (c) Airbus */
OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386");
OUTPUT_ARCH("i386")

ENTRY(entry)

PHDRS
{
   phboot  PT_LOAD FLAGS (7);
   phstack PT_LOAD FLAGS (6);
   phsetup PT_LOAD FLAGS (7);
}

SECTIONS
{
   . = 0x300000;
   .mbh      : { KEEP(*(.mbh)) . = ALIGN(4);     } : phboot
   .stack    : { KEEP(*(.stack))                 } : phstack

   __kernel_start__ = .;

   .idt_jmp  : { KEEP(*(.idt_jmp))               } : phsetup
   .text     : { *(.text)                        } : phsetup
   .rodata   : { *(.rodata)                      } : phsetup
   .data     : { *(.data)                        } : phsetup
   .bss      : { *(.bss COMMON)                  } : phsetup
   /DISCARD/ : { *(.note* .indent .comment)      } : phsetup

   __kernel_end__ = .;

   . = 0x400000;
   .task1_stack : {
       KEEP(*(.task1_stack))
       . = ALIGN(4);
   } : phstack
   .task1_code : {
    KEEP(*(.task1_code))
    . = ALIGN(4);
    } : phsetup

   . = 0x800000;
   .task2_stack : {
       KEEP(*(.task2_stack))
       . = ALIGN(4);
   } : phstack
    .task2_code : {
    KEEP(*(.task2_code))
    . = ALIGN(4);
    } : phsetup

}
