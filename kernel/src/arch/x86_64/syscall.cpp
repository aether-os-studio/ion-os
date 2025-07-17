#include <arch/arch.hpp>
#include <hel/dsocket.hpp>
#include <thread/thread.hpp>

#define MSR_EFER 0xC0000080  // EFER MSR寄存器
#define MSR_STAR 0xC0000081  // STAR MSR寄存器
#define MSR_LSTAR 0xC0000082 // LSTAR MSR寄存器
#define MSR_SYSCALL_MASK 0xC0000084

namespace syscall
{
    extern "C" void asm_syscall_handler();

    extern "C" uint64_t switch_to_kernel_stack()
    {
        if (thread::get_current_thread()->call_in_signal)
        {
            return thread::get_current_thread()->syscall_stack - thread::kThreadStackSize / 2;
        }
        else
        {
            return thread::get_current_thread()->syscall_stack;
        }
    }

    extern "C" void *real_memcpy(void *dst, const void *src, size_t len)
    {
        return memcpy(dst, src, len);
    }

    void init()
    {
        uint64_t efer;
        efer = x86_io::rdmsr(MSR_EFER);
        efer |= 1;
        uint64_t star = ((uint64_t)((0x18 | 0x3) - 8) << 48) | ((uint64_t)0x08 << 32);
        x86_io::wrmsr(MSR_EFER, efer);
        x86_io::wrmsr(MSR_STAR, star);
        x86_io::wrmsr(MSR_LSTAR, (uint64_t)asm_syscall_handler);
        x86_io::wrmsr(MSR_SYSCALL_MASK, (1 << 9));
    }

    extern "C" void syscall_handler(struct pt_regs *regs, struct pt_regs *user_regs)
    {
        regs->rip = regs->rcx;
        regs->rflags = regs->r11;
        regs->cs = 0x23;
        regs->ss = 0x1B;
        regs->ds = 0x1B;
        regs->es = 0x1B;
        regs->rsp = (uint64_t)(user_regs + 1);

        switch (regs->rax)
        {
        case k_debug:
            regs->rax = syscall::k_hel_debug_impl(regs->rdi, regs->rsi, regs->rdx);
            break;
        case k_physmap:
            regs->rax = syscall::k_hel_physmap_impl(regs->rdi, regs->rsi);
            break;
        case k_physunmap:
            regs->rax = syscall::k_hel_physunmap_impl(regs->rdi, regs->rsi);
            break;
        case k_driver_cskt:
            regs->rax = syscall::k_driver_cskt_impl(regs->rdi);
            break;
        case k_driver_connect:
            regs->rax = syscall::k_driver_connect_impl(regs->rdi, (dsocket::d_socket_addr_t *)regs->rsi, regs->rdx);
            break;
        case k_driver_listen:
            regs->rax = syscall::k_driver_listen_impl(regs->rdi, regs->rsi);
            break;
        case k_driver_accept:
            regs->rax = syscall::k_driver_accept_impl(regs->rdi, (dsocket::d_socket_addr_t *)regs->rsi, regs->rdx, regs->r10);
            break;
        case k_driver_send:
            regs->rax = syscall::k_driver_send_impl(regs->rdi, (uint8_t *)regs->rsi, regs->rdx, regs->rcx, (dsocket::d_socket_addr_t *)regs->r10, regs->r8);
            break;
        case k_driver_recv:
            regs->rax = syscall::k_driver_recv_impl(regs->rdi, (uint8_t *)regs->rsi, regs->rdx, regs->rcx, (dsocket::d_socket_addr_t *)regs->r10, (uint32_t *)regs->r8);
            break;

        case k_getpid:
            regs->rax = thread::get_current_thread()->id;
            break;
        case k_clone:
            regs->rax = syscall::k_clone_impl(regs, regs->rdi, regs->rsi, (int *)regs->rdx, (int *)regs->r10, regs->r8);
            break;
        case k_exit:
            regs->rax = syscall::k_exit_impl(regs->rdi);
            break;

        default:
            regs->rax = (uint64_t)-k_hel_error_not_implemented;
            break;
        }
    }

}
