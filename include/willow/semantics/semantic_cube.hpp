#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using std::string, std::vector, std::unordered_map;

namespace willow::semantics
{

    class SemanticCube
    {
    public:
        SemanticCube();
        string query(string, string, string);

    private:
        vector<vector<vector<int>>> semanticMapping;
        unordered_map<string, int> typeMapString;
        unordered_map<int, string> typeMapInt;
        unordered_map<string, int> operatorMap;
    };
}

