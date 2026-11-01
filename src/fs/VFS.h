#pragma once

#include "../disk/VirtualDisk.h"
#include "DirectoryManager.h"
#include "FatManager.h"
#include "Structures.h"
#include "FsTypes.h"

#include <string>
#include <vector>

class VFS
{
    VirtualDisk disk;
    description boot_record;
    FatManager fat_manager;
    DirectoryManager dir_manager;

    bool is_formatted;              // je disk naformatovany?
    std::string current_path = "/"; // aktualni umisteni v adresari (pro reseni relativnich cest a prikaz pwd)
    int32_t current_cluster = 0;    // cislo pocatecniho clusteru aktualniho adresare

    /// @brief Nacte souborovy system ze zavadece a naplni FAT tabulku prislusnymi daty.
    void boot_fs();

    /// @brief Metoda pro bezpecne pridani adresarove polozky do adresare.
    ///        Pri nedostatku mista v aktualnim adresari prialokuje adresari novy cluster ve FAT tabulce.
    /// @param parent_cluster pocatecni cluster rodicovskeho sdresare
    /// @param name jmeno polozky
    /// @param is_dir je to adresar nebo soubor?
    /// @param size velikost polozky
    /// @param target_cluster cilovy cluster, na kterem jsou ulozena data polozky
    /// @return true, pokud se operace podarila, jinak false, pokud nebylo misto na disku
    bool add_to_directory(int32_t parent_cluster, const std::string &name, bool is_dir, int32_t size, int32_t target_cluster);

    /// @brief Metoda pro bzepecne odebrani adresarove polozky z adresare.
    ///        Pokud odstranenim polozky dojde k vyprezdneni nektereho clusteru adresare, cluster korektne uvolni ve FAT tabulce.
    /// @warning Pro uvolneni nejakeho clusteru se zmeni retezec nalezejici rodicovskemu adresari.
    /// @param parent_cluster pocatecni cluster rodicovskeho adresare
    /// @param name jmeno polozky k odstraneni
    void remove_from_directory(int32_t parent_cluster, const std::string &name);

    /// @brief Rozdeli metodu podle znaku / na pole tokenu.
    /// @param path textova cesta k rozdeleni
    /// @return pole tokenu - zatim bez jakekoliv validace existence cesty
    std::vector<std::string> split_path(const std::string &path);

    /// @brief Prevede textovou podobu cesty na strukturu PathResult, obsahujici cisla clusteru a dalsi uzitecne informace.
    /// @param path cesta k prevedeni
    /// @return struktura popisujici validitu cesty, cisla clusteru atd.
    PathResult resolve_path(const std::string &path);

    /// @brief Z relativni cesty na zaklade aktualniho umisteni v adresari sestavi absolutni cestu.
    ///        Dokaze korektne zpracovavat zkratky .. a . pro rodicovsky a aktualni adresar.
    /// @param path relativni cesta k prevedeni
    /// @return sestavena absolutni cesty v textove podobe
    std::string build_absolute_path(const std::string &path);

    /// @brief Nacita data ze streamu po velikosti jednoho clusteru a zapisuje je do clusteru v retezci.
    /// @param hostfile vstupni stream (soubor)
    /// @param chain retezec, do ktereho maji byt data souboru zkopirovana
    /// @param file_size velikost souboru
    void write_stream_to_chain(std::ifstream &hostfile, const std::vector<int32_t> chain, int32_t file_size);

    /// @brief Nacita data po jednom clusteru z retezce a kopiruje je na obecny vystupni proud.
    /// @param out_stream vystupni proud
    /// @param chain zdrojovy retezec clusteru, ve kterem jsou ulozena data
    /// @param file_size velikost souboru k precteni
    void read_chain_to_stream(std::ostream &out_stream, const std::vector<int32_t> chain, int32_t file_size);

    /// @brief Spocita adresu daneho clusteru v datove oblastni na disku.
    /// @param clusetr_idx index clusteru
    /// @return adresa clusteru na disku
    int32_t get_cluster_offset(int32_t clusetr_idx);

public:
    VFS(const std::string &host_file);

    bool is_ready(); // disk je naformatovany a souborovy system je pripraven prijimat a zpracovavat vsechny typy prikazu

    /// @brief Naformatuje disk na danou velikost, spocte hodnoty do zavadece a zapise jej na disk,
    ///        vytvori prazdnou fat tabulku a prida do adresare root
    /// @param total_bytes celkova velikost disku urcena souborovemu systemu
    /// @return VfsStatus::OK, pokud se operace podarila, jiank dojde k vyhozeni vyjimky z nizsich vrstev
    VfsStatus format(int32_t total_bytes);

    /// @brief Vrati aktualni cestu v adresarovem stromu.
    /// @return aktualni cesta
    std::string pwd();

    /// @brief Projde adresar a vrati informace o polozkach v nem ulozenych.
    /// @param path cesta do adresare
    /// @return VfsStatus::OK + informace o polozkach, pokud se operace podarila, jiank VfsStatus::PATH_NOT_FOUND,
    ///         pokud nebyla naleze zadana cesta (status zapouzdrey na prvnim miste struktury)
    LsResult ls(const std::string &path = "");

