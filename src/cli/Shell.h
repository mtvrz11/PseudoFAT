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
    void execute_line(const std::string &line);
    void execute_load(const std::string &input_file);

public:
    Shell(VFS &fs);

    void run();
};