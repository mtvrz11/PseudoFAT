#pragma once
#include <vector>
#include <string>
#include "../disk/VirtualDisk.h"
#include "Structures.h"
#include "FatManager.h"

class DirectoryManager
{
    VirtualDisk &disk;
    description &boot_record;

    int32_t get_cluster_offset(int32_t cluster_idx);

    int32_t get_item_offset(int32_t cluster_idx, int32_t item_idx);

public:
    DirectoryManager(VirtualDisk &disk, description &boot_record);

    std::vector<directory_item> get_directory_content(int32_t cluster_idx);

    bool add_item(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t item_size, int32_t target_cluster);

    void format_new_directory(int32_t new_cluster, int32_t parent_cluster);

    bool directory_contains(int32_t cluster_idx, const std::string &name); // jestli obsahuje soubor s danym jmenem (mozno rozsirit obecne na cestu...)

    void clean_cluster(int32_t cluster);
};