#include <arch/arch.hpp>
#include <mm/hhdm.hpp>

namespace hpet_table
{

    acpi::hpet_info *hpet_addr;
    static std::uint32_t hpet_period = 0;

    std::uint64_t nano_time()
    {
        if (hpet_addr == NULL)
            return 0;
        uint64_t mcv = hpet_addr->mainCounterValue;
        return mcv * hpet_period;
    }

    void init(void *hpet_table)
    {
        acpi::hpet *hpet = (acpi::hpet *)hpet_table;
        hpet_addr = (acpi::hpet_info *)hhdm::phys_to_virt(hpet->base_address.address);
        arch_table::page_table current_table = arch_table::from_current(false);
        current_table.map_range((uintptr_t)hpet_addr, hpet->base_address.address, 1, table::present | table::read_write);
        std::uint32_t counterClockPeriod = hpet_addr->generalCapabilities >> 32;
        hpet_period = counterClockPeriod / 1000000;
        hpet_addr->generalConfiguration |= 1;
    }
}
