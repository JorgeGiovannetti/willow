#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "willow/symbols/symbol_table.hpp"

namespace willow::semantics
{
    class TypeManager
    {
    public:
        TypeManager();
        void newType(const std::string &type_name, const int &size);
        int increaseSize(const std::string &type_name, const int &size);
        std::string getType(int type_code);
        int getType(std::string type_name);
        int getTypeSize(std::string type_name);
        int getTypeSize(int type_code);

    private:
        std::unordered_map<std::string, int> typeStringToInt;
        std::vector<std::string> typeIntToString;
        std::vector<int> typeSize;
    };
}
