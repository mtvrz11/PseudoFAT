#include "VirtualDisk.h"
#include <iostream>
#include <fstream>

VirtualDisk::VirtualDisk(const std::string &name) : filename(name) {}

// Neměl bych radši používat uint32_t? Proč používám zrovna int32_t? To proto, abych měl zajištěnou figní vleikost 4 byty? K čemu to potřebuji?
bool VirtualDisk::create_empty(int32_t total_bites)
{
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
        std::cerr << "CANNOT CREATE FILE" << std::endl;
        return false;
    }

    file.seekp(total_bites - 1);
    file << '\0';

    file.close();
    return true;
}

bool VirtualDisk::write_bytes(int32_t offset, const void *data, size_t size)
{
    std::fstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Co tu mam vypsat???" << std::endl;
        return false;
    }

    file.seekp(offset);
    file.write(reinterpret_cast<const char *>(data), size);

    file.close();
    return true;
}

bool VirtualDisk::read_bytes(int32_t offset, void *buffer, size_t size)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Co tu mam vypsat???" << std::endl;
        return false;
    }

    file.seekg(offset);
    file.read(reinterpret_cast<char *>(buffer), size);

    file.close();
    return true;
}