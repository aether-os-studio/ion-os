#pragma once

#include <libs/klibc.hpp>

typedef struct
{
    volatile uint32_t lock;
    uint64_t rflags;
} spinlock_t;

static inline void spin_lock(spinlock_t *lock)
{
    uint64_t flags;
    asm volatile(
        "pushfq\n\t" // 保存RFLAGS
        "pop %0\n\t" // 存储到flags变量
        "cli\n\t"    // 禁用中断
        : "=r"(flags)
        :
        : "memory");

    asm volatile(
        "1: lock btsl $0, %0\n\t" // 测试并设置
        "   jc 1b\n\t"            // 如果已锁定则重试
        : "+m"(lock->lock)
        :
        : "memory", "cc");

    lock->rflags = flags; // 保存原始中断状态
}

static inline void spin_unlock(spinlock_t *lock)
{
    asm volatile(
        "lock btrl $0, %0\n\t" // 清除锁标志
        : "+m"(lock->lock)
        :
        : "memory", "cc");

    uint64_t flags = lock->rflags;
    asm volatile(
        "push %0\n\t" // 恢复原始RFLAGS
        "popfq"
        :
        : "r"(flags)
        : "memory");
}
