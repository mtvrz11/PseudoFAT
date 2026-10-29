#include "VFS.h"

#include <fstream>

void VFS::write_stream_to_chain(std::ifstream &hostfile, const std::vector<int32_t> chain, int32_t file_size)
{
    std::vector<char> buffer(boot_record.cluster_size, 0);
    int32_t bytes_left = file_size;

    for (int32_t cluster : chain)
    {
        int32_t chunk_size = std::min(bytes_left, boot_record.cluster_size);

        if (chunk_size == 0)                            // soubor nulove vleikosti, ktery chci zapsat do jednoho clusteru
            std::fill(buffer.begin(), buffer.end(), 0); // vycisteni clusteru od nahodnych dat

        hostfile.read(buffer.data(), chunk_size);

        if (chunk_size < boot_record.cluster_size)
            std::fill(buffer.begin() + chunk_size, buffer.end(), 0); // premazani pripadneho odpadu za koncem filu

        bytes_left -= chunk_size;

        disk.write_bytes(get_cluster_offset(cluster), buffer.data(), boot_record.cluster_size);
    }
}

void VFS::read_chain_to_stream(std::ofstream &hostfile, const std::vector<int32_t> chain, int32_t file_size)
{
    std::vector<char> buffer(boot_record.cluster_size, 0);
    int32_t bytes_left = file_size;
    for (int32_t cluster : chain)
    {
        if (bytes_left <= 0)
            break;

        int32_t chunk_size = std::min(bytes_left, boot_record.cluster_size);

        disk.read_bytes(get_cluster_offset(cluster), buffer.data(), boot_record.cluster_size);
        hostfile.write(buffer.data(), chunk_size);
        bytes_left -= chunk_size;
    }
}

VfsStatus VFS::incp(const std::string &from, const std::string &to)
{
    std::ifstream host_file(from, std::ios::binary | std::ios::ate); // binarni s kurzorem na konci filu
    if (!host_file.is_open())
        return VfsStatus::FILE_NOT_FOUND;

    int32_t file_size = static_cast<int32_t>(host_file.tellg());
    host_file.seekg(0, std::ios::beg);

    auto result = resolve_path(to);
    if (!result.is_valid)
        return VfsStatus::PATH_NOT_FOUND;

    // chceme, aby ten cil neexisttoval
    if (result.target_cluster != -1)
        return VfsStatus::ALREADY_EXIST;

    int32_t clusters_needed = (file_size + boot_record.cluster_size - 1) / boot_record.cluster_size;
    if (clusters_needed == 0)
        clusters_needed = 1; // prazdny soubor dostane jeden cluster

    auto chain = fat_manager.allocate_chain(clusters_needed);
    if (chain.empty())
        return VfsStatus::DISK_FULL;

    write_stream_to_chain(host_file, chain, file_size);

    if (!add_to_directory(result.parent_cluster, result.target_name, false, file_size, chain[0]))
    {
        fat_manager.free_chain(chain[0]);
        return VfsStatus::DISK_FULL;
    }

    return VfsStatus::OK;
}

VfsStatus VFS::outcp(const std::string &from, const std::string &to)
{
    auto result = resolve_path(from);
    if (!result.is_valid || result.target_cluster == -1 || result.is_directory)
        return VfsStatus::FILE_NOT_FOUND;

    // potrebuji ziskat konkretni adresarovou polozku pro odecteni velikosti
    auto parent_chain = fat_manager.get_cluster_chain(result.parent_cluster);
    directory_item item;
    if (!dir_manager.find_item(parent_chain, result.target_name, item))
        return VfsStatus::PATH_NOT_FOUND;

    int32_t file_size = item.size;

    std::ofstream host_file(to, std::ios::binary);
    if (!host_file.is_open())
        return VfsStatus::FILE_NOT_FOUND;

    auto chain = fat_manager.get_cluster_chain(result.target_cluster);
    read_chain_to_stream(host_file, chain, file_size);

    return VfsStatus::OK;
}

VfsStatus VFS::cp(const std::string &from, const std::string &to)
{
    auto path_from = resolve_path(from);
    if (!path_from.is_valid || path_from.target_cluster == -1 || path_from.is_directory)
        return VfsStatus::FILE_NOT_FOUND;

    auto src_parent_chain = fat_manager.get_cluster_chain(path_from.parent_cluster);

    directory_item item;
    dir_manager.find_item(src_parent_chain, path_from.target_name, item);
    int32_t file_size = item.size;

    auto src_chain = fat_manager.get_cluster_chain(path_from.target_cluster);

    auto path_to = resolve_path(to);
    if (!path_to.is_valid)
        return VfsStatus::PATH_NOT_FOUND;
    if (path_to.target_cluster != -1)
        return VfsStatus::ALREADY_EXIST; // mam soubor prepisovat nebo ne ??? stejne tak u incp a mv !!!

    auto dest_chain = fat_manager.allocate_chain(src_chain.size());

    // fyzicke nakopirovani souboru na dalsi misto na disku
    std::vector<char> buffer(CLUSTER_SIZE);
    for (size_t i = 0; i < src_chain.size(); i++)
    {
        disk.read_bytes(get_cluster_offset(src_chain[i]), buffer.data(), CLUSTER_SIZE);
        disk.write_bytes(get_cluster_offset(dest_chain[i]), buffer.data(), CLUSTER_SIZE);
    }

    add_to_directory(path_to.parent_cluster, path_to.target_name, false, file_size, dest_chain[0]);

    return VfsStatus::OK;
}

// jak chapat to mv -ma se zadavat cilova cesta vcetne nazvu souboru, nebo jen slozka ??? Asi vcetne souboru...
VfsStatus VFS::mv(const std::string &from, const std::string &to)
{
    auto path_from = resolve_path(from);
    if (!path_from.is_valid || path_from.target_cluster == -1 || path_from.is_directory)
        return VfsStatus::FILE_NOT_FOUND;

    auto path_to = resolve_path(to);
    if (!path_to.is_valid)
        return VfsStatus::PATH_NOT_FOUND;
    if (path_to.target_cluster != -1)
        return VfsStatus::ALREADY_EXIST;

    auto src_parent_chain = fat_manager.get_cluster_chain(path_from.parent_cluster);
    directory_item item;
    dir_manager.find_item(src_parent_chain, path_from.target_name, item);
    add_to_directory(path_to.parent_cluster, path_to.target_name, false, item.size, item.start_cluster);
    dir_manager.remove_item(src_parent_chain, path_from.target_name);

    return VfsStatus::OK;
}

VfsStatus VFS::rm(const std::string &path)
{
    auto result = resolve_path(path);
    if (!result.is_valid || result.target_cluster == -1 || result.is_directory)
        return VfsStatus::FILE_NOT_FOUND;

    auto parent_chain = fat_manager.get_cluster_chain(result.parent_cluster);
    dir_manager.remove_item(parent_chain, result.target_name);
    fat_manager.free_chain(result.target_cluster);

    return VfsStatus::OK;
}