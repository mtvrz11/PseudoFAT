#include "DirectoryManager.h"
#include <cstring>
#include <cstdio>
#include <iostream>

DirectoryManager::DirectoryManager(VirtualDisk &disk, description &boot_record)
    : disk(disk), boot_record(boot_record) {}

int32_t DirectoryManager::get_cluster_offset(int32_t cluster_idx)
{
    return boot_record.data_start_address + (cluster_idx * boot_record.cluster_size);
}

int32_t DirectoryManager::get_item_offset(int32_t cluster_idx, int32_t item_idx)
{
    return get_cluster_offset(cluster_idx) + item_idx * sizeof(directory_item);
}

std::vector<directory_item> DirectoryManager::get_directory_content(int32_t cluster_idx)
{
    int32_t max_items_count = CLUSTER_SIZE / sizeof(directory_item);
    std::vector<directory_item> items(max_items_count); // vytvoreni noveho prazdneho pole polozek

    disk.read_bytes(get_cluster_offset(cluster_idx), items.data(), CLUSTER_SIZE);

    return items;
}

bool DirectoryManager::add_item(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t item_size, int32_t target_cluster)
{
    auto items = get_directory_content(parent_cluster);
    int32_t free_slot = -1;

    // nalezeni prvniho volneho slotu v rodicovskem clusteru
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i].item_name[0] == '\0')
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
        return false;

    // vytvoreni adresarove polozky
    directory_item item;
    memset(&item, 0, sizeof(item));
    item.isDirectory = is_dir;
    item.size = item_size;
    item.start_cluster = target_cluster;
    std::strncpy(item.item_name, name.data(), sizeof(item.item_name));

    // zapis na disk
    return disk.write_bytes(get_item_offset(parent_cluster, free_slot), &item, sizeof(directory_item));
}

void DirectoryManager::format_new_directory(int32_t new_cluster, int32_t parent_cluster)
{
    int32_t max_items_count = CLUSTER_SIZE / sizeof(directory_item);
    std::vector<directory_item> content(max_items_count, {0});
    disk.write_bytes(get_cluster_offset(new_cluster), content.data(), CLUSTER_SIZE); // zapis noveho clusteru na disk
    add_item(new_cluster, "..", true, 0, parent_cluster);                            // pridani odkazu na rodice
}

bool DirectoryManager::directory_contains(int32_t cluster_idx, const std::string &name)
{
    auto items = get_directory_content(cluster_idx);

    for (int i = 0; i < items.size(); i++)
    {
        if (std::strncmp(items[i].item_name, name.data(), sizeof(directory_item::item_name)) == 0)
            return true;
    }

    return false;
}

void DirectoryManager::clean_cluster(int32_t cluster)
{
    std::vector<char> buffer(CLUSTER_SIZE, {0});
    disk.write_bytes(get_cluster_offset(cluster), buffer.data(), CLUSTER_SIZE);
}