#ifndef CLASS_DIRECTORY_HPP
#define CLASS_DIRECTORY_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include "function_directory.hpp"
#include "symbol_table.hpp"
#include "willow/semantics/type_manager.hpp"

namespace willow::symbols
{

    struct Attribute : Symbol
    {
        std::string access;
        int position = 0;
    };

    struct ClassSignature
    {
        std::string id;
        std::unordered_map<std::string, Attribute> attributes;
        std::unordered_map<std::string, FunctionSignature> methods;
        int size = 0;
    };

    class ClassDirectory
    {
    public:
        ClassSignature lookup(std::string);
        void insert(ClassSignature);
        void addAttribute(willow::semantics::TypeManager, std::string, Attribute);
        void addMethod(std::string, FunctionSignature);
        std::unordered_map<std::string, ClassSignature> getClasses();

    private:
        std::unordered_map<std::string, ClassSignature> classes;
    };
}

#endif