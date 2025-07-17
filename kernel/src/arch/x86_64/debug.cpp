#include <arch/arch.hpp>

#define SERIAL_PORT 0x3F8

namespace debug
{

    void init()
    {
        x86_io::io_out8(SERIAL_PORT + 1, 0x00); // 禁止COM的中断发生
        x86_io::io_out8(SERIAL_PORT + 3, 0x80); // 启用DLAB（设置波特率除数）。
        x86_io::io_out8(SERIAL_PORT + 0, 0x03); // 设置除数为3，(低位) 38400波特
        x86_io::io_out8(SERIAL_PORT + 1, 0x00); //            (高位)
        x86_io::io_out8(SERIAL_PORT + 3, 0x03); // 8位，无奇偶性，一个停止位
        x86_io::io_out8(SERIAL_PORT + 2, 0xC7); // 启用FIFO，有14字节的阈值
        x86_io::io_out8(SERIAL_PORT + 4, 0x0B); // 启用IRQ，设置RTS/DSR
        x86_io::io_out8(SERIAL_PORT + 4, 0x1E); // 设置为环回模式，测试串口
        x86_io::io_out8(SERIAL_PORT + 0, 0xAE); // 测试串口（发送字节0xAE并检查串口是否返回相同的字节）

        // 检查串口是否有问题（即：与发送的字节不一样）
        if (x86_io::io_in8(SERIAL_PORT + 0) != 0xAE)
        {
            return;
        }

        // 如果串口没有故障，将其设置为正常运行模式。
        // (非环回，启用IRQ，启用OUT#1和OUT#2位)
        x86_io::io_out8(SERIAL_PORT + 4, 0x0F);
    }

    void write(char a)
    {
        while ((x86_io::io_in8(SERIAL_PORT + 5) & 0x20) == 0)
            ;
        x86_io::io_out8(SERIAL_PORT, a);
    }

    char buf[4096];

    spinlock_t lock = SPIN_INIT;

    void printk(const char *fmt, ...)
    {
        spin_lock(&lock);

        va_list args;
        va_start(args, fmt);
        int len = vsprintf(buf, fmt, args);
        va_end(args);

        for (int i = 0; i < len; i++)
        {
            if (buf[i] == '\n')
                write('\r');
            write(buf[i]);
        }

        spin_unlock(&lock);
    }

}
