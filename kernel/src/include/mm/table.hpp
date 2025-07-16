#pragma once

#include <libs/klibc.hpp>

namespace table
{

    typedef enum page_table_flags
    {
        present = 0x1,
        read_write = 0x2,
        user = 0x4,
        executable = 0x08,
    } page_table_flags;

    class page_table
    {
    public:
        virtual void map(std::uintptr_t virt_addr, std::uintptr_t phys_addr, int flags)
        {
            (void)virt_addr;
            (void)phys_addr;
            (void)flags;
        }
        virtual void map_range(std::uintptr_t virt_addr, std::uintptr_t phys_addr, std::size_t count, int flags)
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
    };
}
