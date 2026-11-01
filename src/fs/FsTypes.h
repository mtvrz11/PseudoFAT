#pragma once

#include <string>
#include <vector>

/// @brief Vysledky vyhodnoceni prikazu.
enum class VfsStatus
{
    OK,
    DISK_NOT_FORMATTED,
    PATH_NOT_FOUND,
    ALREADY_EXIST,
    DISK_FULL,
    FILE_NOT_FOUND,
    NOT_EMPTY,
    CANNOT_CREATE_FILE,
    BUG_DETECTED
};

/// @brief Struktura pro logickou reprezentaci adresaarove cesty.
/// @note Cesta muze byt validni a zaroven cilovy cluster muze zustat -1, pokud na predposledim miste
///       cesty je existujici adresar a na poslednim miste je jmeno polozky, ktera ma teprve byt vytvorena.
struct PathResult
{
    bool is_valid = false;       // je cesta validni
    int32_t parent_cluster = -1; // pocatecni cluster rodicovskeho adresare, ve kterem je cilova polozka ulozena
    int32_t target_cluster = -1; // pocatecni cluster retezce, na kterem je cilova polozka
    std::string target_name;     // jmeno cilove polozky
    bool is_directory = false;   // je to adresar nebo soubor?
};

/// @brief Popis adresarove polozky v rozsahu, ktery staci pro prikaz ls.
struct LsItem
{
    std::string name;  // jmeno polozky
    bool is_directory; // je to adresar nebo soubor?
};

/// @brief Typ pouzivany jako navratovy typ prikazu ls.
struct LsResult
{
    VfsStatus status;          // VfsStatus::OK, pokud se prikaz provedl spravne, jinak jiny status
    std::vector<LsItem> items; // v pripade spravneho provedeni prikazu pole popisu vsech polozek adresare polozek
};

/// @brief Popis adresarove polozky v rozsahu, ktery staci pro prikaz info.
struct InfoItem
{
    std::string name;                   // jmeno polozky
    bool is_directory;                  // je to adresar?
    int32_t size;                       // velikost polozky
    std::vector<int32_t> cluster_chain; // pole clusteru, ktere ma polozka alokovane
};

/// @brief Typ pouzivany jako navratovy typ prikazu info.
struct InfoResult
{
    VfsStatus status; // VfsStatus::OK, pokud se prikaz provedl spravne, jinak jiny status
    InfoItem item;    // v pripade spravneho provedeni prikazu pole popisu vsech polozek adresare polozek
};

/// @brief Mozne typy chyby pro prikazy bug a check.
enum class BugType
{
    OUT_OF_RANGE = 1, // ve fat tabulce je zanesen index, ktery smeruje mimo rozsah tabulky
    CYCLE = 2,        // ve fat tabulce je zanesen vyklus
    BAD_SIZE = 3      // velikost zasana v adresarove polozce neodpovida poctu alokovanych clusteru
};

/// @brief Popis adresarove polozky v rozsahu, ktery staci pro prikaz check.
struct CheckItem
{
    bool is_directory; // je to adresar?
    std::string name;  // nazev polozky
    BugType bug;       // typ chyby, ktery byl na polozce nalezen
};

struct CheckResult
{
    VfsStatus status = VfsStatus::OK; // VfsStatus::OK, pokud snebyly nalezeny zadne chyby v celem souborovem systemu, jinak VfsStatu::BUG_DETECTED
    std::vector<CheckItem> bugs;      // pole popisu polozek a jejich chyb
};