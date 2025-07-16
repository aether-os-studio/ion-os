#pragma once

#include <libs/klibc.hpp>

#include <arch/x86_64/table.hpp>
#include <arch/x86_64/acpi.hpp>
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/idt.hpp>

namespace arch
{

    void init();

}
