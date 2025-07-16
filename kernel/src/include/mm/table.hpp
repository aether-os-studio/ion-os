#pragma once

#include <libs/klibc.hpp>

namespace table
{

    typedef enum class page_table_flags
    {
        present = 0x1,
        read_write = 0x2,
        user = 0x4,
        executable = 0x08,
    } page_table_flags;

    class page_table
    {
    public:
        page_table(std::uintptr_t phys_addr, bool user);
        ~page_table();

        std::uintptr_t get_phys_addr() const { return phys_addr; }

        virtual void map(std::uintptr_t virt_addr, std::uintptr_t phys_addr, page_table_flags flags)
        {
            (void)virt_addr;
            (void)phys_addr;
            (void)flags;
        }
        virtual void map_range(std::uintptr_t virt_addr, std::uintptr_t phys_addr, std::size_t count, page_table_flags flags)
        {
            (void)virt_addr;
            (void)phys_addr;
            (void)count;
            (void)flags;
        }

        virtual void unmap(std::uintptr_t virt_addr)
        {
            (void)virt_addr;
        }
        virtual void unmap_range(std::uintptr_t virt_addr, std::size_t count)
        {
            (void)virt_addr;
            (void)count;
        }

        virtual std::uintptr_t translate_addr(std::uintptr_t virt_addr)
        {
            (void)virt_addr;
            return 0;
        }

    private:
        std::uintptr_t phys_addr;
        bool user;
    };
}
