#pragma once

#include <stdexcept>
#include <string>

/// @brief uzivatelska vyjimka dedici od std::runtime_error
///        informuje o hardwarove chybe
class DiskException : public std::runtime_error
{
public:
    explicit DiskException(const std::string &msg) : std::runtime_error("DISK I/O ERROR: " + msg) {}
};