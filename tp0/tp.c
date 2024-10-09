/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t   *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

void tp() {
   debug("kernel mem [0x%p - 0x%p]\n", &__kernel_start__, &__kernel_end__);
   debug("MBI flags 0x%x\n", info->mbi->flags);

   multiboot_memory_map_t* entry = (multiboot_memory_map_t*)info->mbi->mmap_addr;
   while((uint32_t)entry < (info->mbi->mmap_addr + info->mbi->mmap_length)) {

      // TODO print "[start - end] type" for each entry
      debug("[0x%llx - 0x%llx] ", entry->addr, entry->addr+entry->len);

      if(entry->type==MULTIBOOT_MEMORY_AVAILABLE)
      {
         debug("MULTIBOOT_MEMORY_AVAILABLE\n");
      }
      else if(entry->type==MULTIBOOT_MEMORY_RESERVED)
      {
         debug("MULTIBOOT_MEMORY_RESERVED\n");
      }
      else if(entry->type==MULTIBOOT_MEMORY_ACPI_RECLAIMABLE)
      {
         debug("MULTIBOOT_MEMORY_ACPI_RECLAIMABLE\n");

      }
      else if(entry->type==MULTIBOOT_MEMORY_NVS)
      {
         debug("MULTIBOOT_MEMORY_NVS\n");
      }
      else
      {
         debug("entry type not known\n");
      }
      entry++;
   }

   int *ptr_in_available_mem;
   ptr_in_available_mem = (int*)0x0;
   debug("Available mem (%p): before: 0x%x ",ptr_in_available_mem, *ptr_in_available_mem); // read
   *ptr_in_available_mem = 0xaaaaaaaa;                           // write
   debug("after: 0x%x\n", *ptr_in_available_mem);                // check

   int *ptr_in_reserved_mem;
   ptr_in_reserved_mem = (int*)0x9fc00;
   debug("Reserved mem (at: %p):  before: 0x%x ",ptr_in_reserved_mem, *ptr_in_reserved_mem); // read
   *ptr_in_reserved_mem = 0xaaaaaaaa;                           // write
   debug("after: 0x%x\n", *ptr_in_reserved_mem);                // check

   int *ptr_after_physical_mem;
   ptr_after_physical_mem = (int*)0xffffffffffffffff;
   debug("Ptr after physcial mem (at: %p):  before: 0x%x ",ptr_after_physical_mem, *ptr_after_physical_mem); // read
   *ptr_after_physical_mem = 0xaaaaaaaa;                           // write
   debug("after: 0x%x\n", *ptr_after_physical_mem);                // check




}
