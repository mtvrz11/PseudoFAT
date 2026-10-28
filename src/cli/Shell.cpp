#include "Shell.h"
#include "../exceptions/Exceptions.h"
#include "../fs/FsTypes.h"

#include <iostream>
#include <sstream>

Shell::Shell(VFS &fs) : fs(fs)
{
    register_commands();
}

static const std::unordered_map<VfsStatus, std::string> status_messages = {
    {VfsStatus::OK, "OK"},
    {VfsStatus::ALREADY_EXIST, "EXIST"},
    {VfsStatus::CANNOT_CREATE_FILE, "CANNOT CREATE FILE"},
    {VfsStatus::DISK_FULL, "DISK FULL"},
    {VfsStatus::DISK_NOT_FORMATTED, "DISK NOT FORMATTED"},
    {VfsStatus::FILE_NOT_OFUND, "FILE NOT FOUND"},
    {VfsStatus::IS_DIRECTORY, "IS DIRECTORY"},
    {VfsStatus::NOT_EMPTY, "NOT EMPTY"},
    {VfsStatus::PATH_NOT_FOUND, "PATH NOT FOUND"},
};

void Shell::print_status(VfsStatus status)
{
    auto it = status_messages.find(status);
    if (it != status_messages.end())
        std::cout << it->second << std::endl;
}

void Shell::register_commands()
{
    commands["format"] = [this](std::stringstream &ss)
    {
        std::string size;
        ss >> size;
        size = size.substr(0, size.size() - 2);
        int32_t total_bytes = std::stoi(size) * 1024 * 1024;
        print_status(fs.format(total_bytes));
    };

    commands["pwd"] = [this](std::stringstream &ss)
    {
        std::cout << fs.pwd() << std::endl;
    };

    commands["ls"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        LsResult res = fs.ls(path);
        if (res.status != VfsStatus::OK)
            print_status(res.status);
        else
        {
            for (const auto &item : res.items)
            {
                if (item.is_directory)
                std:
                    std::cout << "DIR: " << item.name << std::endl;
                else
                    std::cout << "FILE: " << item.name << std::endl;
            }
        }
    };

    commands["mkdir"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        print_status(fs.mkdir(path));
    };

    commands["cd"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        print_status(fs.cd(path));
    };

    commands["rmdir"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        print_status(fs.rmdir(path));
    };
}

void Shell::run()
{
    std::string line;

    while (getline(std::cin, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "exit")
            break;

        auto it = commands.find(cmd);
        if (it == commands.end())
        {
            std::cout << "INVALID COMMAND" << std::endl;
            continue;
        }

        if (cmd != "format" && !fs.is_ready())
            print_status(VfsStatus::DISK_NOT_FORMATTED);

        try
        {
            it->second(ss);
        }
        catch (const DiskException &e)
        {
            std::cout << "HARDWARE DISK ERROR: " << e.what() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cout << "FATA ERROR: " << e.what() << std::endl;
        }
    }
}