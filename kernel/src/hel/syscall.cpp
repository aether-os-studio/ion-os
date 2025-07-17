#include <hel/syscall.hpp>

#include <mm/frame.hpp>
#include <mm/heap.hpp>
#include <arch/arch.hpp>

#include <hel/dsocket.hpp>
#include <thread/thread.hpp>

namespace syscall
{

    std::size_t k_hel_debug_impl(int kind, std::uintptr_t str_ptr, std::size_t len)
    {
        (void)len;

        if (kind != k_hel_debug_kind_info && kind != k_hel_debug_kind_debug && kind != k_hel_debug_kind_error && kind != k_hel_debug_kind_warning)
        {
            return (std::size_t)-k_hel_error_invalid_argument;
        }

        std::size_t size = 0;
        switch (kind)
        {
        case k_hel_debug_kind_info:
            size = debug::printk("[info] %s", (char *)str_ptr);
            break;
        case k_hel_debug_kind_debug:
            size = debug::printk("[debug] %s", (char *)str_ptr);
            break;
        case k_hel_debug_kind_warning:
            size = debug::printk("[warning] %s", (char *)str_ptr);
            break;
        case k_hel_debug_kind_error:
            size = debug::printk("[error] %s", (char *)str_ptr);
            break;

        default:
            break;
        }
        return size;
    }

    std::size_t k_hel_physmap_impl(std::uintptr_t physaddr, std::uintptr_t size)
    {
        arch_table::page_table table = arch_table::from_current(true);

        std::size_t count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

        std::uintptr_t virtaddr = frame::alloc_frames(count);

        table.map_range(virtaddr, physaddr, count, table::page_table_flags::present | table::page_table_flags::read_write | table::page_table_flags::user);

        return virtaddr;
    }

    std::size_t k_hel_physunmap_impl(std::uintptr_t virtaddr, std::uintptr_t size)
    {
        arch_table::page_table table = arch_table::from_current(true);

        std::size_t count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

        frame::free_frames(virtaddr, count);

        table.unmap_range(virtaddr, count);

        return k_hel_error_none;
    }

    std::size_t k_driver_cskt_impl(std::uintptr_t flags)
    {
        thread::fd_info_t *file_info = (thread::fd_info_t *)malloc(sizeof(thread::fd_info_t));
        file_info->kind = file_info->fd_info_type_dsocket;
        file_info->dsock_handle = (dsocket::dsocket_handle_t *)malloc(sizeof(dsocket::dsocket_handle_t));
        file_info->dsock_handle->socket = (dsocket::dsocket_t *)malloc(sizeof(dsocket::dsocket_t));
        file_info->dsock_handle->socket->flags = flags;
        file_info->dsock_handle->socket->bind_addr = (char *)malloc(DSOCK_ADDR_MAX);
        file_info->dsock_handle->socket->pair = nullptr;
        file_info->dsock_handle->socket->state = dsocket::dsocket_state_disconnected;

        dsocket::dsocket_t *d = &dsocket::driver_sockets;
        while (d->next)
            d = d->next;
        d->next = file_info->dsock_handle->socket;

        return thread::add_fd_info(thread::get_current_thread(), file_info);
    }

    std::size_t k_driver_bind_impl(std::size_t handle_id, dsocket::d_socket_addr_t *addr, std::size_t addrlen)
    {
        (void)addrlen;

        thread::thread *t = thread::get_current_thread();
        if (!t->file_info || !t->file_info->fds || !t->file_info->fds[handle_id])
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        thread::fd_info_t *fd = t->file_info->fds[handle_id];
        if (fd->kind != fd->fd_info_type_dsocket)
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        dsocket::dsocket_handle_t *handle = fd->dsock_handle;
        dsocket::dsocket_t *dsock = handle->socket;

        std::size_t len = 0;
        if (addr->logical_address)
        {
            len = strlen(addr->d_path + 1) + 1;
        }
        else
        {
            len = strlen(addr->d_path);
        }

        memcpy(dsock->bind_addr, addr->d_path, len);

        return 0;
    }

