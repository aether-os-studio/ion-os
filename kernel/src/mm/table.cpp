#include <mm/table.hpp>

table::page_table::page_table(std::uintptr_t phys_addr, bool user)
{
    this->phys_addr = phys_addr;
    this->user = user;
}

table::page_table::~page_table()
{
}
