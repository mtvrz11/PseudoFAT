#pragma once
#include "../disk/VirtualDisk.h"
#include "Structures.h"
#include <string>
#include <vector>

class FatFileSystem
{
    VirtualDisk disk;
    description boot_record;
    bool is_formatted;
    std::vector<int32_t> fat_table;

    bool boot_fs();

public:
    FatFileSystem(const std::string &host_file);

    bool format(int32_t total_bytes);
};