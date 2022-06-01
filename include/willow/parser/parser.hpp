#pragma once

#include <string>

namespace willow::parser
{
    class Parser
    {
    public:
        Parser();
        void parse(const std::string& filepath);
    };
}