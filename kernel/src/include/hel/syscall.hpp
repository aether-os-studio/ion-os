#pragma once

#include <libs/klibc.hpp>
#include <arch/arch.hpp>
#include <hel/dsocket.hpp>

namespace syscall
{
    enum k_hel_debug_kind
    {
        k_hel_debug_kind_info = 1,
        k_hel_debug_kind_debug,
        k_hel_debug_kind_warning,
        k_hel_debug_kind_error,
    };

    enum k_hel_errors : std::size_t
    {
        k_hel_error_none = 0x0,
        k_hel_error_invalid_argument,
        k_hel_error_not_implemented,
        k_hel_error_bad_handler,
        k_hel_error_not_found,
        k_hel_error_already_connected,
        k_hel_error_not_connected,
    };

    enum k_hel_syscall_index : std::uint32_t
    {
        k_hel_base = 0x80000000,

        // Output
        k_debug = k_hel_base + 1,

        // Randomly allocate virtual addresses and map them to the given physical addresses
        k_physmap = k_hel_base + 2,
        // Unmap memory
        k_physunmap = k_hel_base + 3,

        // Create driver socket
        k_driver_cskt = k_hel_base + 10,
        // Bind address
        k_driver_bind = k_hel_base + 11,
        // Connect to drivers
        k_driver_connect = k_hel_base + 12,
        // Accept the connection
        k_driver_accept = k_hel_base + 13,
        // Listen for connection
        k_driver_listen = k_hel_base + 14,
        // Send messages to driver
        k_driver_send = k_hel_base + 15,
        // Receive messages from driver
        k_driver_recv = k_hel_base + 16,
        // Delete driver socket
        k_driver_dskt = k_hel_base + 17,

        k_getpid = k_hel_base + 20,
        k_clone = k_hel_base + 21,
        k_exit = k_hel_base + 22,
    };

    std::size_t k_hel_debug_impl(int kind, std::uintptr_t str_ptr, std::size_t len);

    std::size_t k_hel_physmap_impl(std::uintptr_t physaddr, std::uintptr_t size);

    std::size_t k_hel_physunmap_impl(std::uintptr_t virtaddr, std::uintptr_t size);

    std::size_t k_driver_cskt_impl(std::uintptr_t flags);

    std::size_t k_driver_bind_impl(std::size_t handle_id, dsocket::d_socket_addr_t *addr, std::size_t addrlen);

    std::size_t k_driver_connect_impl(std::size_t handle_id, dsocket::d_socket_addr_t *addr, std::size_t addrlen);

    std::size_t k_driver_accept_impl(std::size_t handle_id, dsocket::d_socket_addr_t *addr, std::size_t addrlen, std::size_t flags);

    std::size_t k_driver_listen_impl(std::size_t handle_id, int backlog);

    std::size_t k_driver_send_impl(std::size_t handle_id, uint8_t *in, std::size_t limit, int flags, dsocket::d_socket_addr_t *addr, std::uint32_t len);

    std::size_t k_driver_recv_impl(std::size_t handle_id, uint8_t *out, std::size_t limit, int flags, dsocket::d_socket_addr_t *addr, std::uint32_t *len);

    uint64_t k_clone_impl(struct pt_regs *regs, uint64_t flags, uint64_t newsp, int *parent_tid, int *child_tid, uint64_t tls);

}
