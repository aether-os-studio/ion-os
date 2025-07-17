#pragma once

#include <arch/arch.hpp>
#include <thread/thread.hpp>

namespace schedule
{

    void switch_to(struct pt_regs *intr_frame, thread::thread *from, thread::thread *to);

    void init();

}
