#pragma once

#include <libs/klibc.hpp>

struct pt_regs
{
    std::uint64_t r15;
    std::uint64_t r14;
    std::uint64_t r13;
    std::uint64_t r12;
    std::uint64_t r11;
    std::uint64_t r10;
    std::uint64_t r9;
    std::uint64_t r8;
    std::uint64_t rbx;
    std::uint64_t rcx;
    std::uint64_t rdx;
    std::uint64_t rsi;
    std::uint64_t rdi;
    std::uint64_t rbp;
    std::uint64_t ds;
    std::uint64_t es;
    std::uint64_t rax;
    std::uint64_t func;
    std::uint64_t errcode;
    std::uint64_t rip;
    std::uint64_t cs;
    std::uint64_t rflags;
    std::uint64_t rsp;
    std::uint64_t ss;
};

namespace idt
{

    // 除法错误
    extern "C" void divide_error();
    // 调试
    extern "C" void debug();
    // 不可屏蔽中断
    extern "C" void nmi();
    //
    extern "C" void int3();
    // 溢出
    extern "C" void overflow();
    // 边界问题
    extern "C" void bounds();
    // 未定义的操作数
    extern "C" void undefined_opcode();
    // 设备不可用
    extern "C" void dev_not_avaliable();
    extern "C" void double_fault();
    extern "C" void coprocessor_segment_overrun();
    extern "C" void invalid_TSS();
    extern "C" void segment_not_exists();
    extern "C" void stack_segment_fault();
    extern "C" void general_protection();
    // 缺页异常
    extern "C" void page_fault();
    extern "C" void x87_FPU_error();
    extern "C" void alignment_check();
    extern "C" void machine_check();
    extern "C" void SIMD_exception();
    extern "C" void virtualization_exception();

    void init();
}
