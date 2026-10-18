#include "./cli/Parser.h"
#include "./fs/FatFileSystem.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "CHYBA! Ocekavan jeden parametr!" << std::endl;
    }

    std::string host_filename = argv[1];

    FatFileSystem fs(host_filename);

    Parser parser(fs);
    parser.run();

    return 0;
}