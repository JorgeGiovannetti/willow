#include <iostream>

#include <tao/pegtl.hpp>
#include <willow/willow.hpp>

namespace pegtl = tao::pegtl;
using willow::parser::Parser;

int main(int argc, char *argv[])
{

   if (argc < 2)
   {
      std::cerr << "Error: No input files. Compilation terminated." << std::endl;
      return 1;
   }

   Parser parser;
   parser.parse(argv[1]);

   // Add Python VM

   return 0;
}