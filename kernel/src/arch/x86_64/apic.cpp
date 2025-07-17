#include <limine.h>
#include <arch/arch.hpp>
#include <mm/hhdm.hpp>

namespace
{

    __attribute__((used, section(".limine_requests"))) volatile limine_mp_request smp_request = {
        .id = LIMINE_MP_REQUEST,
        .revision = 0,
        .response = nullptr,
        .flags = 0};

}

namespace apic_table
{

    std::uintptr_t lapic_address;
    std::uintptr_t ioapic_address;
    bool x2apic_mode = false;

    void disable_pic()
    {
        x86_io::io_out8(0x21, 0xff);
        x86_io::io_out8(0xa1, 0xff);
    }

    static void lapic_write(uint32_t reg, uint32_t value)
    {
        if (x2apic_mode)
        {
            x86_io::wrmsr(0x800 + (reg >> 4), value);
            return;
        }
        *(uint32_t *)((uint64_t)lapic_address + reg) = value;
    }

    uint32_t lapic_read(uint32_t reg)
    {
        if (x2apic_mode)
        {
            return x86_io::rdmsr(0x800 + (reg >> 4));
        }
        return *((uint32_t *)((uint64_t)lapic_address + reg));
    }

    void local_apic_init()
    {
        x2apic_mode = (smp_request.flags & LIMINE_MP_X2APIC) != 0;

        if (x2apic_mode)
        {
            x86_io::wrmsr(0x1b, x86_io::rdmsr(0x1b) | 1 << 10);
        }

        lapic_write(LAPIC_REG_SPURIOUS, 0xff | 1 << 8);
        lapic_write(LAPIC_REG_TIMER, idx_timer);
        lapic_write(LAPIC_REG_TIMER_DIV, 11);
        lapic_write(LAPIC_REG_TIMER, lapic_read(LAPIC_REG_TIMER) & ~(uint32_t)(0x1000));

        uint64_t b = hpet_table::nano_time();
        lapic_write(LAPIC_REG_TIMER_INITCNT, ~((uint32_t)0));
        for (;;)
            if (hpet_table::nano_time() - b >= 10000000)
                break;
        uint64_t lapic_timer = (~(uint32_t)0) - lapic_read(LAPIC_REG_TIMER_CURCNT);
        uint64_t calibrated_timer_initial =
            (uint64_t)((uint64_t)(lapic_timer * 1000) / LAPIC_TIMER_SPEED);
        lapic_write(LAPIC_REG_TIMER, lapic_read(LAPIC_REG_TIMER) | 1 << 17);
        lapic_write(LAPIC_REG_TIMER_INITCNT, calibrated_timer_initial);

        debug::printk("local apic init successfully\n");
    }

    uint64_t lapic_id()
    {
        uint32_t phy_id = lapic_read(LAPIC_REG_ID);
        return x2apic_mode ? phy_id : (phy_id >> 24);
    }

    static void ioapic_write(uint32_t reg, uint32_t value)
    {
        *(uint32_t *)(ioapic_address) = reg;
        *(uint32_t *)((uint64_t)ioapic_address + 0x10) = value;
    }

    static uint32_t ioapic_read(uint32_t reg)
    {
        *(uint32_t *)(ioapic_address) = reg;
        return *(uint32_t *)((uint64_t)ioapic_address + 0x10);
    }

    void ioapic_add(uint8_t vector, uint32_t irq)
    {
        uint32_t ioredtbl = (uint32_t)(0x10 + (uint32_t)(irq * 2));
        uint64_t redirect = (uint64_t)vector;
        redirect |= lapic_id() << 56;
        ioapic_write(ioredtbl, (uint32_t)redirect);
        ioapic_write(ioredtbl + 1, (uint32_t)(redirect >> 32));
    }

    void io_apic_init()
    {
        arch_table::page_table current_table = arch_table::from_current(false);
        current_table.map_range(hhdm::phys_to_virt(ioapic_address), ioapic_address, 1, table::present | table::read_write);
        ioapic_address = hhdm::phys_to_virt(ioapic_address);
        ioapic_add((uint8_t)idx_timer, 0);
        ioapic_add((uint8_t)idx_keyboard, 1);
        ioapic_add((uint8_t)idx_mouse, 12);
    }

    void init(void *m)
    {
        acpi::madt *madt = (acpi::madt *)m;

        lapic_address = (uint64_t)hhdm::phys_to_virt(madt->local_apic_address);
        arch_table::page_table current_table = arch_table::from_current(false);
        current_table.map_range(lapic_address, madt->local_apic_address, 1, table::present | table::read_write);

        uint64_t current = 0;
        for (;;)
        {
            if (current + ((uint32_t)sizeof(acpi::madt) - 1) >= madt->h.length)
            {
                break;
            }
            acpi::madt_header *header = (acpi::madt_header *)((uint64_t)(&madt->entries) + current);
            if (header->entry_type == MADT_APIC_IO)
            {
                acpi::madt_io_apic *ioapic = (acpi::madt_io_apic *)((uint64_t)(&madt->entries) + current);
                ioapic_address = ioapic->address;
                break;
            }
            current += (uint64_t)header->length;
        }

        disable_pic();
        local_apic_init();
        io_apic_init();
    }

}
