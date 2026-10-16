#pragma once
#include <cstdint>

// specialni znacky ve FAT tabulce
const int32_t FAT_UNUSED = INT32_MAX - 1;
const int32_t FAT_FILE_END = INT32_MAX - 2;
const int32_t FAT_BAD_CLUSTER = INT32_MAX - 3;

struct description
{
    char signature[9];          // identifikace VFS, u nas login autora
    int32_t disk_size;          // celkova velikost VFS v bytech
    int32_t cluster_size;       // velikost clusteru v bytech
    int32_t cluster_count;      // pocet clusteru v datove oblasti
    int32_t fat_entries_count;  // pocet polozek v kazde FAT tabulce (obvykle stejny jako cluster_count)
    int32_t fat1_start_address; // adresa zacatku FAT1 tabulky
    int32_t fat2_start_address; // adresa zacatku FAT2 tabulky
    int32_t data_start_address; // adresa zacatku datove oblasti (korenovy adresar)
};

struct directory_item
{
    char item_name[13];    // 8+3 + '\0'
    bool isDirectory;      // TRUE = adresar, FALSE = soubor
    int32_t size;          // velikost souboru v bajtech, u adresáře hodnota 0
    int32_t start_cluster; // počáteční cluster položky
};