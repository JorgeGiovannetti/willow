#include <willow/semantics/semantic_cube.hpp>
#include <string>
#include <vector>

using std::string, std::vector;

namespace willow::semantics
{
    int n = 30;

    SemanticCube::SemanticCube() : semanticMapping(n, vector<vector<string>(n, vector<string>(n))) {

    }

    string query(string, string, string);
}