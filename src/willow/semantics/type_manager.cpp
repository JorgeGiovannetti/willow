#include <iostream>
#include <string>
#include <vector>

#include <willow/semantics/semantic_cube.hpp>

namespace willow::semantics
{

    TypeManager::TypeManager()
    {

        typeIntToString.resize(6);
        typeIntToString[0] = "none";
        typeIntToString[1] = "int";
        typeIntToString[2] = "float";
        typeIntToString[3] = "bool";
        typeIntToString[4] = "char";
        typeIntToString[5] = "string";

        typeSize.resize(6);
        typeSize[0] = 1;
        typeSize[1] = 1;
        typeSize[2] = 1;
        typeSize[3] = 1;
        typeSize[4] = 1;
        typeSize[5] = 1;

        typeStringToInt["none"] = 0;
        typeStringToInt["int"] = 1;
        typeStringToInt["float"] = 2;
        typeStringToInt["bool"] = 3;
        typeStringToInt["char"] = 4;
        typeStringToInt["string"] = 5;
    }

    void TypeManager::newType(const std::string &type_name, const int &size)
    {

        if (typeIntToString.size() == 255)
        {
            throw std::string("Type limit reached");
        }

        if(typeStringToInt.count(type_name))
        {
            throw std::string("Tried to create type " + type_name + ", which already exists");
        }

        std::cout << "new type in type manager: " << type_name << " with size " << size << std::endl;

        typeIntToString.push_back(type_name);
        typeStringToInt[type_name] = typeIntToString.size() - 1;
        typeSize.push_back(size);

        std::cout << "new type was assigned code " << typeStringToInt[type_name] << std::endl;
    }

    int TypeManager::increaseSize(const std::string &type_name, const int &size)
    {
        int type_code = typeStringToInt[type_name];
        typeSize[type_code] += size;
        return typeSize[type_code];
    }

    std::string TypeManager::getType(int type_code)
    {
        return typeIntToString[type_code];
    }

    int TypeManager::getType(std::string type_name)
    {
        return typeStringToInt[type_name];
    }

    int TypeManager::getTypeSize(int type_code)
    {
        return typeSize[type_code];
    }

    int TypeManager::getTypeSize(std::string type_name)
    {
        return typeSize[typeStringToInt[type_name]];
    }

}