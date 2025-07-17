#pragma once

#include <libs/klibc.hpp>
#include <arch/arch.hpp>

#include <hel/dsocket.hpp>

#define DEFAULT_FD_NUM 32

#define CSIGNAL 0x000000ff              /* Signal mask to be sent at exit.  */
#define CLONE_VM 0x00000100             /* Set if VM shared between processes.  */
#define CLONE_FS 0x00000200             /* Set if fs info shared between processes.  */
#define CLONE_FILES 0x00000400          /* Set if open files shared between processes.  */
#define CLONE_SIGHAND 0x00000800        /* Set if signal handlers shared.  */
#define CLONE_PIDFD 0x00001000          /* Set if a pidfd should be placed \
                           in parent.  */
#define CLONE_PTRACE 0x00002000         /* Set if tracing continues on the child.  */
#define CLONE_VFORK 0x00004000          /* Set if the parent wants the child to \
                           wake it up on mm_release.  */
#define CLONE_PARENT 0x00008000         /* Set if we want to have the same \
                           parent as the cloner.  */
#define CLONE_THREAD 0x00010000         /* Set to add to same thread group.  */
#define CLONE_NEWNS 0x00020000          /* Set to create new namespace.  */
#define CLONE_SYSVSEM 0x00040000        /* Set to shared SVID SEM_UNDO semantics.  */
#define CLONE_SETTLS 0x00080000         /* Set TLS info.  */
#define CLONE_PARENT_SETTID 0x00100000  /* Store TID in userlevel buffer \
                       before MM copy.  */
#define CLONE_CHILD_CLEARTID 0x00200000 /* Register exit futex and memory \
                       location to clear.  */
#define CLONE_DETACHED 0x00400000       /* Create clone detached.  */
#define CLONE_UNTRACED 0x00800000       /* Set if the tracing process can't \
                           force CLONE_PTRACE on this clone.  */
#define CLONE_CHILD_SETTID 0x01000000   /* Store TID in userlevel buffer in \
                       the child.  */
#define CLONE_NEWCGROUP 0x02000000      /* New cgroup namespace.  */
#define CLONE_NEWUTS 0x04000000         /* New utsname group.  */
#define CLONE_NEWIPC 0x08000000         /* New ipcs.  */
#define CLONE_NEWUSER 0x10000000        /* New user namespace.  */
#define CLONE_NEWPID 0x20000000         /* New pid namespace.  */
#define CLONE_NEWNET 0x40000000         /* New network namespace.  */
#define CLONE_IO 0x80000000             /* Clone I/O context.  */

/* cloning flags intersect with CSIGNAL so can be used only with unshare and
   clone3 syscalls.  */
#define CLONE_NEWTIME 0x00000080 /* New time namespace */

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

        bool child_vfork_done;

        bool is_vfork;

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
