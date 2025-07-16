#pragma once

#if defined(__x86_64__)
#include <arch/x86_64/arch.hpp>
#elif defined(__aarch64__)
#include <arch/aarch64/arch.hpp>
#elif defined(__riscv)
#include <arch/riscv64/arch.hpp>
#elif defined(__loongarch64)
#include <arch/loongarch64/arch.hpp>
#endif
