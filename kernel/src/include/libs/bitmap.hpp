#pragma once

#include <cstdint>
#include <cstddef>

namespace bitmap
{

    class bitmap
    {
    public:
        bitmap();
        ~bitmap();

        void init(std::uint8_t *data, std::size_t size);

        bool get(std::size_t index);
        void set(std::size_t index, bool value);

        std::size_t find_range(bool value, std::size_t count);
        void set_range(std::size_t index_start, std::size_t index_end, bool value);

    private:
        std::uint8_t *data;
        std::size_t size;
    };

}
