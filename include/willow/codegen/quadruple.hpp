#pragma once

#include <string>
#include <vector>

namespace willow::codegen
{
    class Quadruple
    {
    public:
        Quadruple(std::string, std::string, std::string, std::string);
        Quadruple(std::string, std::string, std::string);
        std::string to_string();
    
    private:
        std::string op;
        std::vector<std::string> addresses;
    };
}