#pragma once

#include <string>

#include "state.hpp"

namespace willow::parser
{

    class Parser
    {
    public:
        Parser();
        void parse(const std::string &filepath);
        State st;
    };
}