    std::size_t k_driver_connect_impl(std::size_t handle_id, dsocket::d_socket_addr_t *addr, std::size_t addrlen)
    {
        (void)addrlen;

        thread::thread *t = thread::get_current_thread();
        if (!t->file_info || !t->file_info->fds || !t->file_info->fds[handle_id])
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        thread::fd_info_t *fd = t->file_info->fds[handle_id];
        if (fd->kind != fd->fd_info_type_dsocket)
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        dsocket::dsocket_handle_t *handle = fd->dsock_handle;
        dsocket::dsocket_t *dsock = handle->socket;

        if (dsock->pair)
        {
            return (std::size_t)-k_hel_error_already_connected;
        }

        std::size_t len = 0;
        if (addr->logical_address)
        {
            len = strlen(addr->d_path + 1) + 1;
        }
        else
        {
            len = strlen(addr->d_path);
        }

        dsocket::dsocket_t *d = &dsocket::driver_sockets;
        while (d && !memcmp(d->bind_addr, addr->d_path, len))
            d = d->next;

        if (!d || d->state != dsocket::dsocket_state_listening)
        {
            return (std::size_t)-k_hel_error_not_found;
        }

        dsocket::dsocket_pair_t *pair = (dsocket::dsocket_pair_t *)malloc(sizeof(dsocket::dsocket_pair_t));
        memset(pair, 0, sizeof(dsocket::dsocket_pair_t));
        pair->client_fds = 1;
        pair->connected = false;
        d->backlog[d->conn_curr++] = pair;

        dsock->pair = pair;

        while (!pair->connected)
        {
            arch::yield();
        }

        dsock->state = dsocket::dsocket_state_connected;
        handle->op = &dsocket::normal_ops;

        return 0;
    }

    std::size_t k_driver_accept_impl(std::size_t handle_id, dsocket::d_socket_addr_t *addr, std::size_t addrlen, std::size_t flags)
    {
        (void)addr;
        (void)addrlen;

        thread::thread *t = thread::get_current_thread();
        if (!t->file_info || !t->file_info->fds || !t->file_info->fds[handle_id])
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        thread::fd_info_t *fd = t->file_info->fds[handle_id];
        if (fd->kind != fd->fd_info_type_dsocket)
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        dsocket::dsocket_handle_t *handle = fd->dsock_handle;
        dsocket::dsocket_t *dsock = handle->socket;

        while (dsock->conn_curr == 0)
        {
            arch::yield();
        }

        dsocket::dsocket_pair_t *pair = dsock->backlog[0];
        pair->server_fds++;
        pair->path = (char *)malloc(DSOCK_ADDR_MAX);
        pair->server_buffer = (char *)malloc(DSOCK_BUFF_SIZE);
        pair->server_buffer_pos = 0;
        pair->client_buffer = (char *)malloc(DSOCK_BUFF_SIZE);
        pair->client_buffer_pos = 0;
        pair->connected = true;

        dsock->backlog[0] = NULL;
        memmove(dsock->backlog, &dsock->backlog[1], (dsock->conn_max - 1) * sizeof(dsocket::dsocket_handle_t *));
        dsock->conn_max--;
        dsock->state = dsocket::dsocket_state_disconnected;

        thread::fd_info_t *file_info = (thread::fd_info_t *)malloc(sizeof(thread::fd_info_t));
        file_info->kind = file_info->fd_info_type_dsocket;
        file_info->dsock_handle = (dsocket::dsocket_handle_t *)malloc(sizeof(dsocket::dsocket_handle_t));
        file_info->dsock_handle->socket = (dsocket::dsocket_t *)malloc(sizeof(dsocket::dsocket_t));
        file_info->dsock_handle->socket->flags = flags;
        file_info->dsock_handle->socket->bind_addr = (char *)malloc(DSOCK_ADDR_MAX);

        file_info->dsock_handle->socket->pair = pair;
        file_info->dsock_handle->socket->state = dsocket::dsocket_state_connected;

        file_info->dsock_handle->op = &dsocket::accept_ops;

        dsocket::dsocket_t *d = &dsocket::driver_sockets;
        while (d->next)
            d = d->next;
        d->next = file_info->dsock_handle->socket;

        return thread::add_fd_info(t, file_info);
    }

    std::size_t k_driver_listen_impl(std::size_t handle_id, int backlog)
    {
        (void)backlog;

        thread::thread *t = thread::get_current_thread();
        if (!t->file_info || !t->file_info->fds || !t->file_info->fds[handle_id])
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        thread::fd_info_t *fd = t->file_info->fds[handle_id];
        if (fd->kind != fd->fd_info_type_dsocket)
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        dsocket::dsocket_handle_t *handle = fd->dsock_handle;
        dsocket::dsocket_t *dsock = handle->socket;

        dsock->state = dsocket::dsocket_state_listening;

        dsock->conn_curr = 0;
        dsock->conn_max = DSOCKET_CONN_MAX;
        dsock->backlog = (dsocket::dsocket_pair_t **)malloc(sizeof(dsocket::dsocket_pair_t *) * DSOCKET_CONN_MAX);

        return 0;
    }

