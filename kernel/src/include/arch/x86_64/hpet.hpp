#pragma once

#include <arch/x86_64/acpi.hpp>

namespace hpet_table
{

    std::uint64_t nano_time();

    void init(void *hpet_table);

}
