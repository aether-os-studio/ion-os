#pragma once

#include <libs/klibc.hpp>

namespace hhdm
{

    extern std::size_t physical_memory_offset;

    static inline std::uintptr_t phys_to_virt(std::uintptr_t phys_addr)
    {
        return phys_addr + physical_memory_offset;
    }

    static inline std::uintptr_t virt_to_phys(std::uintptr_t phys_addr)
    {
        return phys_addr - physical_memory_offset;
    }

    void init();

}
