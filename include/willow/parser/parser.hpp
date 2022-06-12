#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include "state.hpp"

namespace willow::parser
{

    class Parser
    {
    public:
        void parse(const std::string &filepath);
        void generateObjectFile(const std::string &filepath);
        State st;
    };
}

#endif