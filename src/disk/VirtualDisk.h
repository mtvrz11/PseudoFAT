#pragma once

#include <string>
#include <fstream>

class VirtualDisk
{
    std::string filename;
    std::fstream file;

public:
    bool is_open = false;

    VirtualDisk(const std::string &name);

    void create_empty(int32_t total_bytes);

    void write_bytes(int32_t offset, const void *data, size_t size);

    void read_bytes(int32_t offset, void *buffer, size_t size);
};