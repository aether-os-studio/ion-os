#pragma once

#include <libs/klibc.hpp>

namespace frame
{

    void init();

    std::uintptr_t alloc_frames(std::size_t count);
    void free_frames(std::uintptr_t addr, std::size_t count);

}
