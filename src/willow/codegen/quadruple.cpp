#include <willow/willow.hpp>

namespace willow::codegen
{
    Quadruple::Quadruple(std::string op, std::string address1, std::string address2, std::string targetAddress)
    {
        this->op = op;
        this->address1 = address1;
        this->address2 = address2;
        this->targetAddress = targetAddress;
    }

    std::string Quadruple::to_string()
    {
        return op + "," + address1 + "," + address2 + "," + targetAddress;
    }
}