#include <hel/syscall.hpp>
#include <hel/dsocket.hpp>

namespace dsocket
{
    dsocket_t driver_sockets;

    std::size_t dsocket_send(dsocket_t *dsock, uint8_t *in, std::size_t limit, int flags, d_socket_addr_t *addr, std::uint32_t len)
    {
        (void)flags;
        (void)addr;
        (void)len;

        if (!dsock->pair)
        {
            return (std::size_t)-syscall::k_hel_error_not_connected;
        }

        memcpy(&dsock->pair->server_buffer[dsock->pair->server_buffer_pos], in, limit);
        dsock->pair->server_buffer_pos += limit;

        return (std::size_t)limit;
    }

    std::size_t dsocket_accept_send(dsocket_t *dsock, uint8_t *in, std::size_t limit, int flags, d_socket_addr_t *addr, std::uint32_t len)
    {
        (void)flags;
        (void)addr;
        (void)len;

        if (!dsock->pair)
        {
            return (std::size_t)-syscall::k_hel_error_not_connected;
        }

        memcpy(&dsock->pair->client_buffer[dsock->pair->client_buffer_pos], in, limit);
        dsock->pair->client_buffer_pos += limit;

        return (std::size_t)limit;
    }

    std::size_t dsocket_recv(dsocket_t *dsock, uint8_t *out, std::size_t limit, int flags, d_socket_addr_t *addr, std::uint32_t *len)
    {
        (void)flags;
        (void)addr;
        (void)len;

        if (!dsock->pair)
        {
            return (std::size_t)-syscall::k_hel_error_not_connected;
        }

        size_t toCopy = MIN(limit, (std::size_t)dsock->pair->client_buffer_pos);
        memcpy(out, dsock->pair->client_buffer, toCopy);
        memmove(dsock->pair->client_buffer, &dsock->pair->client_buffer[toCopy], dsock->pair->client_buffer_pos - toCopy);
        dsock->pair->client_buffer_pos -= toCopy;

        return (std::size_t)limit;
    }

    std::size_t dsocket_accept_recv(dsocket_t *dsock, uint8_t *out, std::size_t limit, int flags, d_socket_addr_t *addr, std::uint32_t *len)
    {
        (void)flags;
        (void)addr;
        (void)len;

        if (!dsock->pair)
        {
            return (std::size_t)-syscall::k_hel_error_not_connected;
        }

        size_t toCopy = MIN(limit, (std::size_t)dsock->pair->server_buffer_pos);
        memcpy(out, dsock->pair->server_buffer, toCopy);
        memmove(dsock->pair->server_buffer, &dsock->pair->server_buffer[toCopy], dsock->pair->server_buffer_pos - toCopy);
        dsock->pair->server_buffer_pos -= toCopy;

        return (std::size_t)limit;
    }

    dsocket_op normal_ops = {
        .send = dsocket_send,
        .recv = dsocket_recv,
    };
    dsocket_op accept_ops = {
        .send = dsocket_accept_send,
        .recv = dsocket_accept_recv,
    };
}
