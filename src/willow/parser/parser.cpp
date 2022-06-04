#include <iostream>
#include <string>
#include <tao/pegtl.hpp>
#include "actions.cpp"
#include <willow/willow.hpp>

namespace pegtl = tao::pegtl;
using willow::symbols::SymbolTable;

namespace willow::parser
{
    void parse(const std::string &filepath)
    {
        pegtl::file_input in(filepath);

        State st = State();

        try
        {
            pegtl::parse<main_grammar, action>(in, st);
            std::cout << "Accepted!" << std::endl;
            st.displayQuadruples();
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