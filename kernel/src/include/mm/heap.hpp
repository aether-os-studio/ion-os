#pragma once

#define ALIGNED_BASE 0x1000

#include "mm/alloc/alloc.hpp"

#define KERNEL_HEAP_START 0xffffc00000000000
#define KERNEL_HEAP_SIZE 8 * 1024 * 1024

uint64_t get_all_memusage();
void init_heap();

namespace heap
{

    void init();

}
