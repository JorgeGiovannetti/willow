#pragma once

#include <string>
#include <vector>

namespace willow::codegen
{
    struct Quadruple
    {
        std::string op;
        std::string address1;
        std::string address2;
        std::string targetAddress;
    };
}