#pragma once
#include "../disk/VirtualDisk.h"
#include "Structures.h"
#include <string>

class FatFileSystem
{
    VirtualDisk disk;
    description boot_record;
    bool is_formatted;

public:
    FatFileSystem(const std::string &host_file);

    bool format(int32_t total_bytes);
};