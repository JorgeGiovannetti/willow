#include <string>

#include <willow/symbols/function_directory.hpp>

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
}