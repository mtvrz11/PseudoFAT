#include "VFS.h"
#include <cstring>

VFS::VFS(const std::string &host_file)
    : disk(host_file), is_formatted(false), fat_manager(disk, boot_record), dir_manager(disk, boot_record)
{
    boot_fs();
}

void VFS::boot_fs()
{
    is_formatted = false;

    if (fat_manager.load_from_disk())
        is_formatted = true;
}

VfsStatus VFS::format(int32_t total_bytes)
{
    disk.create_empty(total_bytes);

    std::strncpy(boot_record.signature, SIGNATURE, sizeof(boot_record.signature));
    boot_record.disk_size = total_bytes;
    boot_record.cluster_size = CLUSTER_SIZE;
    boot_record.cluster_count = (total_bytes - sizeof(description)) / (CLUSTER_SIZE + 2 * sizeof(FAT_UNUSED));
    boot_record.fat_entries_count = boot_record.cluster_count;
    boot_record.fat1_start_address = sizeof(description);
    boot_record.fat2_start_address = sizeof(description) + boot_record.fat_entries_count * sizeof(FAT_UNUSED);
    boot_record.data_start_address = sizeof(description) + 2 * boot_record.fat_entries_count * sizeof(FAT_UNUSED);

    disk.write_bytes(0, &boot_record, sizeof(description));

    fat_manager.format_new();
    dir_manager.format_new_directory(0, 0); // vrchol adresaroveho stromu - odkazuje sam na sebe

    is_formatted = true;
    current_cluster = 0;
    current_path = "/";

    return VfsStatus::OK;
}

bool VFS::is_ready()
{
    return is_formatted;
}