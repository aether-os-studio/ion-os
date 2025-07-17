#include <limine.h>
#include <arch/arch.hpp>
#include <mm/hhdm.hpp>
#include <mm/table.hpp>

namespace
{

    __attribute__((used, section(".limine_requests"))) volatile limine_rsdp_request rsdp_request = {
        .id = LIMINE_RSDP_REQUEST,
        .revision = 0,
        .response = nullptr};

}

#define load_table(name, func)                \
    do                                        \
    {                                         \
        void *name = acpi::find_table(#name); \
        if (name == NULL)                     \
        {                                     \
            return;                           \
        }                                     \
        else                                  \
            func(name);                       \
    } while (0)

namespace acpi
{
    xsdt *x;

    void *find_table(const char *name)
    {
        uint64_t entry_count = (x->h.length - 32) / 8;
        uint64_t *t = (uint64_t *)((char *)x + offsetof(xsdt, pointer_to_other_sdt));
        for (uint64_t i = 0; i < entry_count; i++)
        {
            uint64_t phys = (uint64_t)(*(t + i));
            uint64_t ptr = hhdm::phys_to_virt(phys);
            arch_table::page_table current_table = arch_table::from_current(false);
            current_table.map_range(ptr, phys, 1, table::present | table::read_write);
            uint8_t signa[5] = {0};
            memcpy(signa, ((struct ACPISDTheader *)ptr)->signature, 4);
            if (memcmp(signa, name, 4) == 0)
            {
                return (void *)ptr;
            }
        }
        return NULL;
    }

    void init()
    {
        std::uintptr_t rsdp_phys_ptr = (std::uintptr_t)rsdp_request.response->address;
        std::uintptr_t rsdp_virt_ptr = hhdm::phys_to_virt(rsdp_phys_ptr);

        arch_table::page_table current_table = arch_table::from_current(false);
        current_table.map_range(rsdp_virt_ptr, rsdp_phys_ptr, 1, table::present | table::read_write);

        rsdp *p = (rsdp *)rsdp_virt_ptr;

        if (p->xsdt_address == 0)
            return;

        std::uintptr_t xsdt_virt_ptr = hhdm::phys_to_virt(p->xsdt_address);
        current_table.map_range(xsdt_virt_ptr, p->xsdt_address, 1, table::present | table::read_write);

        x = (xsdt *)xsdt_virt_ptr;

        load_table(HPET, hpet_table::init);
        load_table(APIC, apic_table::init);
    }

}
