#pragma once
#include <vector>
#include <cstdint>
#include "../disk/VirtualDisk.h"
#include "Structures.h"

class FatManager
{
    VirtualDisk &disk;
    description &boot_record;
    std::vector<int32_t> fat_table;

    /// @brief Zapise celou FAT tabulku na disk na adresa obou FAT tabulek systemu.
    void flush_fat();

public:
    FatManager(VirtualDisk &disk, description &boot_record);

    /// @brief Nacte obsah FAT tabulky ze zavadece na disku pri spusteni souboroveho systemu.
    /// @return true, pokud byl podpis zpravny a podarilo se nacist data z FAT tabulky, jinak false
    bool load_from_disk();

    /// @brief Oznaci vsechny clustery FAT tabulky za nepouzite, jen na index 0 zapise hodnotu FAT_FILE_END pro root.
    void format_new();

    /// @brief Alokuje retezec clusteru dane delky ve FAT tabulce.
    /// @param count pocet clusteru, ktere jsou potreba alokovat
    /// @return alokovane pole clusteru (pri nenalezeni dostatecneho poctu volnych clusteru prazdny vektor)
    std::vector<int32_t> allocate_chain(int32_t count);

    /// @brief Uvolni alokovany retezec nastavenim hodnoty FAT_UNUSED na vsechny clustery v retezci.
    /// @param start_cluster pocatecni cluster retezce k uvolneni
    void free_chain(int32_t start_cluster);

    /// @brief Vrati vsechny clustery alokovaneho retezce od zadaneho pocatencniho clusteru do konce.
    /// @param start_cluster pocatecni cluster retezce
    /// @return pole clusteru v retezci, nebo prazdne pole v pripade predani neplatneho indexu do tabulky
    std::vector<int32_t> get_cluster_chain(int32_t start_cluster) const;

    /// @brief Prialokuje dalsi cluster do retezce o delce alespon jednoho clusteru.
    /// @param tail_cluster index clusteru, za ktery ma byt pripojen dalsi
    /// @return index nove alokovaneho clusteru, nebo FAT_BAD_CLUSTER v pripade, ze byl predan neplatny index do tabulky,
    ///         nebo -1 v pripade, ze se nepodarilo najit volny cluster
    int32_t append_cluster(int32_t tail_cluster);

    void free_cluster_in_chain(const std::vector<int32_t> chain, int32_t cluster);

    /// @brief Vytvori umele poskozeni souboroveho systemu tim, ze do FAT tabulky zanese spatnou hodnotu.
    /// @warning POSKODI SOUBOROVY SYSTEM !!!
    /// @param cluster index clusteru, na kterem ma byt poskozeni provedeno
    /// @param bad_value hodnota, ktera ma byt do tabulky zapsana na dany index
    void corrupt_fat_entry(int32_t cluster, int32_t bad_value);

    /// @brief Vrati index dalsihu clusteru nasledujiciho v retezci za predanym clusterem.
    /// @param cluster aktualni cluster jehoz nasledovnika chceme
    /// @return cislo clusteru, nebo FAT_BAD_CLUSTER v pripade, ze byl predan neplatny index do tabulky
    int32_t get_fat_entry_raw(int32_t cluster) const;

    /// @brief Vrati velikost FAT tabulky (pocet clusteru).
    /// @return velikost FAT tabulky
    int32_t get_fat_size() const;
};