#include "FatManager.h"
#include <cstring>

FatManager::FatManager(VirtualDisk &disk, description &boot_record)
    : disk(disk), boot_record(boot_record) {}

bool FatManager::load_from_disk()
{
    if (!disk.is_open)
        return false;

    // nedari se cist z disku
    if (!disk.read_bytes(0, &boot_record, sizeof(description)))
        return false;

    // podpis neni spravny - neni to muj fs
    if (std::strncmp(boot_record.signature, SIGNATURE, sizeof(boot_record.signature)) != 0)
        return false;

    fat_table.assign(boot_record.fat_entries_count, 0);
    size_t fat_bytes = boot_record.fat_entries_count * sizeof(FAT_UNUSED);

    if (!disk.read_bytes(boot_record.fat1_start_address, fat_table.data(), fat_bytes))
        return false;

    return true;
}

void FatManager::format_new()
{
    fat_table.assign(boot_record.fat_entries_count, FAT_UNUSED);
    fat_table[0] = FAT_FILE_END; // na idx 0 je korenovy doresar

    disk.write_bytes(boot_record.fat1_start_address, fat_table.data(), fat_table.size() * sizeof(FAT_UNUSED));
    disk.write_bytes(boot_record.fat2_start_address, fat_table.data(), fat_table.size() * sizeof(FAT_UNUSED));
}

std::vector<int32_t> FatManager::allocate_chain(int32_t count)
{
    std::vector<int32_t> allocated;

    for (int i = 0; i < fat_table.size(); i++)
    {
        if (fat_table[i] == FAT_UNUSED)
            allocated.push_back(i);
        if (allocated.size() == count)
            break;
    }

    if (allocated.size() < count)
        return {}; // nabylo dost mista

    for (int i = 0; i < allocated.size(); i++)
    {
        int32_t current = allocated[i];

        if (i == allocated.size() - 1)
            fat_table[current] = FAT_FILE_END;
        else
            fat_table[current] = allocated[i + 1];

        int32_t val = fat_table[current];
        int32_t offset = current * sizeof(FAT_UNUSED);
        disk.write_bytes(boot_record.fat1_start_address + offset, &val, sizeof(val));
        disk.write_bytes(boot_record.fat2_start_address + offset, &val, sizeof(val));
    }

    // bylo by lepsi volat nekolikrat pridani jen jedne polozky, nebo radsi zkopirovat celou fat tabulku, ale jen jednou ???
    // disk.write_bytes(boot_record.fat1_start_address, fat_table.data(), fat_table.size() * sizeof(FAT_UNUSED));
    // disk.write_bytes(boot_record.fat2_start_address, fat_table.data(), fat_table.size() * sizeof(FAT_UNUSED));

    return allocated;
}

void FatManager::free_chain(int32_t start_cluster)
{
    if (start_cluster < 0 || start_cluster >= fat_table.size())
        return;

    int32_t current = start_cluster;

    while (current != FAT_FILE_END && current != FAT_UNUSED)
    {
        int32_t next = fat_table[current];
        fat_table[current] = FAT_UNUSED;

        // opet nekdy pripadne prepsat na jedine volani zmeny...
        int32_t val = FAT_UNUSED;
        int32_t offset = current * sizeof(FAT_UNUSED);
        disk.write_bytes(boot_record.fat1_start_address + offset, &val, sizeof(FAT_UNUSED));
        disk.write_bytes(boot_record.fat2_start_address + offset, &val, sizeof(FAT_UNUSED));

        current = next;
    }
}

std::vector<int32_t> FatManager::get_cluster_chain(int32_t start_cluster)
{
    std::vector<int32_t> chain;
    if (start_cluster < 0 || start_cluster >= fat_table.size())
        return chain;

    int32_t current = start_cluster;
    while (current != FAT_FILE_END && current != FAT_UNUSED)
    {
        chain.push_back(current);
        current = fat_table[current];
    }
    return chain;
}

int32_t FatManager::append_cluster(int32_t tail_cluster)
{
    if (tail_cluster < 0 || tail_cluster >= fat_table.size())
        return FAT_BAD_CLUSTER;

    int32_t new_cluster = -1;

    for (int i = 0; i < fat_table.size(); i++)
    {
        if (fat_table[i] == FAT_UNUSED)
        {
            new_cluster = i;
            break;
        }
    }

    if (new_cluster == -1)
        return -1;

    fat_table[tail_cluster] = new_cluster;
    fat_table[new_cluster] = FAT_FILE_END;

    int32_t offset_tail = tail_cluster * sizeof(FAT_UNUSED);
    disk.write_bytes(boot_record.fat1_start_address + offset_tail, &new_cluster, sizeof(new_cluster));
    disk.write_bytes(boot_record.fat2_start_address + offset_tail, &new_cluster, sizeof(new_cluster));

    int32_t offset_new = new_cluster * sizeof(FAT_UNUSED);
    int32_t val = FAT_FILE_END;
    disk.write_bytes(boot_record.fat1_start_address + offset_new, &val, sizeof(val));
    disk.write_bytes(boot_record.fat2_start_address + offset_new, &val, sizeof(val));
}