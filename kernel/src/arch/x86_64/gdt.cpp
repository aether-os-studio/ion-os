#include <arch/arch.hpp>

namespace gdt
{

    uint64_t gdt_entries[10 + MAX_CPU_NUM * 2];

    void set_tss_descriptor(unsigned int n, void *addr)
    {
        uint64_t limit = sizeof(tss_t);
        *(uint64_t *)(&gdt_entries[n]) = (limit & 0xffff) | (((uint64_t)addr & 0xffff) << 16) | ((((uint64_t)addr >> 16) & 0xff) << 32) | ((uint64_t)0x89 << 40) | ((limit >> 16 & 0x0f) << 48) | (((uint64_t)addr >> 24 & 0xff) << 56); /////89 is attribute
        *(uint64_t *)(&gdt_entries[n + 1]) = (((uint64_t)addr >> 32) & 0xffffffff) | 0;
    }

    void init()
    {
        memset(gdt_entries, 0, sizeof(gdt_entries));

        gdt_entries[0] = 0x0000000000000000U;
        gdt_entries[1] = 0x00a09a0000000000U;
        gdt_entries[2] = 0x00c0920000000000U;
        gdt_entries[3] = 0x00c0f20000000000U;
        gdt_entries[4] = 0x00a0fa0000000000U;

        struct gdtr gdt_pointer = ((struct gdtr){
            .limit = ((uint16_t)((uint32_t)sizeof(gdt_entries) - 1U)),
            .pointer = &gdt_entries,
        });

        asm volatile("lgdt %[ptr]\n\t"
                     "mov %[cseg], %%ax\n\t"
                     "movq .ready_to_ret(%%rip), %%rbx\n\t"
                     "pushq %%rax\n\t"
                     "pushq %%rbx\n\t"
                     "lretq\n\t"
                     ".ready_to_ret:\n\t"
                     ".quad .to_ret\n\t"
                     ".to_ret:\n\t"
                     "mov %[dseg], %%ds\n\t"
                     "mov %[dseg], %%fs\n\t"
                     "mov %[dseg], %%gs\n\t"
                     "mov %[dseg], %%es\n\t"
                     "mov %[dseg], %%ss\n\t"
                     :
                     : [ptr] "m"(gdt_pointer), [cseg] "r"((uint16_t)0x8U), [dseg] "r"((uint16_t)0x10U)
                     : "memory");
    }

    void init_ap()
    {
        struct gdtr gdt_pointer = ((struct gdtr){
            .limit = ((uint16_t)((uint32_t)sizeof(gdt_entries) - 1U)),
            .pointer = &gdt_entries,
        });

        asm volatile("lgdt %[ptr]\n\t"
                     "mov %[cseg], %%ax\n\t"
                     "movq .ready_to_ret_ap(%%rip), %%rbx\n\t"
                     "pushq %%rax\n\t"
                     "pushq %%rbx\n\t"
                     "lretq\n\t"
                     ".ready_to_ret_ap:\n\t"
                     ".quad .to_ret_ap\n\t"
                     ".to_ret_ap:\n\t"
                     "mov %[dseg], %%ds\n\t"
                     "mov %[dseg], %%fs\n\t"
                     "mov %[dseg], %%gs\n\t"
                     "mov %[dseg], %%es\n\t"
                     "mov %[dseg], %%ss\n\t"
                     :
                     : [ptr] "m"(gdt_pointer), [cseg] "r"((uint16_t)0x8U), [dseg] "r"((uint16_t)0x10U)
                     : "memory");
    }

}
