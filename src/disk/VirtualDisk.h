#pragma once

#include <string>
#include <fstream>

class VirtualDisk
{
    std::string filename;
    std::fstream file;

public:
    bool is_open = false; // je hostitelsky soubor otevreny?

    /// @brief Otevre soubor, ktery reprezentuje virtualni disk.
    /// @param filename nazev hostitelskeho souboru reprezentujiciho disk
    VirtualDisk(const std::string &filename);

    /// @brief Pokud soubor existoval, smaze jeho data. Vytvori soubor o dane velikosti umistenim jednoho znaku na jeho konec.
    /// @param total_bytes velikost ciloveho souboru
    void create_empty(int32_t total_bytes);

    /// @brief Zapise data na danou adresu na virtualnim disku (zapise data na dany ofset do hostitelskeho souboru).
    /// @param offset adresa na virtualnim disku
    /// @param data pointer na data
    /// @param size velikost dat k zapsani
    void write_bytes(int32_t offset, const void *data, size_t size);

    /// @brief Precte data z dane adresy na virtualnim disku a ulozi je do bufferu.
    /// @param offset adresa na virtualnim disku
    /// @param buffer buffer na zapis dat
    /// @param size velikost dat k precteni
    void read_bytes(int32_t offset, void *buffer, size_t size);
};