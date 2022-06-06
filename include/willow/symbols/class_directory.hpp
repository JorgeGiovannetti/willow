#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include "function_directory.hpp"
#include "symbol_table.hpp"

namespace willow::symbols
{

    struct Attribute : Symbol {
        std::string access;
    };

    struct ClassSignature
    {
        std::string id;
        std::vector<Attribute> attributes;
        std::vector<FunctionDirectory> methods;
        int size;
    };

    class ClassDirectory
    {
    public:
        ClassSignature lookup(std::string);
        void insert(ClassSignature);

    private:
        std::unordered_map<std::string, ClassSignature> classes;
    };
}