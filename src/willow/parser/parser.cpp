#include <iostream>
#include <string>
// #include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl.hpp>
#include "actions.cpp"
#include <willow/willow.hpp>

namespace pegtl = tao::pegtl;
using willow::symbols::SymbolTable;

namespace willow::parser
{
    Parser::Parser()
    {
    }

    void Parser::parse(const std::string &filepath)
    {
        pegtl::file_input in(filepath);

        state st;

        try
        {
            // const std::size_t issues = pegtl::analyze<grammar>();
            // std::cout << "Issues: " << issues << std::endl;
            pegtl::parse<grammar, action>(in, st);
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