/**
 * @file VirtualDisk.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-10-13
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once
#include <string>
#include <fstream>

class VirtualDisk
{
    std::string filename;

public:
    VirtualDisk(const std::string &name);

    bool create_empty(int32_t total_bites);

    bool write_bytes(int32_t offset, const void *data, size_t size);

    bool read_bytes(int32_t offset, void *buffer, size_t size);
};