#pragma once

#include <libs/klibc.hpp>

namespace gdt
{

    struct gdtr
    {
        uint16_t limit;
        void *pointer;
    } __attribute__((packed));

    void init();

}
