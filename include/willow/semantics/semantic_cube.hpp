#pragma once

#include <string>
#include <vector>

using std::string, std::vector;

namespace willow::semantics
{

    class SemanticCube
    {
    public:
        SemanticCube();
        int query(string, string, string);

    private:
        vector<vector<vector<int>>> semanticMapping;
        enum types
        {
            NONE,
            INT,
            FLOAT,
            BOOL,
            CHAR,
            STRING
        };
    };
}

