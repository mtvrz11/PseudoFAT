#pragma once
#include <vector>
#include <string>
#include "../disk/VirtualDisk.h"
#include "Structures.h"

class DirectoryManager
{
    VirtualDisk &disk;
    description &boot_record;

    /// @brief Spocita adresu daneho clusteru na disku.
    /// @param cluster_idx index clusteru
    /// @return adresa na disku
    int32_t get_cluster_offset(int32_t cluster_idx);

    /// @brief Spocita adresu na disku dane polozky v danem clusteru.
    /// @param cluster_idx index rodicovskeho clusteru
    /// @param item_idx index polozky v clusteru
    /// @return adresa polozky na disku
    int32_t get_item_offset(int32_t cluster_idx, int32_t item_idx);

public:
    DirectoryManager(VirtualDisk &disk, description &boot_record);

    /// @brief Projde jeden cluster adresare a vrati jeho poloczky jako pole struktur directory_item.
    /// @param cluster_idx index clusteru k projdeni
    /// @return pole polozek ulozenych na dane clusteru
    std::vector<directory_item> get_directory_content(int32_t cluster_idx);

    /// @brief Prida do adresare novou polozku.
    /// @param parent_cluster cluster adresare, ve kterem bude polozka ulozena
    /// @param name jmeno polozky
    /// @param is_dir je to adresar nebo soubor?
    /// @param item_size velikost polozky (pokud je to adresar, nastavit na 0)
    /// @param target_cluster cislo clusteru, ve kterem budou ulozena data polozky
    /// @return true pokud se operace podarila, nebo false, pokud v zadanem clusteru
    ///         rodicovskeho adresare uz nebylo misto pro pridani dalsi polozky
    bool add_item(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t item_size, int32_t target_cluster);

    /// @brief Odebere polozku z daneho adresare.
    /// @param parent_chain retezec clusteru daneho adresare
    /// @param name jmeno polozky k odstraneni
    void remove_item(const std::vector<int32_t> &parent_chain, const std::string &name);

    /// @brief Vycisti cilovy cluster a prida prvni dve polozky .. a . (odkaz na rodice a na sebe sama).
    /// @param new_cluster cluster alokovany pro vznik noveho adresare
    /// @param parent_cluster cluster, rodicovskeho adresare, ve kterem bude ten novy ulozen
    void format_new_directory(int32_t new_cluster, int32_t parent_cluster);

    /// @brief Projde vsechny clustery adresare a hleda v nich danou polozku. Pri nalezeni vrati referenci na ni.
    /// @param parent_chain retezec clusteru rodicovskeho adresare hledane polozky
    /// @param name jmeno hledane polozky
    /// @param out_item misto pro ulozeni nalezene polozky (v pripade nenalezeni se nijak nemodifikuje)
    /// @return true pokud byla polozka nalezena a vrazena, jinak false
    bool find_item(const std::vector<int32_t> &parent_chain, const std::string &name, directory_item &out_item);

    /// @brief Vycisti dany cluster v datove oblasti (prepise jej na disku nulami).
    /// @param cluster cluster k vycisteni
    void clean_cluster(int32_t cluster);

    /// @brief Prejde uloziste a zkouma, zda obsahuje nejake polozky (polozky .. a . ignoruje).
    /// @param chain retezec clusteru adresare k prohledani
    /// @return true, pokud je adresar prazdny, jinak false
    bool is_directory_empty(const std::vector<int32_t> &chain);

    /// @brief Vytvori umele poskozeni souboroveho systemu tim, ze prepise velikost adresarove polozky na jinou.
    /// @warning POSKODI SOUBOROVY SYSTEM !!!
    /// @param parent_chain retezec clusteru adresare, ve kterem se nachazi polozka k poskozeni
    /// @param name jmeno polozky k poskozeni
    /// @param bad_size nova velikost polozky, ktera ji ma byt prirazena
    /// @return true, pokud se akce podarila, nebo false v pripade, ze polozka nebyla nalezena
    bool corrupt_item_size(const std::vector<int32_t> &parent_chain, const std::string &name, int32_t bad_size);
};