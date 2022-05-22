#include <willow/willow.hpp>

namespace willow::codegen
{
    Quadruple::Quadruple(std::string op, std::string address1, std::string address2, std::string address3) : addresses{address1, address2, address3}
    {
        this->op = op;
    }

    Quadruple::Quadruple(std::string op, std::string address1, std::string address2) : addresses{address1, address2}
    {
        this->op = op;
    }

    std::string Quadruple::to_string()
    {
        std::string quadString = op;
        for (const std::string &s : addresses)
        {
            quadString += "," + s;
        }

        return quadString;
    }
}