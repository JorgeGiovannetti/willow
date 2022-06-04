#include <iostream>

#include <tao/pegtl.hpp>
#include "parser/parser.cpp"

namespace pegtl = tao::pegtl;

int main(int argc, char *argv[])
{

   if (argc < 2)
   {
      std::cerr << "Error: No input files. Compilation terminated." << std::endl;
      return 1;
   }

   willow::parser::parse(argv[1]);

   return 0;
}