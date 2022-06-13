#include <string>

#include "willow/symbols/function_directory.hpp"

namespace willow::symbols
{
    void FunctionDirectory::insert(FunctionSignature signature)
    {
        if (functions.count(signature.id))
        {
            throw std::string("Function with identifier " + signature.id + " has already been declared");
        }

        functions[signature.id] = signature;
    }

    FunctionSignature FunctionDirectory::lookup(std::string id)
    {
        if (!functions.count(id))
        {
            throw std::string("Function with identifier " + id + " does not exist");
        }

        return functions[id];
    }

    void FunctionDirectory::addParam(willow::semantics::TypeManager tm, std::string id, std::string param_type)
    {
        if (!functions.count(id))
        {
            throw std::string("Function with identifier " + id + " does not exist");
        }
        
        functions[id].params.push_back(param_type);
    }
    void FunctionDirectory::addReturnType(willow::semantics::TypeManager tm, std::string id, std::string return_type)
    {
        if (!functions.count(id))
        {
            throw std::string("Function with identifier " + id + " does not exist");
        }

        if (functions[id].return_type != "none")
        {
            throw std::string("Cannot redeclare return type for function " + id);
        }

        functions[id].return_type = return_type;
    }

    std::unordered_map<std::string, FunctionSignature> FunctionDirectory::getFunctions()
    {
        return functions;
    }
}