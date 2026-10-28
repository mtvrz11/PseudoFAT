#include "./cli/Shell.h"
#include "./fs/VFS.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "CHYBA! Ocekavan jeden parametr!" << std::endl;
        return 1;
    }

    std::string host_filename = argv[1];

    VFS fs(host_filename);

    Shell shell(fs);
    shell.run();

    return 0;
}