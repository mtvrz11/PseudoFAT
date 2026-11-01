#include "VFS.h"
#include <cstring>

FatManager::FatManager(VirtualDisk &disk, description &boot_record)
    : disk(disk), boot_record(boot_record) {}

void FatManager::flush_fat()
{
    size_t fat_bytes = fat_table.size() * sizeof(FAT_UNUSED);
    disk.write_bytes(boot_record.fat1_start_address, fat_table.data(), fat_bytes);
    disk.write_bytes(boot_record.fat2_start_address, fat_table.data(), fat_bytes);
}

bool FatManager::load_from_disk()
{
    if (!disk.is_open)
        return false;

    disk.read_bytes(0, &boot_record, sizeof(description));

    // podpis neni spravny - neni to muj fs
    if (std::strncmp(boot_record.signature, SIGNATURE, sizeof(boot_record.signature)) != 0)
        return false;

    // vytvoreni prazdne fat tabulky
    fat_table.assign(boot_record.fat_entries_count, 0);
    size_t fat_bytes = boot_record.fat_entries_count * sizeof(FAT_UNUSED);

    // naplneni fat tabulky
    disk.read_bytes(boot_record.fat1_start_address, fat_table.data(), fat_bytes);
    return true;
}

void FatManager::format_new()
{
    fat_table.assign(boot_record.fat_entries_count, FAT_UNUSED);
    fat_table[0] = FAT_FILE_END; // na idx 0 je korenovy doresar
    flush_fat();
}

std::vector<int32_t> FatManager::allocate_chain(int32_t count)
{
    std::vector<int32_t> allocated;

    // nalezeni count prvnich volnych clusteru
    for (int i = 0; i < fat_table.size(); i++)
    {
        if (fat_table[i] == FAT_UNUSED)
            allocated.push_back(i);
        if (allocated.size() == count)
            break;
    }

    if (allocated.size() < count)
        return {}; // nebylo dost mista

    // zapsani retezce do FAT tabulky
    for (int i = 0; i < allocated.size(); i++)
    {
        int32_t current = allocated[i]; // posledni alokovany

        if (i == allocated.size() - 1)
            fat_table[current] = FAT_FILE_END; // uz mam kompletni retezec
        else
            fat_table[current] = allocated[i + 1]; // zapsani odkazu na nasledovnika
    }

    flush_fat();
    return allocated;
}

void FatManager::free_chain(int32_t start_cluster)
{
    if (start_cluster < 0 || start_cluster >= fat_table.size())
        return; // neni co uovolnovat

    int32_t current = start_cluster;
    bool modified = false;

    while (current != FAT_FILE_END && current != FAT_UNUSED)
    {
        int32_t next = fat_table[current]; // posun na dalsi
        fat_table[current] = FAT_UNUSED;   // uvolneni
        modified = true;
        current = next;
    }

    if (modified)
        flush_fat();
}

std::vector<int32_t> FatManager::get_cluster_chain(int32_t start_cluster) const
{
    std::vector<int32_t> chain;
    if (start_cluster < 0 || start_cluster >= fat_table.size())
        return chain;

    int32_t current = start_cluster;
    while (current != FAT_FILE_END && current != FAT_UNUSED)
    {
        if (chain.size() >= fat_table.size())
            break;                    // ochrana pred poskozenim cyklem ve fat tabulce, aby mi nezamrzla RAM
        chain.push_back(current);     // pridani do pole
        current = fat_table[current]; // a posun na dalsi
    }
    return chain;
}

int32_t FatManager::append_cluster(int32_t tail_cluster)
{
    if (tail_cluster < 0 || tail_cluster >= fat_table.size())
        return FAT_BAD_CLUSTER; // neplatny index od tabulky

    int32_t new_cluster = -1;

    // hledani volneho clusteru od zacatku tabulky do nalezeni nebo konce tabulky
    for (int i = 0; i < fat_table.size(); i++)
    {
        if (fat_table[i] == FAT_UNUSED)
        {
            new_cluster = i;
            break;
        }
    }

    // zadny volny nenalezen
    if (new_cluster == -1)
        return -1;

    // pripojeni do retezce
    fat_table[tail_cluster] = new_cluster;
    fat_table[new_cluster] = FAT_FILE_END;

    flush_fat();

    return new_cluster;
}

void FatManager::free_cluster_in_chain(const std::vector<int32_t> chain, int32_t cluster)
{
    // prazdne retezce nebo retezce o jedinem prvku neuvolnuji, startovni cluster nikdy neuvolnuji
    // (je totiz zanesen v adresarove polozce)
    if (chain.empty() || chain.size() <= 1)
        return;

    // pokud uvolnuji cluster na konci retezce, prepisu jen predposledni cluster na konec a uvolnim
    if (cluster == chain.back())
        fat_table[chain[chain.size() - 2]] = FAT_FILE_END;

    // cilovy cluster je uprostred retezce
    else
    {
        for (size_t i = 1; i < chain.size() - 1; i++)
        {
            if (chain[i] == cluster)
            {
                fat_table[chain[i - 1]] = chain[i + 1]; // preskocime prazdny cluster
                break;
            }
        }
    }

    fat_table[cluster] = FAT_UNUSED; // uvolneni clusteru
    flush_fat();
}

void FatManager::corrupt_fat_entry(int32_t cluster, int32_t bad_value)
{
    if (cluster >= 0 && cluster < fat_table.size())
    {
        fat_table[cluster] = bad_value;
        flush_fat();
    }
}

int32_t FatManager::get_fat_entry_raw(int32_t cluster) const
{
    if (cluster >= 0 && cluster < fat_table.size())
        return fat_table[cluster];
    else
        return FAT_BAD_CLUSTER;
}

int32_t FatManager::get_fat_size() const
{
    return fat_table.size();
}