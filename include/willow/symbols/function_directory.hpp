#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include "symbol_table.hpp"

namespace willow::symbols
{

    struct FunctionSignature
    {
        std::string id;
        std::vector<Symbol> params;
        std::string return_type;
        int size;
    };

    class FunctionDirectory
    {
    public:
        FunctionSignature lookup(std::string);
        void insert(FunctionSignature);

    private:
        std::unordered_map<std::string, FunctionSignature> functions;
    };
}