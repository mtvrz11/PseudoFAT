#include "VFS.h"
#include <vector>

int32_t VFS::get_cluster_offset(int32_t cluster_idx)
{
    return boot_record.data_start_address + (cluster_idx * boot_record.cluster_size);
}

bool VFS::add_to_directory(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t size, int32_t target_cluster)
{
    auto chain = fat_manager.get_cluster_chain(parent_cluster);

    for (int32_t cluster : chain)
    {
        if (dir_manager.add_item(cluster, name, is_dir, size, target_cluster))
            return true; // podarilo se zapsat
    }

    // vsechny clustery plne - potrebuji pridat dalsi
    int32_t tail_cluster = chain.back();
    int32_t new_cluster = fat_manager.append_cluster(tail_cluster);

    if (new_cluster == -1)
        return false;

    dir_manager.clean_cluster(new_cluster);
    return dir_manager.add_item(new_cluster, name, is_dir, size, target_cluster);
}

LsResult VFS::ls(const std::string &path)
{
    auto res_path = resolve_path(path);
    LsResult result;
    if (!res_path.is_valid || !res_path.is_directory)
    {
        result.status = VfsStatus::PATH_NOT_FOUND;
        return result;
    }

    auto chain = fat_manager.get_cluster_chain(res_path.target_cluster);
    for (int32_t cluster : chain)
    {
        auto items = dir_manager.get_directory_content(cluster);
        for (int i = 0; i < items.size(); i++)
        {
            // polozka nema jmeno -> neexistuje
            if (items[i].item_name[0] == '\0')
                continue;
            LsItem item;
            item.name = items[i].item_name;
            item.is_directory = items[i].isDirectory;
            result.items.push_back(item);
        }
    }

    result.status = VfsStatus::OK;
    return result;
}

VfsStatus VFS::mkdir(const std::string &path)
{
    auto result = resolve_path(path);
    if (!result.is_valid)
        return VfsStatus::PATH_NOT_FOUND;

    // polozka byla nalezena -> uz existuje
    if (result.target_cluster != -1)
        return VfsStatus::ALREADY_EXIST;

    // cesta validni a slozka jeste neexistuje
    auto chain = fat_manager.allocate_chain(1);
    if (chain.empty())
        return VfsStatus::DISK_FULL;

    auto new_cluster = chain[0];
    if (!add_to_directory(result.parent_cluster, result.target_name, true, 0, new_cluster))
    {
        fat_manager.free_chain(new_cluster);
        return VfsStatus::DISK_FULL;
    }

    dir_manager.format_new_directory(new_cluster, result.parent_cluster);
    return VfsStatus::OK;
}

VfsStatus VFS::rmdir(const std::string &path)
{
    auto result = resolve_path(path);
    if (!result.is_valid || result.target_cluster == -1 || !result.is_directory)
        return VfsStatus::PATH_NOT_FOUND;

    // nesmim smazat aktualni slozku ani root
    if (result.target_cluster == 0 || result.target_cluster == current_cluster)
        return VfsStatus::PATH_NOT_FOUND;

    auto target_chain = fat_manager.get_cluster_chain(result.target_cluster);
    if (!dir_manager.is_directory_empty(target_chain))
        return VfsStatus::NOT_EMPTY;

    auto parent_chain = fat_manager.get_cluster_chain(result.parent_cluster);
    if (!dir_manager.remove_item(parent_chain, result.target_name))
        return VfsStatus::PATH_NOT_FOUND; // polozka nebyla nalezena - nemelo by k tomu dojit kvuli predchozi kontrole...

    fat_manager.free_chain(result.target_cluster); // vymazani zaznamu z fat tabulky
    return VfsStatus::OK;
}

VfsStatus VFS::cd(const std::string &path)
{
    auto result = resolve_path(path);

    if (!result.is_valid || !result.is_directory)
        return VfsStatus::PATH_NOT_FOUND;

    current_cluster = result.target_cluster;
    current_path = build_absolute_path(path);
    return VfsStatus::OK;
}

InfoResult VFS::info(const std::string &path)
{
    InfoResult result;

    auto res_path = resolve_path(path);
    if (!res_path.is_valid || res_path.target_cluster == -1)
    {
        result.status = VfsStatus::FILE_NOT_FOUND;
        return result;
    }

    auto parent_chain = fat_manager.get_cluster_chain(res_path.parent_cluster);
    directory_item item;
    dir_manager.find_item(parent_chain, res_path.target_name, item);

    result.status = VfsStatus::OK;
    result.item.name = item.item_name;
    result.item.is_directory = item.isDirectory;
    result.item.size = item.size;
    result.item.cluster_chain = fat_manager.get_cluster_chain(res_path.target_cluster);

    return result;
}