#include <arch/x86_64/table.hpp>
#include <mm/hhdm.hpp>
#include <mm/frame.hpp>

typedef struct page_table_entry
{
    uint64_t value;
} __attribute__((packed)) page_table_entry_t;

typedef struct
{
    page_table_entry_t entries[512];
} __attribute__((packed)) page_table_t;

void page_table_clear(page_table_t *table)
{
    for (int i = 0; i < 512; i++)
    {
        table->entries[i].value = 0;
    }
}

page_table_t *page_table_create(page_table_entry_t *entry)
{
    if (entry->value == (uint64_t)NULL)
    {
        std::uint64_t frame = frame::alloc_frames(1);
        entry->value = frame | arch_table::arch_page_table_flags::present | arch_table::arch_page_table_flags::read_write | arch_table::arch_page_table_flags::user;
        page_table_t *table = (page_table_t *)hhdm::phys_to_virt(entry->value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1));
        page_table_clear(table);
        return table;
    }
    page_table_t *table = (page_table_t *)hhdm::phys_to_virt(entry->value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1));
    return table;
}

void arch_table::page_table::map(std::uintptr_t virt_addr, std::uintptr_t phys_addr, int flags)
{
    page_table_t *table = (page_table_t *)hhdm::phys_to_virt(this->get_phys_addr());

    std::uint64_t l4_index = (((virt_addr >> 39)) & 0x1FF);
    std::uint64_t l3_index = (((virt_addr >> 30)) & 0x1FF);
    std::uint64_t l2_index = (((virt_addr >> 21)) & 0x1FF);
    std::uint64_t l1_index = (((virt_addr >> 12)) & 0x1FF);

    page_table_t *l4_table = table;
    page_table_t *l3_table = page_table_create(&(l4_table->entries[l4_index]));
    page_table_t *l2_table = page_table_create(&(l3_table->entries[l3_index]));
    page_table_t *l1_table = page_table_create(&(l2_table->entries[l2_index]));

    std::uint64_t arch_flags = arch_table::arch_page_table_flags::null;

    if (flags & table::page_table_flags::present)
    {
        arch_flags |= arch_table::arch_page_table_flags::present;
    }
    if (flags & table::page_table_flags::read_write)
    {
        arch_flags |= arch_table::arch_page_table_flags::read_write;
    }
    if (flags & table::page_table_flags::user)
    {
        arch_flags |= arch_table::arch_page_table_flags::user;
    }
    if (!(flags & table::page_table_flags::executable))
    {
        arch_flags |= arch_table::arch_page_table_flags::no_executable;
    }

    l1_table->entries[l1_index].value = (phys_addr & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1)) | arch_flags;

    asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
}

void arch_table::page_table::map_range(std::uintptr_t virt_addr, std::uintptr_t phys_addr, std::size_t count, int flags)
{
    for (std::size_t i = 0; i < count; i++)
    {
        if (phys_addr == 0)
        {
            this->map(virt_addr + i * PAGE_SIZE, frame::alloc_frames(1), flags);
        }
        else
        {
            this->map(virt_addr + i * PAGE_SIZE, phys_addr + i * PAGE_SIZE, flags);
        }
    }
}

void arch_table::page_table::unmap(std::uintptr_t virt_addr)
{
    page_table_t *table = (page_table_t *)hhdm::phys_to_virt(this->get_phys_addr());

    std::uint64_t l4_index = (((virt_addr >> 39)) & 0x1FF);
    std::uint64_t l3_index = (((virt_addr >> 30)) & 0x1FF);
    std::uint64_t l2_index = (((virt_addr >> 21)) & 0x1FF);
    std::uint64_t l1_index = (((virt_addr >> 12)) & 0x1FF);

    page_table_t *l4_table = table;
    page_table_t *l3_table = (page_table_t *)hhdm::phys_to_virt((&(l4_table->entries[l4_index]))->value & 0x000fffffffff000);
    page_table_t *l2_table = (page_table_t *)hhdm::phys_to_virt((&(l3_table->entries[l3_index]))->value & 0x000fffffffff000);
    page_table_t *l1_table = (page_table_t *)hhdm::phys_to_virt((&(l2_table->entries[l2_index]))->value & 0x000fffffffff000);

    l4_table->entries[l4_index].value = 0;
    frame::free_frames(l4_table->entries[l4_index].value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1), 1);
    l3_table->entries[l3_index].value = 0;
    frame::free_frames(l3_table->entries[l3_index].value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1), 1);
    l2_table->entries[l2_index].value = 0;
    frame::free_frames(l2_table->entries[l2_index].value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1), 1);
    l1_table->entries[l1_index].value = 0;
    frame::free_frames(l1_table->entries[l1_index].value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1), 1);

    asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
}

void arch_table::page_table::unmap_range(std::uintptr_t virt_addr, std::size_t count)
{
    for (std::size_t i = 0; i < count; i++)
    {
        this->unmap(virt_addr + i * PAGE_SIZE);
    }
}

arch_table::page_table::page_table(std::uintptr_t phys_addr, bool user)
{
    this->phys_addr = phys_addr;
    this->user = user;
}

arch_table::page_table::~page_table()
{
}

namespace arch_table
{

    page_table from_current(bool user)
    {
        std::uintptr_t cr3 = 0;
        asm volatile("movq %%cr3, %0" : "=r"(cr3));
        return arch_table::page_table(cr3, user);
    }

}

std::uintptr_t arch_table::page_table::translate_addr(std::uintptr_t virt_addr)
{
    page_table_t *table = (page_table_t *)hhdm::phys_to_virt(this->get_phys_addr());

    std::uint64_t l4_index = (((virt_addr >> 39)) & 0x1FF);
    std::uint64_t l3_index = (((virt_addr >> 30)) & 0x1FF);
    std::uint64_t l2_index = (((virt_addr >> 21)) & 0x1FF);
    std::uint64_t l1_index = (((virt_addr >> 12)) & 0x1FF);

    page_table_t *l4_table = table;
    page_table_t *l3_table = (page_table_t *)hhdm::phys_to_virt((&(l4_table->entries[l4_index]))->value & 0x000fffffffff000);
    page_table_t *l2_table = (page_table_t *)hhdm::phys_to_virt((&(l3_table->entries[l3_index]))->value & 0x000fffffffff000);
    page_table_t *l1_table = (page_table_t *)hhdm::phys_to_virt((&(l2_table->entries[l2_index]))->value & 0x000fffffffff000);

    return l1_table->entries[l1_index].value & ~hhdm::physical_memory_offset & ~(PAGE_SIZE - 1);
}
