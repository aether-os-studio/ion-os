#pragma once

#include <arch/x86_64/acpi.hpp>

#define LAPIC_TIMER_SPEED 50

namespace apic_table
{

    enum interrupt_index
    {
        idx_timer,
        idx_keyboard,
        idx_mouse,
    };

    void init(void *madt);

}