    std::size_t k_driver_send_impl(std::size_t handle_id, uint8_t *in, std::size_t limit, int flags, dsocket::d_socket_addr_t *addr, std::uint32_t len)
    {
        thread::thread *t = thread::get_current_thread();
        if (!t->file_info || !t->file_info->fds || !t->file_info->fds[handle_id])
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        thread::fd_info_t *fd = t->file_info->fds[handle_id];
        if (fd->kind != fd->fd_info_type_dsocket)
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        dsocket::dsocket_handle_t *handle = fd->dsock_handle;
        dsocket::dsocket_t *dsock = handle->socket;

        return handle->op->send(dsock, in, limit, flags, addr, len);
    }

    std::size_t k_driver_recv_impl(std::size_t handle_id, uint8_t *out, std::size_t limit, int flags, dsocket::d_socket_addr_t *addr, std::uint32_t *len)
    {
        thread::thread *t = thread::get_current_thread();
        if (!t->file_info || !t->file_info->fds || !t->file_info->fds[handle_id])
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        thread::fd_info_t *fd = t->file_info->fds[handle_id];
        if (fd->kind != fd->fd_info_type_dsocket)
        {
            return (std::size_t)-k_hel_error_bad_handler;
        }

        dsocket::dsocket_handle_t *handle = fd->dsock_handle;
        dsocket::dsocket_t *dsock = handle->socket;

        return handle->op->recv(dsock, out, limit, flags, addr, len);
    }

    uint64_t k_clone_impl(struct pt_regs *regs, uint64_t flags, uint64_t newsp, int *parent_tid, int *child_tid, uint64_t tls)
    {
        thread::thread *t = thread::create(NULL, thread::get_current_thread()->tflags);

        thread::get_current_thread()->arch_context->context = regs;

        t->call_in_signal = thread::get_current_thread()->call_in_signal;
        context::context_copy(t->arch_context, thread::get_current_thread()->arch_context, flags);
        t->mm = arch_table::clone_page_table(thread::get_current_thread()->mm, flags);
#if defined(__x86_64__)
        if (newsp)
            t->arch_context->context->rsp = newsp;
#endif
        // t->ppid = thread::get_current_thread()->pid;
        // t->uid = thread::get_current_thread()->uid;
        // t->gid = thread::get_current_thread()->gid;
        // t->euid = thread::get_current_thread()->euid;
        // t->egid = thread::get_current_thread()->egid;
        // t->ruid = thread::get_current_thread()->ruid;
        // t->rgid = thread::get_current_thread()->rgid;
        // t->pgid = thread::get_current_thread()->pgid;
        // t->sid = thread::get_current_thread()->sid;

        t->jiffies = thread::get_current_thread()->jiffies;

        t->file_info = (flags & CLONE_FILES) ? thread::get_current_thread()->file_info : (thread::thread_file_info_t *)malloc(sizeof(thread::thread_file_info_t));

        if (!(flags & CLONE_FILES))
        {
            memset(t->file_info->fds, 0, sizeof(t->file_info->fds));

            for (int i = 0; i < thread::get_current_thread()->file_info->max_fd_count; i++)
            {
                thread::fd_info_t *fd = thread::get_current_thread()->file_info->fds[i];

                if (fd)
                {
                    t->file_info->fds[i] = (thread::fd_info_t *)malloc(sizeof(thread::fd_info_t));
                    memcpy(t->file_info->fds[i], fd, sizeof(thread::fd_info_t));
                }
                else
                {
                    t->file_info->fds[i] = NULL;
                }
            }
        }
        else
        {
            t->file_info->ref_count++;
        }

        // if (flags & CLONE_SIGHAND)
        // {
        //     memcpy(t->actions, thread::get_current_thread()->actions, sizeof(t->actions));
        //     t->signal = thread::get_current_thread()->signal;
        //     t->blocked = thread::get_current_thread()->blocked;
        // }
        // else
        // {
        //     memset(t->actions, 0, sizeof(t->actions));
        // }

        if (flags & CLONE_SETTLS)
        {
#if defined(__x86_64__)
            t->arch_context->fsbase = tls;
#endif
        }

        if (flags & CLONE_PARENT_SETTID)
        {
            *parent_tid = (int)thread::get_current_thread()->id;
        }

        if (flags & CLONE_CHILD_SETTID)
        {
            *child_tid = (int)t->id;
        }

        t->child_vfork_done = false;

        if (flags & CLONE_VFORK)
        {
            t->is_vfork = true;
        }
        else
        {
            t->is_vfork = false;
        }

        if (flags & CLONE_VFORK)
        {
            thread::get_current_thread()->child_vfork_done = false;

            while (!thread::get_current_thread()->child_vfork_done)
            {
                arch::yield();
            }

            thread::get_current_thread()->child_vfork_done = false;
        }

        return t->id;
    }

}
