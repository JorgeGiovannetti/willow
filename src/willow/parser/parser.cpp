#include <iostream>
#include <string>
#include <tao/pegtl.hpp>
#include "grammar.cpp"
#include <willow/willow.hpp>

namespace pegtl = tao::pegtl;

namespace willow::parser
{
    Parser::Parser()
    {
    }

    void Parser::parse(const std::string &filepath)
    {
        pegtl::file_input in(filepath);

        try
        {
            pegtl::parse<grammar_main>(in);
            std::cout << "Accepted!" << std::endl;
        }
        catch (const pegtl::parse_error &e)
        {
            const auto p = e.positions().front();
            std::cerr << e.what() << std::endl
                      << in.line_at(p) << '\n'
                      << std::setw(p.column) << '^' << std::endl;
        }
    }
}