#pragma once
#include <vector>
#include <cstdint>
#include "../disk/VirtualDisk.h"
#include "Structures.h"

class FatManager
{
    VirtualDisk &disk;
    description &boot_record;
    std::vector<int32_t> fat_table;

public:
    FatManager(VirtualDisk &disk, description &boot_record);
    bool load_from_disk();
    void format_new();
    std::vector<int32_t> allocate_chain(int32_t count);
    void free_chain(int32_t start_cluster);
    std::vector<int32_t> get_cluster_chain(int32_t start_cluster);
    int32_t append_cluster(int32_t tail_cluster);
};