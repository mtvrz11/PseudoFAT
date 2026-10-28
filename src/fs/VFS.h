#pragma once
#include "../disk/VirtualDisk.h"
#include "DirectoryManager.h"
#include "FatManager.h"
#include "Structures.h"
#include "FsTypes.h"

#include <string>
#include <vector>

class VFS
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
    std::vector<std::string> split_path(const std::string &path);
    PathResult resolve_path(const std::string &path);
    std::string build_absolute_path(const std::string &path);

public:
    VFS(const std::string &host_file);

    bool is_ready();

    VfsStatus format(int32_t total_bytes);
    std::string pwd();
    LsResult ls(const std::string &path = "");
    VfsStatus mkdir(const std::string &path);
    VfsStatus cd(const std::string &path);
    VfsStatus rmdir(const std::string &path);
};