#include "FatFileSystem.h"

FatFileSystem::FatFileSystem(const std::string &host_file)
    : disk(host_file), is_formated(false) {};
