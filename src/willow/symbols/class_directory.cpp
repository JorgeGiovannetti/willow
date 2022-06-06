#include <string>

#include <willow/symbols/class_directory.hpp>

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
}