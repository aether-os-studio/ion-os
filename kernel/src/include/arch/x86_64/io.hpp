#pragma once

#include <libs/klibc.hpp>

namespace x86_io
{

    static inline uint8_t io_in8(uint16_t port)
    {
        uint8_t data;
        __asm__ volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
        return data;
    }

    static inline uint16_t io_in16(uint16_t port)
    {
        uint16_t data;
        __asm__ volatile("inw %w1, %w0" : "=a"(data) : "Nd"(port));
        return data;
    }

    static inline uint32_t io_in32(uint16_t port)
    {
        uint32_t data;
        __asm__ volatile("inl %1, %0" : "=a"(data) : "Nd"(port));
        return data;
    }

    static inline void io_out8(uint16_t port, uint8_t data)
    {
        __asm__ volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
    }

    static inline void io_out16(uint16_t port, uint16_t data)
    {
        __asm__ volatile("outw %w0, %w1" : : "a"(data), "Nd"(port));
    }

    static inline void io_out32(uint16_t port, uint32_t data)
    {
        __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
    }

    static inline uint64_t rdmsr(uint32_t msr)
    {
        uint32_t eax, edx;
        __asm__ volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(msr));
        return ((uint64_t)edx << 32) | eax;
    }

    static inline void wrmsr(uint32_t msr, uint64_t value)
    {
        uint32_t eax = (uint32_t)value;
        uint32_t edx = value >> 32;
        __asm__ volatile("wrmsr" : : "c"(msr), "a"(eax), "d"(edx));
    }

}
