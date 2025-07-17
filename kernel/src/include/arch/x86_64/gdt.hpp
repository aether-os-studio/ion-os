#pragma once

#include <libs/klibc.hpp>

#define IOBITMAP_SIZE (65536 / 8)

namespace gdt
{

    struct gdtr
    {
        uint16_t limit;
        void *pointer;
    } __attribute__((packed));

    typedef struct tss
    {
        uint32_t reserved0;
        uint64_t rsp0;
        uint64_t rsp1;
        uint64_t rsp2;
        uint64_t reserved1;
        uint64_t ist1;
        uint64_t ist2;
        uint64_t ist3;
        uint64_t ist4;
        uint64_t ist5;
        uint64_t ist6;
        uint64_t ist7;
        uint64_t reserved2;
        uint16_t reserved3;
        uint16_t iomapbaseaddr;
        uint8_t iobitmap[IOBITMAP_SIZE];
    } __attribute__((packed)) tss_t;

    static inline void set_tss64(uint32_t *Table, uint64_t rsp0, uint64_t rsp1, uint64_t rsp2, uint64_t ist1, uint64_t ist2, uint64_t ist3,
                                 uint64_t ist4, uint64_t ist5, uint64_t ist6, uint64_t ist7)
    {
        *(uint64_t *)(Table + 1) = rsp0;
        *(uint64_t *)(Table + 3) = rsp1;
        *(uint64_t *)(Table + 5) = rsp2;

        *(uint64_t *)(Table + 9) = ist1;
        *(uint64_t *)(Table + 11) = ist2;
        *(uint64_t *)(Table + 13) = ist3;
        *(uint64_t *)(Table + 15) = ist4;
        *(uint64_t *)(Table + 17) = ist5;
        *(uint64_t *)(Table + 19) = ist6;
        *(uint64_t *)(Table + 21) = ist7;
    }

    void set_tss_descriptor(unsigned int n, void *addr);

    void init();

    void init_ap();

}
