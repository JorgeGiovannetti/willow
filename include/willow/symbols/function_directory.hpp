#ifndef FUNCTION_DIRECTORY_HPP
#define FUNCTION_DIRECTORY_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include "symbol_table.hpp"
#include "willow/semantics/type_manager.hpp"

namespace willow::symbols
{

    struct FunctionSignature
    {
        std::string id;
        std::string location;
        std::vector<std::string> params;
        std::string return_type = "none";
    };

    class FunctionDirectory
    {
    public:
        FunctionSignature lookup(std::string);
        void insert(FunctionSignature);
        void addParam(willow::semantics::TypeManager tm, std::string function_id, std::string param_type);
        void addReturnType(willow::semantics::TypeManager tm, std::string function_id, std::string return_type);
        std::unordered_map<std::string, FunctionSignature> getFunctions();

    private:
        std::unordered_map<std::string, FunctionSignature> functions;
    };
}

#endif