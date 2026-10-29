#include "xe.hpp"
#include "replay.hpp"
#include "cbf.hpp"

#include <filesystem>
#include <fstream>

using namespace geode::prelude;

void saveMacro(std::filesystem::path path, char name[512], int tps, bool cbf, std::vector<input> macro)
{
    std::filesystem::path macroPath = path / (std::string(name) + ".xe");
    std::ofstream macroFile(macroPath, std::ios::out | std::ios::trunc);
    if (macroFile.is_open())
    {
        macroFile << tps;
        macroFile << " ";
        macroFile << cbf;
        macroFile << "\n";
        if (!cbf)
        {
            for (int i = 0; i < macro.size(); i++)
            {
                macroFile << macro[i].frame;
                macroFile << " ";
                macroFile << macro[i].isHolding;
                macroFile << " ";
                macroFile << macro[i].isPlayer1;
                macroFile << "\n";
            }
            macroFile.close();
        }
        else
        {
            for (int i = 0; i < cbfMacro.size(); i++)
            {
                
                macroFile << cbfMacro[i].preciseFrame;
                macroFile << " ";
                macroFile << cbfMacro[i].isHolding;
                macroFile << " ";
                macroFile << cbfMacro[i].isPlayer1;
                macroFile << "\n";
            }
            macroFile.close();
        }
    }
}

void loadMacro(std::filesystem::path path, char name[512])
{
    std::filesystem::path macroPath = path / (std::string(name) + ".xe");
    std::ifstream macroFile(macroPath);
    if (!macroFile.is_open()) { return; }

    macro.clear(); cbfMacro.clear();
    cbf = false;
    std::string line;

    while (std::getline(macroFile, line))
    {
        if (!cbf) 
        {
            auto nums = parseDelimitedNumbers(line);
            if (nums.size() == 2)
            {
                tps = nums[0];
                cbf = (nums[1] != 0);
            }
            else if (nums.size() == 3)
            {
                macro.push_back({nums[0], (bool)nums[1], (bool)nums[2]});
            }
        }
        else
        {
            auto nums = parseDelimitedNumbersCBF(line);
            if (nums.size() == 3)
            {
                cbfMacro.push_back({static_cast<double>(nums[0]), bool(nums[1]), bool(nums[2])});
            }
        }
    }
    replayStatus = 1;
}

$execute
{
    auto dir = getReplaysDir();
    if (!std::filesystem::exists(dir)) { std::filesystem::create_directories(dir); }
}