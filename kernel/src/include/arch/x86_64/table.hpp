#include <libs/klibc.hpp>

#include <mm/table.hpp>

#define PAGE_SIZE 4096

namespace arch_table
{

    typedef enum arch_page_table_flags
    {
        null = (std::size_t)0,
        present = (std::size_t)1 << 0,
        read_write = (std::size_t)1 << 1,
        user = (std::size_t)1 << 2,
        no_executable = (std::size_t)1 << 63,
    } arch_page_table_flags;

    class page_table : table::page_table
    {
    public:
        page_table(std::uintptr_t phys_addr, bool user);
        ~page_table();

        std::uintptr_t get_phys_addr() const { return phys_addr; }

        void map(std::uintptr_t virt_addr, std::uintptr_t phys_addr, int flags);
        void map_range(std::uintptr_t virt_addr, std::uintptr_t phys_addr, std::size_t count, int flags);

        void unmap(std::uintptr_t virt_addr);
        void unmap_range(std::uintptr_t virt_addr, std::size_t count);

        std::uintptr_t translate_addr(std::uintptr_t virt_addr);

    private:
        std::uintptr_t phys_addr;
        bool user;
    };

    page_table from_current(bool user);

}
