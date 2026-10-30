#include "VFS.h"

#include <unordered_set>

VfsStatus VFS::bug(const std::string &path, BugType type)
{
    auto result = resolve_path(path);
    if (!result.is_valid || result.target_cluster == -1 || result.is_directory)
        return VfsStatus::FILE_NOT_FOUND;

    auto parent_chain = fat_manager.get_cluster_chain(result.parent_cluster);
    auto target_chain = fat_manager.get_cluster_chain(result.target_cluster);

    if (type == BugType::BAD_SIZE)
    {
        directory_item item;
        if (!dir_manager.find_item(parent_chain, result.target_name, item))
            return VfsStatus::FILE_NOT_FOUND;                                                   // nemelo by se stat kvuli predchozi kontrole...
        if (!dir_manager.corrupt_item_size(parent_chain, result.target_name, item.size * 1000)) // zvetseni velikosti polozky tisickrat
            return VfsStatus::FILE_NOT_FOUND;
    }
    else if (type == BugType::CYCLE)
    {
        if (target_chain.size() >= 1)
            fat_manager.corrupt_fat_entry(target_chain.back(), target_chain.front());
    }
    else if (type == BugType::OUT_OF_RANGE)
    {
        fat_manager.corrupt_fat_entry(result.target_cluster, fat_manager.get_fat_size() + 10); // cluster 10 polozek za hranici tabulky
    }
    return VfsStatus::OK;
}

CheckResult VFS::check()
{
    CheckResult res;
    std::vector<int32_t> dirs_to_visit = {0}; // zacinam od rootu
    std::unordered_set<int32_t> visited_dirs;

    while (!dirs_to_visit.empty())
    {
        int32_t current_dir = dirs_to_visit.back(); // vyzvednuti adresare
        dirs_to_visit.pop_back();

        if (visited_dirs.count(current_dir))
            continue;
        visited_dirs.insert(current_dir);

        auto chain = fat_manager.get_cluster_chain(current_dir);

        for (int32_t cluster : chain)
        {
            auto items = dir_manager.get_directory_content(cluster);
            for (const auto &item : items)
            {
                if (item.item_name[0] == '\0')
                    continue;

                std::string name(item.item_name);
                if (name == "." || name == "..")
                    continue;

                CheckItem bug;
                bug.is_directory = item.isDirectory;
                bug.name = name;

                int32_t curr_cluster = item.start_cluster;
                std::unordered_set<int32_t> seen_clusters;
                bool bug_detected = false;
                size_t chain_length = 0;

                while (curr_cluster != FAT_FILE_END && curr_cluster != FAT_UNUSED)
                {
                    if (curr_cluster < 0 || curr_cluster >= fat_manager.get_fat_size())
                    {
                        bug.bug = BugType::OUT_OF_RANGE;
                        bug_detected = true;
                        break;
                    }
                    if (seen_clusters.count(curr_cluster))
                    {
                        bug.bug = BugType::CYCLE;
                        bug_detected = true;
                        break;
                    }

                    chain_length++;
                    seen_clusters.insert(curr_cluster);
                    curr_cluster = fat_manager.get_fat_entry_raw(curr_cluster);
                }

                // pro soubory jeste zkontroluju nesoulad velikosti
                if (!bug_detected && !item.isDirectory)
                {
                    size_t expected_chain_length = (item.size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
                    if (expected_chain_length == 0)
                        expected_chain_length = 1;

                    if (expected_chain_length != chain_length)
                    {
                        bug.bug = BugType::BAD_SIZE;
                        bug_detected = true;
                    }
                }

                // pokud to je bezchybny adresar, pridam ho do seznamu na zkontrolovani
                if (!bug_detected && item.isDirectory)
                    dirs_to_visit.push_back(item.start_cluster);

                if (bug_detected)
                {
                    res.status = VfsStatus::BUG_DETECTED;
                    res.bugs.push_back(bug);
                }
            }
        }
    }

    return res;
}