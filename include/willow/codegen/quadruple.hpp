#ifndef QUADRUPLE_HPP
#define QUADRUPLE_HPP

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
        std::string to_string()
        {
            return std::string(op + "," + address1 + "," + address2 + "," + targetAddress);
        }
    };
}

#endif