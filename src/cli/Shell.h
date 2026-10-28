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

    void print_status(VfsStatus status);
    void register_commands();

public:
    Shell(VFS &fs);

    void run();
};