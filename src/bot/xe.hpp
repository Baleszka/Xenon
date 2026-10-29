#pragma once

#include "replay.hpp"

#include <filesystem>
#include <asp/fs/std.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace geode::prelude;

inline char replayName[512] = "";

inline std::filesystem::path getReplaysDir()
{
    return (Mod::get()->getPersistentDir() / "replays").toStdPath().unwrap();
}

void saveMacro(std::filesystem::path path, char name[512], int tps, bool cbf, std::vector<input> macro);
void loadMacro(std::filesystem::path path, char name[512]);

inline std::vector<unsigned long long> parseDelimitedNumbers(std::string &input)
{
    std::vector<unsigned long long> numbers;
    std::istringstream ss(input);
    unsigned long long n;
    while (ss >> n) { numbers.push_back(n); }
    return numbers;
}

inline std::vector<std::string> getReplays()
{
    std::vector<std::string> replays;
    for (auto &entry : std::filesystem::directory_iterator(getReplaysDir()))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".xe")
        {
            replays.push_back(entry.path().stem().string());
        }
    }
    return replays;
}

inline std::vector<double> parseDelimitedNumbersCBF(std::string &input)
{
    std::vector<double> numbers;
    std::istringstream ss(input);
    double n;
    while (ss >> n) { numbers.push_back(n); }
    return numbers;
}
