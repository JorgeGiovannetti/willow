#include <string>

#include "willow/symbols/class_directory.hpp"

namespace willow::symbols
{
    void ClassDirectory::insert(ClassSignature signature)
    {
        if (classes.count(signature.id))
        {
            throw std::string("Class with identifier " + signature.id + " has already been declared");
        }

        classes[signature.id] = signature;
    }

    ClassSignature ClassDirectory::lookup(std::string id)
    {
        if (!classes.count(id))
        {
            throw std::string("Class with identifier " + id + " does not exist");
        }

        return classes[id];
    }

    void ClassDirectory::addAttribute(willow::semantics::TypeManager tm, std::string id, Attribute attr)
    {
        if (!classes.count(id))
        {
            throw std::string("Class with identifier " + id + " does not exist");
        }

        attr.position = classes[id].size;
        int size = tm.getTypeSize(attr.type);

        if (classes[id].attributes.count(attr.id))
        {
            throw std::string("Attribute " + attr.id + " already exists");
        }

        for (Dim dim : attr.dims)
        {
            size *= dim.size;
        }

        classes[id].size += size;
        classes[id].attributes[attr.id] = attr;
    }

    void ClassDirectory::addMethod(std::string id, FunctionSignature func)
    {
    }

    std::unordered_map<std::string, ClassSignature> ClassDirectory::getClasses()
    {
        return classes;
    }
}