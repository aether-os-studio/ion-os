#pragma once

#include <libs/klibc.hpp>

#define DSOCK_ADDR_MAX 109
#define DSOCK_BUFF_SIZE 4096

namespace dsocket
{
    typedef struct dsocket_addr
    {
        std::uint8_t logical_address;
        char d_path[DSOCK_ADDR_MAX];
    } d_socket_addr_t;

    typedef struct dsocket_pair
    {
        bool connected;

        char *path;

        char *client_buffer;
        int client_buffer_pos;
        int client_buffer_size;
        int client_fds;

        char *server_buffer;
        int server_buffer_pos;
        int server_fds;
    } dsocket_pair_t;

    typedef enum dsocket_state
    {
        dsocket_state_disconnected,
        dsocket_state_listening,
        dsocket_state_connected,
    } dsocket_state_t;

#define DSOCKET_CONN_MAX 8

    typedef struct dsocket
    {
        struct dsocket *next;

        std::uint64_t flags;

        char *bind_addr;

        dsocket_state_t state;

        int conn_max;
        int conn_curr;
        dsocket_pair_t **backlog;

        dsocket_pair_t *pair;
    } dsocket_t;

    extern dsocket_t driver_sockets;

    typedef struct dsocket_op
    {
        std::size_t (*send)(dsocket_t *dsock, uint8_t *in, std::size_t limit, int flags, d_socket_addr_t *addr, std::uint32_t len);
        std::size_t (*recv)(dsocket_t *dsock, uint8_t *out, std::size_t limit, int flags, d_socket_addr_t *addr, std::uint32_t *len);
    } dsocket_op_t;

    extern dsocket_op normal_ops;
    extern dsocket_op accept_ops;

    typedef struct dsocket_handle
    {
        dsocket_op_t *op;
        dsocket_t *socket;
    } dsocket_handle_t;
}
