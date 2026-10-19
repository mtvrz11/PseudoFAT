#pragma once
#include "../disk/VirtualDisk.h"
#include "DirectoryManager.h"
#include "FatManager.h"
#include "Structures.h"
#include <string>
#include <vector>

class FatFileSystem
{
    VirtualDisk disk;
    description boot_record;
    FatManager fat_manager;
    DirectoryManager dir_manager;
    bool is_formatted;
    std::string current_path = "/";
    int32_t current_cluster = 0;

    void boot_fs();

    bool add_to_directory(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t size, int32_t target_cluster);

public:
    FatFileSystem(const std::string &host_file);

    bool format(int32_t total_bytes);
    void pwd();
    void ls(const std::string &path = "");
    void mkdir(const std::string &name);
};