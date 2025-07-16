#include <limine.h>
#include <arch/arch.hpp>
#include <mm/hhdm.hpp>
#include <mm/frame.hpp>
#include <libs/bitmap.hpp>

namespace
{

    __attribute__((used, section(".limine_requests"))) volatile limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
        .response = nullptr};

}

namespace frame
{
    bitmap::bitmap frame_allocator_bitmap;

    std::uint64_t get_memory_size()
    {
        std::uint64_t all_memory_size = 0;
        struct limine_memmap_response *memory_map = memmap_request.response;

        for (std::uint64_t i = memory_map->entry_count - 1;; i--)
        {
            struct limine_memmap_entry *region = memory_map->entries[i];
            if (region->type == LIMINE_MEMMAP_USABLE)
            {
                all_memory_size = region->base + region->length;
                break;
            }
        }
        return all_memory_size;
    }

    void init()
    {
        struct limine_memmap_response *memory_map = memmap_request.response;

        std::uint64_t memory_size = get_memory_size();

        std::size_t bitmap_size = (memory_size / PAGE_SIZE + 7) / 8;
        std::uintptr_t bitmap_address = 0;

        for (uint64_t i = 0; i < memory_map->entry_count; i++)
        {
            struct limine_memmap_entry *region = memory_map->entries[i];
            if (region->type == LIMINE_MEMMAP_USABLE && region->length >= bitmap_size)
            {
                bitmap_address = region->base;
                break;
            }
        }

        frame_allocator_bitmap.init((std::uint8_t *)(hhdm::phys_to_virt(bitmap_address)), bitmap_size);

        size_t origin_frames = 0;
        for (uint64_t i = 0; i < memory_map->entry_count; i++)
        {
            struct limine_memmap_entry *region = memory_map->entries[i];
            if (region->type == LIMINE_MEMMAP_USABLE)
            {
                size_t start_frame = region->base / PAGE_SIZE;
                size_t frame_count = region->length / PAGE_SIZE;
                origin_frames += frame_count;
                frame_allocator_bitmap.set_range(start_frame, start_frame + frame_count, true);
            }
        }

        size_t bitmap_frame_start = bitmap_address / PAGE_SIZE;
        size_t bitmap_frame_count = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
        size_t bitmap_frame_end = bitmap_frame_start + bitmap_frame_count;
        frame_allocator_bitmap.set_range(bitmap_frame_start, bitmap_frame_end, false);
    }

    std::uintptr_t alloc_frames(std::size_t count)
    {
        size_t frame_index = frame_allocator_bitmap.find_range(count, true);

        if (frame_index == (size_t)-1)
            return 0;

        frame_allocator_bitmap.set_range(frame_index, frame_index + count, false);

        return frame_index * PAGE_SIZE;
    }

    void free_frames(std::uintptr_t addr, std::size_t count)
    {
        size_t frame_index = addr / PAGE_SIZE;
        frame_allocator_bitmap.set_range(frame_index, frame_index + count, true);
    }

}
