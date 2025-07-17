#pragma once

#include <libs/klibc.hpp>
#include <arch/arch.hpp>

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

        context::arch_context_t *arch_context;

        context::mm_context_t *mm;

    private:
        run_state state;

        enum signal
        {
            kSigNone,
            kSigInterrupt
        };

        enum flags : std::uint32_t
        {
            kFlagServer = 1
        };
    };

    thread *create(void *entry);

    thread *search(run_state state, int cpu_id);

    void init();

}
