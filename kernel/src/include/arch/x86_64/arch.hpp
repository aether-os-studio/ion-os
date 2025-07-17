#pragma once

#include <libs/klibc.hpp>

#include <arch/x86_64/spinlock.hpp>
#include <arch/x86_64/debug.hpp>
#include <arch/x86_64/table.hpp>
#include <arch/x86_64/io.hpp>
#include <arch/x86_64/acpi.hpp>
#include <arch/x86_64/hpet.hpp>
#include <arch/x86_64/apic.hpp>
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/idt.hpp>
#include <arch/x86_64/syscall.hpp>
#include <arch/x86_64/context.hpp>

namespace arch
{

    void init();

    static inline std::uintptr_t get_current_page_table()
    {
        std::uintptr_t cr3;
        asm volatile("movq %%cr3, %0" : "=r"(cr3));
        return cr3;
    }

    static inline int get_current_cpu_id()
    {
        return apic_table::get_cpuid_by_lapic_id(apic_table::lapic_id());
    }

    static inline void yield()
    {
        asm volatile("int %0" ::"i"(apic_table::idx_timer));
    }

}
