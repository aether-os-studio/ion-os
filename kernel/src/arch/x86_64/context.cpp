#include <arch/arch.hpp>
#include <mm/heap.hpp>

namespace context
{

    arch_context_t *arch_context_init(void *entry, void *stack)
    {
        arch_context_t *ctx = (arch_context_t *)malloc(sizeof(arch_context_t));
        ctx->context = (struct pt_regs *)stack - 1;
        memset(ctx->context, 0, sizeof(arch_context_t));
        ctx->context->rflags = 0x200;
        ctx->context->rip = (uint64_t)entry;
        ctx->context->rsp = (uint64_t)ctx->context & ~(uint64_t)0xa;
        ctx->context->cs = 0x8;
        ctx->context->ss = 0x10;
        ctx->context->ds = 0x10;
        ctx->context->es = 0x10;
        ctx->fs = 0x10;
        ctx->gs = 0x10;
        ctx->fsbase = 0;
        ctx->gsbase = 0;
        ctx->fpu = (struct fx_state *)aligned_alloc(16, sizeof(struct fx_state));
        ctx->fpu->mxcsr = 0x1f80;
        ctx->fpu->fcw = 0x037f;
        return ctx;
    }

    void thread_switch_mm(std::uintptr_t next)
    {
        asm volatile("movq %0, %%cr3" ::"r"(next) : "memory");
    }

    void thread_switch_to(struct pt_regs *intr_frame, arch_context_t *prev, arch_context_t *next)
    {
        prev->context = intr_frame;

        asm volatile("fxsave (%0)" ::"r"(prev->fpu));
        asm volatile("fxrstor (%0)" ::"r"(next->fpu));

        prev->fsbase = x86_io::rdmsr(0xc0000100);
        prev->gsbase = x86_io::rdmsr(0xc0000101);

        apic_table::tss[arch::get_current_cpu_id()].rsp0 = (std::uint64_t)(next->context + 1);

        asm volatile("mov %0, %%fs\n\t" ::"r"(next->fs));
        asm volatile("mov %0, %%gs\n\t" ::"r"(next->gs));
        x86_io::wrmsr(0xc0000100, next->fsbase);
        x86_io::wrmsr(0xc0000101, next->gsbase);

        asm volatile("mov %0, %%rsp\n\t"
                     "jmp ret_from_exception" ::"r"(next->context));
    }

}
