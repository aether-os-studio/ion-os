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

        apic_table::tss[arch::get_current_cpu_id()].rsp0 = (std::uint64_t)(next->context + 1);

        asm volatile("mov %0, %%rsp\n\t"
                     "jmp ret_from_exception" ::"r"(next->context));
    }

}
