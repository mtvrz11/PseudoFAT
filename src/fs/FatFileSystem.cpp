#include "FatFileSystem.h"
#include <iostream>
#include <cstring>
#include <vector>

FatFileSystem::FatFileSystem(const std::string &host_file)
    : disk(host_file), is_formatted(false), fat_manager(disk, boot_record), dir_manager(disk, boot_record)
{
    boot_fs();
}

void FatFileSystem::boot_fs()
{
    is_formatted = false;

    if (fat_manager.load_from_disk())
        is_formatted = true;
}

bool FatFileSystem::add_to_directory(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t size, int32_t target_cluster)
{
    auto chain = fat_manager.get_cluster_chain(parent_cluster);

    for (int32_t cluster : chain)
    {
        if (dir_manager.add_item(parent_cluster, name, is_dir, size, target_cluster))
            return true; // podarilo se zapsat
    }

    // vsechny clustery plne - potrebuji pridat dalsi
    int32_t tail_cluster = chain.back();
    int32_t new_cluster = fat_manager.append_cluster(tail_cluster);

    if (new_cluster == -1)
    {
        std::cout << "DISK FULL" << std::endl;
        return false;
    }

    dir_manager.clean_cluster(new_cluster);
    return dir_manager.add_item(new_cluster, name, is_dir, size, target_cluster);
}

bool FatFileSystem::format(int32_t total_bytes)
{
    if (!disk.create_empty(total_bytes))
    {
        std::cout << "CANNOT CREATE FILE" << std::endl;
        return false;
    }

    // NAPLNENI BOOT_RECORD NECHAT ZDE ???
    std::strncpy(boot_record.signature, SIGNATURE, sizeof(boot_record.signature));
    boot_record.disk_size = total_bytes;
    boot_record.cluster_size = CLUSTER_SIZE;
    boot_record.cluster_count = (total_bytes - sizeof(description)) / (CLUSTER_SIZE + 2 * sizeof(FAT_UNUSED));
    boot_record.fat_entries_count = boot_record.cluster_count;
    boot_record.fat1_start_address = sizeof(description);
    boot_record.fat2_start_address = sizeof(description) + boot_record.fat_entries_count * sizeof(FAT_UNUSED);
    boot_record.data_start_address = sizeof(description) + 2 * boot_record.fat_entries_count * sizeof(FAT_UNUSED);
    if (!disk.write_bytes(0, &boot_record, sizeof(description)))
        return false;

    fat_manager.format_new();
    dir_manager.format_new_directory(0, 0); // vrchol adresaroveho stromu - odkazuje sam na sebe

    is_formatted = true;
    std::cout << "OK" << std::endl;
    return true;
}

void FatFileSystem::pwd()
{
    if (!is_formatted)
    {
        std::cout << "DISK NOT FORMATTED" << std::endl;
        return;
    }

    std::cout << current_path << std::endl;
}

void FatFileSystem::ls(const std::string &path)
{
    if (!is_formatted)
    {
        std::cout << "DISK NOT FORMATTED" << std::endl;
        return;
    }

    auto items = dir_manager.get_directory_content(current_cluster); // predelat na cestu !!!

    for (int i = 0; i < items.size(); i++)
    {
        // polozka nema jmeno -> neexistuje
        if (items[i].item_name[0] == '\0')
            continue;

        if (items[i].isDirectory == true)
            std::cout << "DIR: " << items[i].item_name << std::endl;
        else
            std::cout << "FILE: " << items[i].item_name << std::endl;
    }
}

void FatFileSystem::mkdir(const std::string &name)
{
    if (!is_formatted)
    {
        std::cout << "DISK NOT FORMATTED" << std::endl;
        return;
    }

    if (dir_manager.directory_contains(current_cluster, name))
    {
        std::cout << "EXIST" << std::endl;
        return;
    }

    auto chain = fat_manager.allocate_chain(1);
    if (chain.empty())
    {
        std::cout << "DISK FULL" << std::endl;
        return;
    }

    auto new_cluster = chain[0];

    if (!dir_manager.add_item(current_cluster, name, true, 0, new_cluster))
    {
        fat_manager.free_chain(new_cluster);
        return;
    }

    dir_manager.format_new_directory(new_cluster, current_cluster);

    std::cout << "OK" << std::endl;
}