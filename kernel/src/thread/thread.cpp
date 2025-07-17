#include <thread/thread.hpp>
#include <mm/hhdm.hpp>
#include <mm/frame.hpp>
#include <mm/heap.hpp>

namespace thread
{

    thread *threads[kMaxThreadCount];

    thread::thread()
    {
    }

    thread::~thread()
    {
    }

    static inline thread *get_free_thread()
    {
        for (std::size_t i = 0; i < kMaxThreadCount; i++)
        {
            if (!threads[i])
            {
                threads[i] = (thread *)malloc(sizeof(thread));
                threads[i]->id = i;
                return threads[i];
            }
        }

        return nullptr;
    }

    thread *create(void *entry)
    {
        thread *t = get_free_thread();

        uint64_t stack = hhdm::phys_to_virt(frame::alloc_frames(kThreadStackSize / PAGE_SIZE));
        t->arch_context = context::arch_context_init(entry, (void *)stack);

        t->mm = (context::mm_context_t *)malloc(sizeof(context::mm_context_t));
        memset(t->mm, 0, sizeof(context::mm_context_t));
        t->mm->page_table_phys = arch::get_current_page_table();
        t->mm->ref_count++;

        return t;
    }

    thread *search(run_state state, int cpu_id)
    {
        return nullptr;
    }

    void init()
    {
        memset(threads, 0, sizeof(threads));
        thread *t = create(nullptr);
    }

}
