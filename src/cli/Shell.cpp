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
    {VfsStatus::FILE_NOT_FOUND, "FILE NOT FOUND"},
    {VfsStatus::NOT_EMPTY, "NOT EMPTY"},
    {VfsStatus::PATH_NOT_FOUND, "PATH NOT FOUND"},
    {VfsStatus::BUG_DETECTED, "BUG DETECTED"}};

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

    commands["incp"] = [this](std::stringstream &ss)
    {
        std::string from, to;
        ss >> from >> to;
        print_status(fs.incp(from, to));
    };

    commands["outcp"] = [this](std::stringstream &ss)
    {
        std::string from, to;
        ss >> from >> to;
        print_status(fs.outcp(from, to));
    };

    commands["mv"] = [this](std::stringstream &ss)
    {
        std::string from, to;
        ss >> from >> to;
        print_status(fs.mv(from, to));
    };

    commands["cp"] = [this](std::stringstream &ss)
    {
        std::string from, to;
        ss >> from >> to;
        print_status(fs.cp(from, to));
    };

    commands["rm"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        print_status(fs.rm(path));
    };

    commands["cat"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        VfsStatus res = fs.cat(path, std::cout);
        if (res == VfsStatus::OK)
            std::cout << std::endl;
        else
            print_status(res);
    };

    commands["info"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        InfoResult res = fs.info(path);
        if (res.status != VfsStatus::OK)
            print_status(res.status);
        else
        {
            std::cout << res.item.name << " - ";
            if (res.item.is_directory)
                std::cout << "dir - ";
            else
                std::cout << "file - size: " << res.item.size << " - ";

            auto &clusters = res.item.cluster_chain;
            for (size_t i = 0; i < clusters.size(); i++)
            {
                if (i == clusters.size() - 1)
                    std::cout << clusters[i];
                else
                    std::cout << clusters[i] << ",";
            }
            std::cout << std::endl;
        }
    };

    commands["load"] = [this](std::stringstream &ss)
    {
        std::string path;
        ss >> path;
        execute_load(path);
    };

    commands["bug"] = [this](std::stringstream &ss)
    {
        std::string path;
        int type = 3; // defaultne bad size
        ss >> path;
        if (!ss.eof())
            ss >> type;
        if (type == 1)
            print_status(fs.bug(path, BugType::OUT_OF_RANGE));
        else if (type == 2)
            print_status(fs.bug(path, BugType::CYCLE));
        else
            print_status(fs.bug(path, BugType::BAD_SIZE));
    };

    commands["check"] = [this](std::stringstream &ss)
    {
        auto res = fs.check();
        if (res.status == VfsStatus::OK)
            print_status(res.status);
        else
        {
            for (const auto &bug : res.bugs)
            {
                print_status(VfsStatus::BUG_DETECTED);
                std::cout << "DESCRIPTION: ";
                if (bug.bug == BugType::BAD_SIZE)
                    std::cout << "Size in directory does not match number of clusters allocated in FAT table. ";
                else if (bug.bug == BugType::CYCLE)
                    std::cout << "Cicle in FAT table detected. ";
                else if (bug.bug == BugType::OUT_OF_RANGE)
                    std::cout << "Cluster number out of range of the FAT table. ";
                std::cout << (bug.is_directory ? "Dir: " : "File: ") << bug.name << std::endl;
            }
        }
    };
}

void Shell::execute_line(const std::string &line)
{
    if (line.empty())
        return;

    std::stringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd == "exit")
    {
        is_running = false;
        return;
    }

    auto it = commands.find(cmd);
    if (it == commands.end())
    {
        std::cout << "INVALID COMMAND" << std::endl;
        return;
    }

    if (cmd != "format" && cmd != "load" && !fs.is_ready())
    {
        print_status(VfsStatus::DISK_NOT_FORMATTED);
        return;
    }

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

void Shell::execute_load(const std::string &input_file)
{
    std::ifstream script_file(input_file);
    if (!script_file.is_open())
    {
        print_status(VfsStatus::FILE_NOT_FOUND);
        return;
    }

    std::string line;
    while (is_running && std::getline(script_file, line))
    {
        execute_line(line);
    }
}

void Shell::run()
{
    std::string line;
    is_running = true;

    while (is_running && getline(std::cin, line))
    {
        execute_line(line);
    }
}