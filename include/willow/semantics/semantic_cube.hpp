#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace willow::semantics
{
    class SemanticCube
    {
    public:
        SemanticCube();
        std::string query(const std::string &, const std::string &, const std::string &);
        void newType(const std::string &);

    private:
        std::vector<std::vector<std::vector<int>>> semanticMapping;
        std::unordered_map<std::string, int> typeStringToInt;
        std::unordered_map<int, std::string> typeIntToString;
        std::unordered_map<std::string, int> operatorMap;
    };
}
