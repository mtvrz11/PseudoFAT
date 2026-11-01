#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>

#include "../fs/VFS.h"

class Shell
{
    VFS &fs;
    std::unordered_map<std::string, std::function<void(std::stringstream &ss)>> commands;
    bool is_running = false;

    /// @brief Vytiskne vysledek prikazu (hlasku) na standardni vystup.
    /// @param status vysledek prikazu
    void print_status(VfsStatus status);

    /// @brief Namapuje prikazy na funkci, ktera zajisti rozparsovani parametru,
    ///        zavolani zpracovani prikazu ve VFS a vytisteni vysledku.
    void register_commands();

    /// @brief Zpracuje jednu radku prikazu.
    /// @param line radka ke zpracovani
    void execute_line(const std::string &line);

    /// @brief Nacte vstupni soubor a po radkach jej odesila ke zpracovani.
    /// @param input_file cesta ke vstupnimu souboru v hostitelskem OS
    void execute_load(const std::string &input_file);

public:
    Shell(VFS &fs);

    /// @brief Ve smycce nacita prikazy ze standardniho vstupu a predava je dal ke zpracovani.
    void run();
};