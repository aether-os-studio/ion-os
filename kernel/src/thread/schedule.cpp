#include <thread/schedule.hpp>

namespace schedule
{

    void switch_to(struct pt_regs *intr_frame, thread::thread *from, thread::thread *to)
    {
        context::thread_switch_mm(to->mm->page_table_phys);
        from->set_state(thread::kRunNone);
        to->set_state(thread::kRunActive);
        context::thread_switch_to(intr_frame, from->arch_context, to->arch_context);
    }

    void init()
    {
    }

}
