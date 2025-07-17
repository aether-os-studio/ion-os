#include <arch/arch.hpp>
#include <mm/heap.hpp>

namespace context
{

    arch_context_t *arch_context_init(void *entry, void *stack)
    {
        arch_context_t *ctx = (arch_context_t *)malloc(sizeof(arch_context_t));
        ctx->context = (struct pt_regs *)stack;
        ctx->context->rip = (uint64_t)entry;
        ctx->context->rsp = (uint64_t)stack;
        return ctx;
    }

}
