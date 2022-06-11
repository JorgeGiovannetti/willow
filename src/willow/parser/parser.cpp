#include <iostream>
#include <filesystem>
#include <fstream>

#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "actions.cpp"

namespace pegtl = tao::pegtl;

namespace willow::parser
{
    void Parser::generateObjectFile(const std::string &filepath)
    {

        std::string outFilepath = std::filesystem::path(filepath).filename().replace_extension("wol").string();

        std::ofstream file;

        file.open(outFilepath);

        // Output classes
        for (const auto &[key, value] : st.classdir.getClasses())
        {
            file << key << " " << value.size << std::endl;
        }

        // Separator
        file << "<%>" << std::endl;

        // Output functions
        for (const auto &[key, value] : st.funcdir.getFunctions())
        {
            file << key << " " << value.location << std::endl;
        }

        // Separator
        file << "<%>" << std::endl;

        // Output quadruples
        for (Quadruple quad : st.quadruples)
        {
            file << quad.to_string() << std::endl;
        }
        file.close();
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

                generateObjectFile(filepath);
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