    /// @brief Vytvori novy adresar v zadanem rodicovskem adresari.
    /// @param path cesta k novemu adresari
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::DISK_FULL, pokud na disku nebylo dost mista pro vytvoreni nove polozky,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta,
    ///         VfsStatus::ALREADY_EXIST v pripad, ze polozka daneho jmena v rodicovskem adresari jiz existuje.
    VfsStatus mkdir(const std::string &path);

    /// @brief Zmeni aktuualni cestu na zadanou cestu.
    /// @param path cesta, kam se chceme presunout
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta
    VfsStatus cd(const std::string &path);

    /// @brief Odstrani existujici adresar z rodicovskeho adresare a uvolni clustery, ktere zabiral ve FAT tabulce.
    /// @param path cesta k adresari, ktery ma byt odstranen
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::NOT_EMPTY, pokud adresar k odstraneni neni prazdny,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta
    VfsStatus rmdir(const std::string &path);

    /// @brief Vkopiruje soubor z hostitelskeho OS do aźadaného adresare
    /// @param from cesta k souboru v hostitelskem OS
    /// @param to cesta k umisteni v nasem souborovem systemu
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::DISK_FULL, pokud na disku nebylo dost mista pro vytvoreni nove polozky,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta,
    ///         VfsStatus::ALREADY_EXIST v pripad, ze polozka daneho jmena v rodicovskem adresari jiz existuje
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje zdroj
    VfsStatus incp(const std::string &from, const std::string &to);

    /// @brief Vykopiruje zadany soubor ven do hostitelskeho OS
    /// @param from cesta k souboru v nasem souborovem systemu
    /// @param to cesta k umisteni v souborovem systemu hostitelskeho OS
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta,
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje zdroj
    VfsStatus outcp(const std::string &from, const std::string &to);

    /// @brief Zkopiruje soubor na zadanem umisteni do nove vytvoreneho souboru v cilovem umsteni. Nepodporuje prepisovani souboru.
    /// @param from cesta k existujicimu souboru, ktery ma byt zkopirovan
    /// @param to cesta k cilovemu souboru (ktery zatim neexistuje a ma byt vytvoren)
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::DISK_FULL, pokud na disku nebylo dost mista pro vytvoreni nove polozky,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta,
    ///         VfsStatus::ALREADY_EXIST v pripad, ze polozka daneho jmena v rodicovskem adresari jiz existuje
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje zdroj
    VfsStatus cp(const std::string &from, const std::string &to);

    /// @brief Presune nebo prejmenuje soubor. Prepisuje pouze udaje v adresari, na disk ani FAT tabulku nesaha.
    /// @param from soubor, ktery ma byt presunut nebo prejmenovan
    /// @param to cilove umisteni (jmeno) souboru
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::DISK_FULL, pokud na disku nebylo dost mista pro vytvoreni nove polozky,
    ///         VfsStatus::PATH_NOT_FOUND, pokud byla zadana nevalidni cesta,
    ///         VfsStatus::ALREADY_EXIST v pripad, ze polozka daneho jmena v rodicovskem adresari jiz existuje
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje zdroj
    /// @note V pripade nepodarene akce zustava puvodni soubor nezmenen.
    VfsStatus mv(const std::string &from, const std::string &to);

    /// @brief Odstrani soubor na danem umisteni.
    /// @param path cesta k souboru, ktery ma byt odstranen
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje soubor k odstraneni
    VfsStatus rm(const std::string &path);

    /// @brief Vypise obsah souboru do zadaneho streamu.
    /// @param path cesta k souboru, ktery ma byt vypsan
    /// @param os instance streamu, do ktereho ma byt zapisovano
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje zdroj
    VfsStatus cat(const std::string &path, std::ostream &os);

    /// @brief Vrati informace o zadanem souboru nebo adresari.
    /// @param path cesta k polozce
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje zdroj.
    ///         Status je zapouzdren na prvnim miste vracene struktury. V pripade uspechu nasleduje struktura s popisem polozky.
    InfoResult info(const std::string &path);

    /// @brief Vytvori umele poskozeni souboroveho systemu.
    /// @warning POSKODI SOUBOROVY SYSTEM !!!
    /// @param path cesta k souboru, ktery ma byt poskozen
    /// @param type typ poskozeni
    /// @return VfsStatus::OK, pokud se operace podarila,
    ///         VfsStatus::FILE_NOT_FOUND, pokud neexistuje soubor k poskozeni
    VfsStatus bug(const std::string &path, BugType type);

    /// @brief Projde cely adresarovy strom a kazdou polozku otestuje na vyskyt chyb.
    /// @return VfsStatus::OK, pokud nebyla nalezena zadna chyba,
    ///         VfsStatus::BUG_DETECTED, bylo nalezene nejake poskozeni. V tom pripade nasleduje struktura se seznamem
    ///         poskocenych polozek a popisem chyb.
    CheckResult check();
};