#include <iostream>
#include <filesystem>
#include <tao/pegtl.hpp>
#include "actions.cpp"
#include <willow/willow.hpp>

namespace pegtl = tao::pegtl;

namespace willow::parser
{
    Parser::Parser()
    {
        st = State();
    }

    void Parser::parse(const std::string &filepath)
    {

        
        std::string currDirectory = std::filesystem::path(filepath).parent_path().string();
        st.filepathStack.push(currDirectory);

        try
        {
            pegtl::file_input in(filepath);
            try
            {
                pegtl::parse<main_grammar, action>(in, st);
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
        catch (std::filesystem::filesystem_error &e)
        {
            std::cerr << "Error: Failed to find file with path " << e.path1() << std::endl;
        }
    }
}