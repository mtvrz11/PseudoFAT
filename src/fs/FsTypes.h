#pragma once

#include <string>
#include <vector>

enum class VfsStatus
{
    OK,
    DISK_NOT_FORMATTED,
    PATH_NOT_FOUND,
    ALREADY_EXIST,
    DISK_FULL,
    FILE_NOT_FOUND,
    IS_DIRECTORY,
    NOT_EMPTY,
    CANNOT_CREATE_FILE
};

struct PathResult
{
    bool is_valid = false;
    int32_t parent_cluster = -1;
    int32_t target_cluster = -1;
    std::string target_name;
    bool is_directory = false;
};

struct LsItem
{
    std::string name;
    bool is_directory;
};

struct LsResult
{
    VfsStatus status;
    std::vector<LsItem> items;
};
