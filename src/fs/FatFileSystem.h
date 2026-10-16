#pragma once
#include "../disk/VirtualDisk.h"
#include "Structures.h"
#include <string>

class FatFileSystem
{
    VirtualDisk disk;
    description boot_record;
    bool is_formated;

public:
    FatFileSystem(const std::string &host_file);

    bool format(int32_t megabytes);
};