#include <iostream>

#include <tao/pegtl.hpp>
#include <willow/willow.hpp>

namespace pegtl = tao::pegtl;
using willow::parser::Parser;
using willow::vm::VM;

int main(int argc, char *argv[])
{

   if (argc < 2)
   {
      std::cerr << "Error: No input files. Compilation terminated." << std::endl;
      return 1;
   }

   Parser parser = Parser();
   parser.parse(argv[1]);

   // VM vm = VM();

   // vm.loadQuadruples(parser.st.quadruples);
   // vm.run();

   return 0;
}