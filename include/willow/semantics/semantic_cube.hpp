#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "type_manager.hpp"

namespace willow::semantics
{
    class SemanticCube
    {
    public:
        SemanticCube();
        std::string query(const std::string &, const std::string &, const std::string &);
        void newType(const std::string &type_name, const int &size);
        std::string getType(int type_code);
        int getType(std::string type_name);
        int getTypeSize(std::string type_name);
        int getTypeSize(int type_code);

    private:
        std::vector<std::vector<std::vector<int>>> semanticMapping;
        TypeManager typeManager;
        std::unordered_map<std::string, int> operatorMap;
    };
}
