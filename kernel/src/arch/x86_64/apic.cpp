#include <limine.h>
#include <arch/arch.hpp>
#include <mm/hhdm.hpp>
#include <mm/frame.hpp>
#include <thread/thread.hpp>

namespace
{

    __attribute__((used, section(".limine_requests"))) volatile limine_mp_request mp_request = {
        .id = LIMINE_MP_REQUEST,
        .revision = 0,
        .response = nullptr,
        .flags = 0};

}

#define ltr(n)                                    \
    do                                            \
    {                                             \
        asm volatile("ltr %%ax" ::"a"((n) << 3)); \
    } while (0)

namespace apic_table
{
    gdt::tss_t tss[MAX_CPU_NUM];

    std::uintptr_t lapic_address;
    std::uintptr_t ioapic_address;
    bool x2apic_mode = false;

    std::size_t cpu_count = 0;
    uint32_t cpuid_to_lapicid[MAX_CPU_NUM];

    void tss_init()
    {
        uint64_t sp = hhdm::phys_to_virt(frame::alloc_frames(thread::kThreadStackSize / PAGE_SIZE)) + thread::kThreadStackSize;
        uint64_t offset = 10 + arch::get_current_cpu_id() * 2;
        gdt::set_tss64((uint32_t *)&tss[arch::get_current_cpu_id()], sp, sp, sp, sp, sp, sp, sp, sp, sp, sp);
        gdt::set_tss_descriptor(offset, &tss[arch::get_current_cpu_id()]);
        ltr(offset);
    }

    uint32_t get_cpuid_by_lapic_id(uint32_t lapic_id)
    {
        for (uint32_t cpu_id = 0; cpu_id < cpu_count; cpu_id++)
        {
            if (cpuid_to_lapicid[cpu_id] == lapic_id)
            {
                return cpu_id;
            }
        }

        debug::printk("Cannot get cpu id, lapic id = %d\n", lapic_id);

        return 0;
    }

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
        x2apic_mode = (mp_request.flags & LIMINE_MP_X2APIC) != 0;

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

    void ioapic_enable(uint8_t vector)
    {
        uint64_t index = 0x10 + ((vector - 32) * 2);
        uint64_t value = (uint64_t)ioapic_read(index + 1) << 32 | (uint64_t)ioapic_read(index);
        value &= (~0x10000UL);
        ioapic_write(index, (uint32_t)(value & 0xFFFFFFFF));
        ioapic_write(index + 1, (uint32_t)(value >> 32));
    }

    void io_apic_init()
    {
        arch_table::page_table current_table = arch_table::from_current(false);
        current_table.map_range(hhdm::phys_to_virt(ioapic_address), ioapic_address, 1, table::present | table::read_write);
        ioapic_address = hhdm::phys_to_virt(ioapic_address);
        ioapic_add((uint8_t)idx_timer, 0);
        ioapic_enable((uint8_t)idx_timer);
        ioapic_add((uint8_t)idx_keyboard, 1);
        ioapic_enable((uint8_t)idx_keyboard);
        ioapic_add((uint8_t)idx_mouse, 12);
        ioapic_enable((uint8_t)idx_mouse);
    }

    void sse_init()
    {
        asm volatile("movq %cr0, %rax\n\t"
                     "and $0xFFF3, %ax	\n\t" // clear coprocessor emulation CR0.EM and CR0.TS
                     "or $0x2, %ax\n\t"       // set coprocessor monitoring  CR0.MP
                     "movq %rax, %cr0\n\t"
                     "movq %cr4, %rax\n\t"
                     "or $(3 << 9), %ax\n\t" // set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
                     "movq %rax, %cr4\n\t");
    }

    bool ap_initialized = false;

    void ap_entry(struct limine_mp_info *cpu)
    {
        while (!ap_initialized)
        {
            asm volatile("pause");
        }

        sse_init();

        gdt::init_ap();
        idt::init_ap();

        tss_init();

        debug::printk("AP %d starting\n", cpu->processor_id);

        apic_table::local_apic_init();

        while (!thread::thread_initialized)
        {
            asm volatile("pause");
        }

        thread::set_current_thread(thread::idle_threads[cpu->processor_id]);

        for (;;)
        {
#if defined(__x86_64__)
            asm("sti\n\thlt");
#elif defined(__aarch64__) || defined(__riscv)
            asm("wfi");
#elif defined(__loongarch64)
            asm("idle 0");
#endif
        }
    }

    void smp_init()
    {
        struct limine_mp_response *mp_response = mp_request.response;

        cpu_count = mp_response->cpu_count;

        for (uint64_t i = 0; i < mp_response->cpu_count; i++)
        {
            struct limine_mp_info *cpu = mp_response->cpus[i];
            cpuid_to_lapicid[cpu->processor_id] = cpu->lapic_id;

            if (cpu->lapic_id == mp_response->bsp_lapic_id)
                continue;

            cpu->goto_address = ap_entry;
        }

        ap_initialized = true;
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

        sse_init();

        disable_pic();
        local_apic_init();
        io_apic_init();

        smp_init();

        tss_init();
    }

    void end_of_interrupt()
    {
        lapic_write(0xb0, 0);
    }

}
