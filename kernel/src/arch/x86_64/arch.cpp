#include <arch/arch.hpp>

namespace arch
{

    void init()
    {
        gdt::init();
        idt::init();
    }

}
