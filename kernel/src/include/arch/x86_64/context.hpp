#pragma once

#include <libs/klibc.hpp>
#include <arch/x86_64/idt.hpp>
#include <arch/x86_64/io.hpp>

#define IA32_KERNEL_GS_BASE 0xc0000102

namespace context
{

    typedef struct mm_context
    {
        std::uintptr_t page_table_phys;
        std::size_t ref_count;
    } mm_context_t;

    struct fx_state
    {
        uint16_t fcw; // x87 control word
        uint16_t fsw; // x87 status word
        uint8_t ftw;  // x87 tag word
        uint8_t reserved0;
        uint16_t fop;
        uint64_t fpuIp;
        uint64_t fpuDp;
        uint32_t mxcsr;
        uint32_t mxcsrMask;
        uint8_t st0[10];
        uint8_t reserved1[6];
        uint8_t st1[10];
        uint8_t reserved2[6];
        uint8_t st2[10];
        uint8_t reserved3[6];
        uint8_t st3[10];
        uint8_t reserved4[6];
        uint8_t st4[10];
        uint8_t reserved5[6];
        uint8_t st5[10];
        uint8_t reserved6[6];
        uint8_t st6[10];
        uint8_t reserved7[6];
        uint8_t st7[10];
        uint8_t reserved8[6];
        uint8_t xmm0[16];
        uint8_t xmm1[16];
        uint8_t xmm2[16];
        uint8_t xmm3[16];
        uint8_t xmm4[16];
        uint8_t xmm5[16];
        uint8_t xmm6[16];
        uint8_t xmm7[16];
        uint8_t xmm8[16];
        uint8_t xmm9[16];
        uint8_t xmm10[16];
        uint8_t xmm11[16];
        uint8_t xmm12[16];
        uint8_t xmm13[16];
        uint8_t xmm14[16];
        uint8_t xmm15[16];
        uint8_t reserved9[48];
        uint8_t available[48];
    };

    typedef struct arch_context
    {
        struct pt_regs *context;
        struct fx_state *fpu;
        std::uint64_t fs;
        std::uint64_t gs;
        std::uint64_t fsbase;
        std::uint64_t gsbase;
    } arch_context_t;

    arch_context_t *arch_context_init(void *entry, void *stack);
    void arch_context_free(arch_context_t *ctx);

    void thread_switch_mm(std::uintptr_t next);
    void thread_switch_to(struct pt_regs *intr_frame, arch_context_t *prev, arch_context_t *next);

    static inline std::uint64_t get_current_thread_addr()
    {
        return (std::uint64_t)x86_io::rdmsr(IA32_KERNEL_GS_BASE);
    }

    static inline void set_current_thread_addr(std::uint64_t thread)
    {
        x86_io::wrmsr(IA32_KERNEL_GS_BASE, thread);
    }

    static inline void context_copy(arch_context *to, arch_context_t *from, std::uint64_t flags)
    {
        (void)flags;

        memcpy(to->context, from->context, sizeof(struct pt_regs));
        memcpy(to->fpu, from->fpu, sizeof(struct fx_state));
        to->fs = from->fs;
        to->gs = from->gs;
        to->fsbase = from->fsbase;
        to->gsbase = from->gsbase;
    }

}
