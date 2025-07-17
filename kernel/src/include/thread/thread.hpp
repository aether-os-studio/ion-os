#pragma once

#include <libs/klibc.hpp>
#include <arch/arch.hpp>

#include <hel/dsocket.hpp>

#define DEFAULT_FD_NUM 32

namespace thread
{
    static const std::size_t kMaxThreadCount = 4096;
    static const std::size_t kThreadStackSize = 65536;

    enum run_state
    {
        kRunNone,
        kRunActive,
        kRunSuspended,
        kRunDeferred,
        kRunBlocked,
        kRunInterrupted,
        kRunTerminated
    };

    enum signal
    {
        kSigNone,
        kSigInterrupt
    };

    enum flags : std::uint32_t
    {
        kFlagNone = 0,
        kFlagServer = 1
    };

    typedef struct fd_info
    {
        enum
        {
            fd_info_type_dsocket,
        } kind;

        union
        {
            dsocket::dsocket_handle_t *dsock_handle;
        };
    } fd_info_t;

    typedef struct thread_file_info
    {
        fd_info_t **fds;
        int max_fd_count;
        int ref_count;
    } thread_file_info_t;

    class thread
    {
    public:
        thread();
        ~thread();

        run_state get_state() const
        {
            return state;
        }

        void set_state(run_state new_state)
        {
            state = new_state;
        }

        std::size_t get_id() const
        {
            return id;
        }

        std::size_t id;

        bool call_in_signal;

        int cpu_id;

        std::uint64_t jiffies;

        context::arch_context_t *arch_context;

        std::uintptr_t syscall_stack;

        context::mm_context_t *mm;

        thread_file_info_t *file_info;

        flags tflags;

    private:
        run_state state;
    };

    extern bool thread_initialized;

    extern thread *threads[kMaxThreadCount];
    extern thread *idle_threads[MAX_CPU_NUM];

    thread *create(void *entry, flags tflags);

    thread *search(run_state state, int cpu_id);

    static inline thread *get_current_thread()
    {
        return (thread *)context::get_current_thread_addr();
    }

    static inline void set_current_thread(thread *thread)
    {
        context::set_current_thread_addr((std::uint64_t)thread);
    }

    void init();

    std::size_t add_fd_info(thread *t, fd_info_t *f);

}
