#include <thread/thread.hpp>
#include <mm/hhdm.hpp>
#include <mm/frame.hpp>
#include <mm/heap.hpp>

namespace thread
{

    thread *threads[kMaxThreadCount];
    thread *idle_threads[MAX_CPU_NUM];

    thread::thread()
    {
    }

    thread::~thread()
    {
    }

    static inline thread *get_free_thread()
    {
        for (std::size_t i = 0; i < apic_table::cpu_count; i++)
        {
            if (!idle_threads[i])
            {
                idle_threads[i] = (thread *)malloc(sizeof(thread));
                idle_threads[i]->id = 0;
                return idle_threads[i];
            }
        }

        for (std::size_t i = 1; i < kMaxThreadCount; i++)
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

    spinlock_t new_thread_lock = SPIN_INIT;

    uint32_t cpu_idx = 0;

    uint32_t alloc_cpu_id()
    {
        uint32_t idx = cpu_idx;
        cpu_idx = (cpu_idx + 1) % apic_table::cpu_count;
        return idx;
    }

    thread *create(void *entry, flags tflags)
    {
        spin_lock(&new_thread_lock);

        thread *t = get_free_thread();

        t->call_in_signal = false;

        t->cpu_id = alloc_cpu_id();
        t->jiffies = 0;

        uint64_t stack = hhdm::phys_to_virt(frame::alloc_frames(kThreadStackSize / PAGE_SIZE));
        t->arch_context = context::arch_context_init(entry, (void *)stack);
        t->syscall_stack = hhdm::phys_to_virt(frame::alloc_frames(kThreadStackSize / PAGE_SIZE));

        t->mm = (context::mm_context_t *)malloc(sizeof(context::mm_context_t));
        memset(t->mm, 0, sizeof(context::mm_context_t));
        t->mm->page_table_phys = arch::get_current_page_table();
        t->mm->ref_count++;

        t->tflags = tflags;

        t->file_info = (thread_file_info_t *)malloc(sizeof(thread_file_info_t));
        memset(t->file_info, 0, sizeof(thread_file_info_t));
        t->file_info->fds = (fd_info_t **)malloc(sizeof(fd_info_t *) * DEFAULT_FD_NUM);
        memset(t->file_info->fds, 0, sizeof(fd_info_t *) * DEFAULT_FD_NUM);
        t->file_info->max_fd_count = DEFAULT_FD_NUM;
        t->file_info->ref_count++;

        t->child_vfork_done = false;
        t->is_vfork = false;

        t->set_state(kRunNone);

        spin_unlock(&new_thread_lock);

        return t;
    }

    std::size_t add_fd_info(thread *t, fd_info_t *f)
    {
        if (t->file_info)
        {
            for (int i = 0; i < t->file_info->max_fd_count; i++)
            {
                if (!t->file_info->fds[i])
                {
                    t->file_info->fds[i] = f;
                    return i;
                }
            }
        }

        return (std::size_t)-1;
    }

    spinlock_t search_lock = SPIN_INIT;

    thread *search(run_state state, int cpu_id)
    {
        spin_lock(&search_lock);

        thread *task = NULL;

        for (size_t i = 1; i < kMaxThreadCount; i++)
        {
            thread *ptr = threads[i];
            if (ptr == NULL)
                break;
            if (ptr->get_state() != state)
                continue;
            if (ptr->cpu_id != cpu_id)
                continue;

            if (task == NULL || ptr->jiffies < task->jiffies)
                task = ptr;
        }

        if (task == NULL && state == kRunNone)
        {
            task = idle_threads[cpu_id];
        }

        spin_unlock(&search_lock);

        return task;
    }

    void idle_entry()
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

    extern "C" void init_entry();

    bool thread_initialized = false;

    void init()
    {
        memset(threads, 0, sizeof(threads));
        for (std::size_t i = 0; i < apic_table::cpu_count; i++)
        {
            create((void *)idle_entry, flags::kFlagNone);
        }
        cpu_idx = 0;

        create((void *)init_entry, flags::kFlagServer);

        set_current_thread(idle_threads[arch::get_current_cpu_id()]);

        thread_initialized = true;
    }

    std::size_t getpid()
    {
        return get_current_thread()->id;
    }

    std::size_t clone(struct pt_regs *regs)
    {
        (void)regs;
        return syscall::k_hel_error_not_implemented;
    }

    std::size_t exit(std::size_t exit_code)
    {
        (void)exit_code;
        return syscall::k_hel_error_none;
    }

}
