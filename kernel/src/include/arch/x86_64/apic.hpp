#pragma once

#include <arch/x86_64/acpi.hpp>
#include <arch/x86_64/gdt.hpp>

#define LAPIC_TIMER_SPEED 50

namespace apic_table
{
    extern gdt::tss_t tss[MAX_CPU_NUM];

    extern std::size_t cpu_count;

    enum interrupt_index
    {
        idx_timer = 0x20,
        idx_keyboard,
        idx_mouse,
    };

    void init(void *madt);

    uint32_t get_cpuid_by_lapic_id(uint32_t lapic_id);

    uint64_t lapic_id();

    void end_of_interrupt();

}
