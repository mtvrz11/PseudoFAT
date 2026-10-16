#include "Parser.h"
#include "../fs/FatFileSystem.h"
#include <iostream>
#include <sstream>

Parser::Parser(const std::string &host_file) : host_filename(host_file) {}

void Parser::run()
{
    std::string line;
    FatFileSystem fs{host_filename};

    while (getline(std::cin, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "exit")
            break;
        else if (cmd == "format")
        {
            std::string size;

            ss >> size;
            size = size.substr(0, size.size() - 2); // oriznuti poslednich dvou znaku (MB)

            int32_t total_bytes = std::stoi(size) * 1024 * 1024;

            fs.format(total_bytes);
        }
        else
        {
            std::cout << "Zadan neznamy prikaz: " << cmd << std::endl;
        }
    }
}