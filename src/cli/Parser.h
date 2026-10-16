#pragma once
#include <string>

class Parser
{
    std::string host_filename;

public:
    Parser(const std::string &host_file);

    void run();
};