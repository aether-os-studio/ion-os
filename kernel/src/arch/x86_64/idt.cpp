#include <arch/arch.hpp>

#define _set_gate(gate_selector_addr, attr, ist, code_addr)                                    \
    do                                                                                         \
    {                                                                                          \
        uint64_t __d0, __d1;                                                                   \
        asm volatile("movw	%%dx,	%%ax	\n\t"                                                    \
                     "andq	$0x7,	%%rcx	\n\t"                                                   \
                     "addq	%4,	%%rcx	\n\t"                                                     \
                     "shlq	$32,	%%rcx	\n\t"                                                    \
                     "addq	%%rcx,	%%rax	\n\t"                                                  \
                     "xorq	%%rcx,	%%rcx	\n\t"                                                  \
                     "movl	%%edx,	%%ecx	\n\t"                                                  \
                     "shrq	$16,	%%rcx	\n\t"                                                    \
                     "shlq	$48,	%%rcx	\n\t"                                                    \
                     "addq	%%rcx,	%%rax	\n\t"                                                  \
                     "movq	%%rax,	%0	\n\t"                                                     \
                     "shrq	$32,	%%rdx	\n\t"                                                    \
                     "movq	%%rdx,	%1	\n\t"                                                     \
                     : "=m"(*((uint64_t *)(gate_selector_addr))),                              \
                       "=m"(*(1 + (uint64_t *)(gate_selector_addr))), "=&a"(__d0), "=&d"(__d1) \
                     : "i"(attr << 8),                                                         \
                       "3"((uint64_t *)(code_addr)), "2"(0x8 << 16), "c"(ist)                  \
                     : "memory");                                                              \
    } while (0)

namespace idt
{
    struct gate_struct
    {
        unsigned char x[16];
    };

    struct gate_struct idt_table[256];

    static inline void set_intr_gate(unsigned int n, unsigned char ist, void *addr)
    {
        _set_gate((idt_table + n), 0x8E, ist, addr); // p=1，DPL=0, type=E
    }

    static inline void set_trap_gate(unsigned int n, unsigned char ist, void *addr)
    {
        _set_gate((idt_table + n), 0x8F, ist, addr); // p=1，DPL=0, type=F
    }

    static inline void set_system_trap_gate(unsigned int n, unsigned char ist, void *addr)
    {
        _set_gate((idt_table + n), 0xEF, ist, addr); // p=1，DPL=3, type=F
    }

    void dump_regs(struct pt_regs *regs, const char *error_str, ...)
    {
        char buf[128];
        va_list args;
        va_start(args, error_str);
        vsprintf(buf, error_str, args);
        va_end(args);

        // printk("\033[0;0H");
        // printk("\033[2J");
        debug::printk("%s\n", buf);

        debug::printk("RIP = %#018lx\n", regs->rip);
        debug::printk("RAX = %#018lx, RBX = %#018lx\n", regs->rax, regs->rbx);
        debug::printk("RCX = %#018lx, RDX = %#018lx\n", regs->rcx, regs->rdx);
        debug::printk("RDI = %#018lx, RSI = %#018lx\n", regs->rdi, regs->rsi);
        debug::printk("RSP = %#018lx, RBP = %#018lx\n", regs->rsp, regs->rbp);
        debug::printk("R08 = %#018lx, R09 = %#018lx\n", regs->r8, regs->r9);
        debug::printk("R10 = %#018lx, R11 = %#018lx\n", regs->r10, regs->r11);
        debug::printk("R12 = %#018lx, R13 = %#018lx\n", regs->r12, regs->r13);
        debug::printk("R14 = %#018lx, R15 = %#018lx\n", regs->r14, regs->r15);
    }

    // 0 #DE 除法错误
    extern "C" void do_divide_error(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_divder_error(0)");

        while (1)
            asm volatile("hlt");
    }

    // 1 #DB 调试异常
    extern "C" void do_debug(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_debug(1)");

        while (1)
            asm volatile("hlt");
    }

    // 2 不可屏蔽中断
    extern "C" void do_nmi(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_nmi(2)");

        while (1)
            asm volatile("hlt");
    }

    // 3 #BP 断点异常
    extern "C" void do_int3(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_int3(3)");

        return;
    }

    // 4 #OF 溢出异常
    extern "C" void do_overflow(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_overflow(4)");

        while (1)
            asm volatile("hlt");
    }

    // 5 #BR 越界异常
    extern "C" void do_bounds(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_bounds(5)");

        while (1)
            asm volatile("hlt");
    }

