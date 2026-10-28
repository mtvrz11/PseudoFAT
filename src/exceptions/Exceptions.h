#pragma once

#include <stdexcept>
#include <string>

class DiskException : public std::runtime_error
{
public:
    explicit DiskException(const std::string &msg) : std::runtime_error("DISK I/O ERROR: " + msg) {}
};