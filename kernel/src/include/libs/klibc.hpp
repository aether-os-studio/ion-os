#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdarg>

#define MAX_CPU_NUM 64

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

extern "C"
{

    void *memcpy(void *__restrict dest, const void *__restrict src, std::size_t n);

    void *memset(void *s, int c, std::size_t n);

    void *memmove(void *dest, const void *src, std::size_t n);

    int memcmp(const void *s1, const void *s2, std::size_t n);

    std::size_t strnlen(const char *s, std::size_t maxlen);

    std::size_t strlen(const char *s);
}

static inline int isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

static inline int isalpha(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}

int vsprintf(char *buf, const char *fmt, va_list args);