    // 6 #UD 无效/未定义的机器码
    extern "C" void do_undefined_opcode(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_undefined_opcode(6)");

        while (1)
            asm volatile("hlt");
    }

    // 7 #NM 设备异常（FPU不存在）
    extern "C" void do_dev_not_avaliable(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_dev_not_avaliable(7)");

        while (1)
            asm volatile("hlt");
    }

    // 8 #DF 双重错误
    extern "C" void do_double_fault(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_double_fault(8)");

        while (1)
            asm volatile("hlt");
    }

    // 9 协处理器越界（保留）
    extern "C" void do_coprocessor_segment_overrun(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_coprocessor_segment_overrun(9)");

        while (1)
            asm volatile("hlt");
    }

    // 10 #TS 无效的TSS段
    extern "C" void do_invalid_TSS(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_invalid_TSS(10)");

        while (1)
            asm volatile("hlt");
    }

    // 11 #NP 段不存在
    extern "C" void do_segment_not_exists(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_segment_not_exists(11)");

        while (1)
            asm volatile("hlt");
    }

    // 12 #SS SS段错误
    extern "C" void do_stack_segment_fault(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_stack_segment_fault(12)");

        while (1)
            asm volatile("hlt");
    }

    // 13 #GP 通用保护性异常
    extern "C" void do_general_protection(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_general_protection(13)");

        while (1)
            asm volatile("hlt");
    }

    // 14 #PF 页故障
    extern "C" void do_page_fault(struct pt_regs *regs, uint64_t error_code)
    {
        uint64_t cr2 = 0;
        // 先保存cr2寄存器的值，避免由于再次触发页故障而丢失值
        // cr2存储着触发异常的线性地址
        asm volatile("movq %%cr2, %0"
                     : "=r"(cr2)::"memory");

        dump_regs(regs, "do_page_fault(14), fault address = %#018lx", cr2);

        (void)error_code;
        (void)regs;

        while (1)
            asm volatile("hlt");
    }

    // 15 Intel保留，请勿使用

    // 16 #MF x87FPU错误
    extern "C" void do_x87_FPU_error(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_x87_FPU_error(16)");

        while (1)
            asm volatile("hlt");
    }

    // 17 #AC 对齐检测
    extern "C" void do_alignment_check(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_alignment_check(17)");

        while (1)
            asm volatile("hlt");
    }

    // 18 #MC 机器检测
    extern "C" void do_machine_check(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_machine_check(18)");

        while (1)
            asm volatile("hlt");
    }

    // 19 #XM SIMD浮点异常
    extern "C" void do_SIMD_exception(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_SIMD_exception(19)");

        while (1)
            asm volatile("hlt");
    }

    // 20 #VE 虚拟化异常
    extern "C" void do_virtualization_exception(struct pt_regs *regs, uint64_t error_code)
    {
        (void)error_code;
        (void)regs;

        dump_regs(regs, "do_virtualization_exception(20)");

        while (1)
            asm volatile("hlt");
    }

    void init()
    {
        struct gdt::gdtr idt_pointer = ((struct gdt::gdtr){
            .limit = ((uint16_t)((uint32_t)sizeof(idt_table) - 1U)),
            .pointer = &idt_table,
        });

        asm volatile("lidt %[ptr]\n\t" : : [ptr] "m"(idt_pointer));

        set_trap_gate(0, 0, (void *)divide_error);
        set_trap_gate(1, 0, (void *)debug);
        set_intr_gate(2, 0, (void *)nmi);
        set_system_trap_gate(3, 0, (void *)int3);
        set_system_trap_gate(4, 0, (void *)overflow);
        set_system_trap_gate(5, 0, (void *)bounds);
        set_trap_gate(6, 0, (void *)undefined_opcode);
        set_trap_gate(7, 0, (void *)dev_not_avaliable);
        set_trap_gate(8, 0, (void *)double_fault);
        set_trap_gate(9, 0, (void *)coprocessor_segment_overrun);
        set_trap_gate(10, 0, (void *)invalid_TSS);
        set_trap_gate(11, 0, (void *)segment_not_exists);
        set_trap_gate(12, 0, (void *)stack_segment_fault);
        set_trap_gate(13, 0, (void *)general_protection);
        set_trap_gate(14, 0, (void *)page_fault);
        // 中断号15由Intel保留，不能使用
        set_trap_gate(16, 0, (void *)x87_FPU_error);
        set_trap_gate(17, 0, (void *)alignment_check);
        set_trap_gate(18, 0, (void *)machine_check);
        set_trap_gate(19, 0, (void *)SIMD_exception);
        set_trap_gate(20, 0, (void *)virtualization_exception);
    }
}
