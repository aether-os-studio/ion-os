#include <mm/hhdm.hpp>
#include <limine.h>

namespace
{

    __attribute__((used, section(".limine_requests"))) volatile limine_hhdm_request hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
        .response = nullptr};

}

namespace hhdm
{

    std::size_t physical_memory_offset;

    void init()
    {
        physical_memory_offset = static_cast<std::size_t>(hhdm_request.response->offset);
    }

}
