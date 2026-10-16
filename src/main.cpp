#include "./cli/Parser.h"

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

    Parser parser(host_filename);
    parser.run();

    return 0;
}