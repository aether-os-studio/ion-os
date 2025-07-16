#include <arch/x86_64/table.hpp>

typedef struct page_table_entry
{
    uint64_t value;
} __attribute__((packed)) page_table_entry_t;

typedef struct
{
    page_table_entry_t entries[512];
} __attribute__((packed)) page_table_t;

typedef struct page_directory
{
    page_table_t *table;
} page_directory_t;

void arch_table::page_table::map(std::uintptr_t virt_addr, std::uintptr_t phys_addr, table::page_table_flags flags)
{
}

void arch_table::page_table::map_range(std::uintptr_t virt_addr, std::uintptr_t phys_addr, std::size_t count, table::page_table_flags flags)
{
}

void arch_table::page_table::unmap(std::uintptr_t virt_addr)
{
}

void arch_table::page_table::unmap_range(std::uintptr_t virt_addr, std::size_t count)
{
}

std::uintptr_t arch_table::page_table::translate_addr(std::uintptr_t virt_addr)
{
}
