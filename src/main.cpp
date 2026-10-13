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

    std::string fs_filename = argv[1];

    std::string line;
    while (true)
    {
        if (!getline(std::cin, line))
            break; // !!! proc?
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "exit")
            break;
        else if (command == "format")
        {
            int size;
            ss >> size;

            // TODO: dodelat formatovani...
            std::cout << "OK" << std::endl;
        }
        else
        {
            // TODO: ostatni prikazy
        }
    }

    return 0;
}