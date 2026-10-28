#include "VFS.h"
#include <sstream>

std::vector<std::string> VFS::split_path(const std::string &path)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokens_stream(path);
    while (std::getline(tokens_stream, token, '/'))
    {
        if (token.empty())
            continue;
        tokens.push_back(token);
    }
    return tokens;
}

PathResult VFS::resolve_path(const std::string &path)
{
    PathResult result;
    // prazdna cesta - validni cesta -> aktualni adresar
    if (path.empty())
    {
        result.is_valid = true;
        result.is_directory = true;
        result.target_cluster = current_cluster;
        return result;
    }

    auto tokens = split_path(path);
    // cesta nabyla prazdna, ale obsahovala pouze lomitko -> root
    if (tokens.empty())
    {
        result.is_valid = true;
        result.is_directory = true;
        result.target_cluster = 0; // root
        return result;
    }

    int32_t search_cluster = (path[0] == '/') ? 0 : current_cluster;
    int32_t parent = search_cluster;
    result.target_name = tokens.back();

    for (int i = 0; i < tokens.size(); i++)
    {
        const std::string &token = tokens[i];
        auto chain = fat_manager.get_cluster_chain(search_cluster);
        directory_item item;
        // polozka v adresari neexistuje
        if (!dir_manager.find_item(chain, token, item))
        {
            // neexistuje posledni polozka v retezci -> je to validni cesta a posledni polozku musim teprve vytvorit
            if (i == tokens.size() - 1)
            {
                result.is_valid = true;
                result.parent_cluster = search_cluster;
                result.target_cluster = -1; // neexistuje
            }
            // neexistuje nejaka polozka uprostred cesty -> nevalidni cesta
            else
            {
                result.is_valid = false;
            }
            return result;
        }

        // soubor uprostred cesty
        if (i < tokens.size() - 1 && !item.isDirectory)
        {
            result.is_valid = false;
            return result;
        }

        parent = search_cluster;
        search_cluster = item.start_cluster;
        result.is_directory = item.isDirectory;
    }

    // vsechno dobehlo a existovalo; informaci o tom, zda je cil adresar nebo soubor uz mam z cyklu
    result.is_valid = true;
    result.parent_cluster = parent;
    result.target_cluster = search_cluster;

    return result;
}

std::string VFS::build_absolute_path(const std::string &path)
{
    std::vector<std::string> stack;

    // pokud jde o relativni cestu
    if (path.empty() || path[0] != '/')
    {
        stack = split_path(current_path); // dame do stacku nejprve tokeny od rootu k aktualnimu adresari
    }

    auto tokens = split_path(path);
    for (const auto &token : tokens)
    {
        if (token == ".")
            continue;
        if (token == "..")
        {
            if (!stack.empty())
                stack.pop_back();
            continue;
        }
        stack.push_back(token);
    }

    if (stack.empty())
        return "/";

    std::string result = "";
    for (const auto &dir : stack)
    {
        result += ("/" + dir);
    }

    return result;
}

std::string VFS::pwd()
{
    return current_path; // ma pwd byt v tomto souboru, nebo treba spis v VSF_dir?
}