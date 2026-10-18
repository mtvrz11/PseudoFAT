#pragma once
#include <string>
#include "../fs/FatFileSystem.h"

class Parser
{
    FatFileSystem &fs;

public:
    Parser(FatFileSystem &fs);

    void run();
};