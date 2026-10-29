#include "VirtualDisk.h"
#include "../exceptions/Exceptions.h"

#include <iostream>
#include <fstream>

VirtualDisk::VirtualDisk(const std::string &filename)
    : filename(filename), file(filename, std::ios::binary | std::ios::in | std::ios::out)
{
    if (file.is_open())
        is_open = true;
}

void VirtualDisk::create_empty(int32_t total_bytes)
{
    // pokud byl soubor otevren, zavru ho, abych ho mohl premazat a znovu naformatovat
    if (file.is_open())
    {
        file.close();
        is_open = false;
    }

    std::ofstream f(filename, std::ios::binary | std::ios::trunc);

    if (!f.is_open())
        throw DiskException("Virtual disk file is not open.");

    // natazeni na pozadovanou velikost
    f.seekp(total_bytes - 1);
    f.put('\0');

    f.close();                                                            // uzavreni souboru pro zapis
    file.open(filename, std::ios::binary | std::ios::in | std::ios::out); // a nasledne otevreni pro cteni i zapis
    if (!file.is_open())
        throw DiskException("Virtual disk file is not open.");

    is_open = true;
}

void VirtualDisk::write_bytes(int32_t offset, const void *data, size_t size)
{
    if (!file.is_open())
        throw DiskException("Virtual disk file is not open.");

    file.seekp(offset);
    file.write(reinterpret_cast<const char *>(data), size);

    if (!file.good())
        throw DiskException("Failed to write " + std::to_string(size) + " bytes at offset " + std::to_string(offset));

    file.flush();
}

void VirtualDisk::read_bytes(int32_t offset, void *buffer, size_t size)
{
    if (!file.is_open())
        throw DiskException("Virtual disk file is not open.");

    file.seekg(offset);
    file.read(reinterpret_cast<char *>(buffer), size);

    if (!file.good())
        throw DiskException("Failed to read " + std::to_string(size) + " bytes at offset " + std::to_string(offset));
}