#include <iostream>

#include <tao/pegtl.hpp>
#include "grammar.cpp"

namespace pegtl = tao::pegtl;

int main(int argc, char* argv[])
{
   if (argc < 2) return 1;

   pegtl::file_input in(argv[1]);

   try {
      pegtl::parse<programa>(in);
      std::cout << "Accepted!" << std::endl;
   }
   catch (const pegtl::parse_error& e) {
      const auto p = e.positions().front();
      std::cerr << e.what() << std::endl
         << in.line_at(p) << '\n'
         << std::setw(p.column) << '^' << std::endl;
   }

   return 0;
}