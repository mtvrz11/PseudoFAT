#include "FatFileSystem.h"
#include <iostream>
#include <cstring>
#include <vector>

constexpr int32_t cluster_size = 4096;

FatFileSystem::FatFileSystem(const std::string &host_file)
    : disk(host_file), is_formatted(false)
{
    boot_fs();
}

void FatFileSystem::boot_fs()
{
    is_formatted = false;

    if (!disk.is_open)
        return;

    // nedari se cist z disku
    if (!disk.read_bytes(0, &boot_record, sizeof(description)))
        return;

    // podpis neni spravny - neni to muj fs
    if (std::strncmp(boot_record.signature, SIGNATURE, sizeof(boot_record.signature)) != 0)
        return;

    fat_table.assign(boot_record.fat_entries_count, 0);
    size_t fat_bytes = boot_record.fat_entries_count * sizeof(FAT_UNUSED);

    if (!disk.read_bytes(boot_record.fat1_start_address, fat_table.data(), fat_bytes))
        return;

    is_formatted = true;
}

bool FatFileSystem::format(int32_t total_bytes)
{
    if (!disk.create_empty(total_bytes))
    {
        std::cout << "CANNOT CREATE FILE" << std::endl;
        return false;
    }

    std::strncpy(boot_record.signature, SIGNATURE, sizeof(boot_record.signature));
    boot_record.disk_size = total_bytes;
    boot_record.cluster_size = cluster_size;
    boot_record.cluster_count = (total_bytes - sizeof(description)) / (cluster_size + 2 * sizeof(FAT_UNUSED));
    boot_record.fat_entries_count = boot_record.cluster_count;
    boot_record.fat1_start_address = sizeof(description);
    boot_record.fat2_start_address = sizeof(description) + boot_record.fat_entries_count * sizeof(FAT_UNUSED);
    boot_record.data_start_address = sizeof(description) + 2 * boot_record.fat_entries_count * sizeof(FAT_UNUSED);

    fat_table.assign(boot_record.fat_entries_count, FAT_UNUSED);
    fat_table[0] = FAT_FILE_END; // na idx 0 je korenovy doresar

    directory_item root_item;
    memset(&root_item, 0, sizeof(root_item));
    root_item.isDirectory = true;
    root_item.size = 0;
    root_item.start_cluster = 0;
    std::strncpy(root_item.item_name, "..", 3);

    disk.write_bytes(0, &boot_record, sizeof(boot_record));
    disk.write_bytes(boot_record.fat1_start_address, fat_table.data(), fat_table.size() * sizeof(FAT_UNUSED));
    disk.write_bytes(boot_record.fat2_start_address, fat_table.data(), fat_table.size() * sizeof(FAT_UNUSED));
    disk.write_bytes(boot_record.data_start_address, &root_item, sizeof(root_item));

    is_formatted = true;
    std::cout << "OK" << std::endl;
    return true;
}

void FatFileSystem::pwd()
{
    if (!is_formatted)
    {
        std::cout << "DISC NOT FORMATTED" << std::endl;
        return;
    }

    std::cout << current_path << std::endl;
}