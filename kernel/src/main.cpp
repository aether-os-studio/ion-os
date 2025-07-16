#include <cstdint>
#include <cstddef>
#include <limine.h>

namespace
{

    __attribute__((used, section(".limine_requests"))) volatile LIMINE_BASE_REVISION(3);

}

namespace
{

    __attribute__((used, section(".limine_requests_start"))) volatile LIMINE_REQUESTS_START_MARKER;

    __attribute__((used, section(".limine_requests_end"))) volatile LIMINE_REQUESTS_END_MARKER;

}

namespace
{

    void hcf()
    {
        for (;;)
        {
#if defined(__x86_64__)
            asm("hlt");
#elif defined(__aarch64__) || defined(__riscv)
            asm("wfi");
#elif defined(__loongarch64)
            asm("idle 0");
#endif
        }
    }

}

extern "C"
{
    int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
    void __cxa_pure_virtual() { hcf(); }
    void *__dso_handle;
}

extern void (*__init_array[])();
extern void (*__init_array_end[])();

#include <mm/hhdm.hpp>
#include <mm/frame.hpp>
#include <arch/arch.hpp>

extern "C" void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    for (std::size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }

    hhdm::init();

    frame::init();

    hcf();
}
