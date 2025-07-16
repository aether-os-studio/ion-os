#include <libs/klibc.hpp>

#include <mm/table.hpp>

#define PAGE_SIZE 4096

namespace arch_table
{

    typedef enum class arch_page_table_flags : std::size_t
    {
        present = (std::size_t)1 << 0,
        read_write = (std::size_t)1 << 1,
        user = (std::size_t)1 << 2,
        no_executable = (std::size_t)1 << 63,
    } arch_page_table_flags;

    class page_table : table::page_table
    {
    public:
        void map(std::uintptr_t virt_addr, std::uintptr_t phys_addr, table::page_table_flags flags);
        void map_range(std::uintptr_t virt_addr, std::uintptr_t phys_addr, std::size_t count, table::page_table_flags flags);

        void unmap(std::uintptr_t virt_addr);
        void unmap_range(std::uintptr_t virt_addr, std::size_t count);

        std::uintptr_t translate_addr(std::uintptr_t virt_addr);
    };

}
