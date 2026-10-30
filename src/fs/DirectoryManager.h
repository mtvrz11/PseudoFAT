#pragma once
#include <vector>
#include <string>
#include "../disk/VirtualDisk.h"
#include "Structures.h"

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

    void remove_item(const std::vector<int32_t> &parent_chain, std::string &name);

    void format_new_directory(int32_t new_cluster, int32_t parent_cluster);

    bool find_item(const std::vector<int32_t> &parent_chain, const std::string &name, directory_item &out_item);

    void clean_cluster(int32_t cluster);

    bool is_directory_empty(const std::vector<int32_t> &chain);

    bool corrupt_item_size(const std::vector<int32_t> &parent_chain, const std::string &name, int32_t bad_size);
};