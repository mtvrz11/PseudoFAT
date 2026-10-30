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
    NOT_EMPTY,
    CANNOT_CREATE_FILE,
    BUG_DETECTED
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

struct InfoItem
{
    std::string name;
    bool is_directory;
    int32_t size;
    std::vector<int32_t> cluster_chain;
};

struct InfoResult
{
    VfsStatus status;
    InfoItem item;
};

enum class BugType
{
    OUT_OF_RANGE = 1,
    CYCLE = 2,
    BAD_SIZE = 3
};

struct CheckItem
{
    bool is_directory;
    std::string name;
    BugType bug;
};

struct CheckResult
{
    VfsStatus status = VfsStatus::OK;
    std::vector<CheckItem> bugs;
};