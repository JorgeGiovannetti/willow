#include <iostream>
#include <filesystem>

#include <stack>
#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "grammar_errors.cpp"

namespace pegtl = tao::pegtl;

using willow::codegen::Quadruple;
using willow::symbols::Symbol;
using willow::symbols::Dim;

namespace willow::parser
{

   std::string addDimsToType(std::string type, std::vector<Dim> dims, int currDim)
   {
      std::cout << "addDimsToType got type " << type << " and currDim is " << currDim << std::endl;
      for (int i = currDim; i < dims.size(); i++)
      {
         type += "[" + std::to_string(dims[i].size) + "]";
      }
      return type;
   }

   void addUnaryOperation(State& state)
   {
      std::string operation = state.operatorStack.top();
      state.operatorStack.pop();

      Symbol op1 = state.operandStack.top();
      state.operandStack.pop();

      std::string result_type = state.sc.query(op1.type, "none", operation);

      int allocatedAddress = state.memory.allocMemory(memory::TEMP, state.sc.getType(result_type), state.sc.getTypeSize(result_type));
      std::string address_str = '&' + std::to_string(allocatedAddress);
      Quadruple quad = { operation, op1.address, "", address_str };
      state.quadruples.push_back(quad);

      state.operandStack.push({ address_str, result_type, address_str });
   }

   void addBinaryOperation(State& state)
   {
      std::string operation = state.operatorStack.top();
      state.operatorStack.pop();

      Symbol op2 = state.operandStack.top();
      state.operandStack.pop();

      Symbol op1 = state.operandStack.top();
      state.operandStack.pop();

      std::string op1_type_with_dims = addDimsToType(op1.type, op1.dims, op1.currDimPosition);
      std::string op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);

      std::string result_type = state.sc.query(op1_type_with_dims, op2_type_with_dims, operation);

      int allocatedAddress = state.memory.allocMemory(memory::TEMP, state.sc.getType(result_type), state.sc.getTypeSize(result_type));
      std::string address_str = '&' + std::to_string(allocatedAddress);
      Quadruple quad = { operation, op1.address, op2.address, address_str };
      state.quadruples.push_back(quad);

      state.operandStack.push({ address_str, result_type, address_str });
   }

   template <typename Rule>
   struct action
   {
   };

   // Imports

   template <>
   struct action<a_imports>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         std::string filepath = state.operandStack.top().id.substr(1, state.operandStack.top().id.length() - 2);
         state.operandStack.pop();

         assert(!state.filepathStack.empty());

         std::string currDirectory = state.filepathStack.top();
         std::string importedDirectory = std::filesystem::path(filepath).parent_path().string();

         state.filepathStack.push(currDirectory + "/" + importedDirectory);

         try
         {
            pegtl::file_input importedIn(currDirectory + "/" + filepath);
            pegtl::parse_nested<grammar, action>(in, importedIn, state);
         }
         catch (std::filesystem::filesystem_error& e)
         {
            throw pegtl::parse_error("Error: Failed to find file with path " + e.path1().string(), in);
         }
      }
   };

   template <>
   struct action<a_eof>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         assert(!state.filepathStack.empty());
         state.filepathStack.pop();
      }
   };

   // Scopes

   template <>
   struct action<a_open_scope>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         try
         {
            state.st->createScope(state.currScopeKind);
            state.memory.cacheCurrentMemstate();
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<t_braceclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.st->exitScope();
         state.memory.deallocMemory();
      }
   };

   // Vars

   template <>
   struct action<identifier>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         try
         {
            state.operandStack.push(state.st->lookup(in.string()));
         }
         catch (std::string msg)
         {
            state.operandStack.push({ in.string(), willow::symbols::NONE_TYPE });
         }
      }
   };

   template <>
   struct action<type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.currType = in.string();
      }
   };

   template <>
   struct action<structured_type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {

         for (int i = state.currDims.size() - 2; i >= 0; i--)
         {
            state.currDims[i].displacement_size *= state.currDims[i + 1].displacement_size;
         }

         int type_code = state.sc.getType(state.currType);
         int type_size = state.sc.getTypeSize(type_code);

         int memSegment = state.currScopeKind == symbols::GLOBAL ? memory::GLOBAL : memory::LOCAL;

         state.operandStack.top().type = state.currType;
         state.operandStack.top().dims = state.currDims;

         state.currDims.clear();

         int dims_size = 1;
         for (Dim dim : state.operandStack.top().dims)
         {
            dims_size *= dim.size;
         }

         int allocatedAddress = state.memory.allocMemory(memSegment, type_code, type_size * dims_size);
         std::string address_str = '&' + std::to_string(allocatedAddress);

         state.operandStack.top().address = address_str;
      }
   };

   template <>
   struct action<s_var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         try
         {
            const Symbol& operand = state.operandStack.top();
            state.st->insert(operand.id, operand.type, operand.address, operand.dims);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   // Operators

   template <>
   struct action<t_not>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_mult>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_div>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_mod>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_plus>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_minus>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_lesser>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_greater>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_leq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_geq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_eq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_neq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_and>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_or>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_assign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_multassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_divassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_modassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_plusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_minusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   // Literals

   template <>
   struct action<t_lit_int>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operandStack.push({ in.string(), "int", in.string() });
      }
   };

   template <>
   struct action<t_lit_float>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operandStack.push({ in.string(), "float", in.string() });
      }
   };

   template <>
   struct action<t_lit_bool>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operandStack.push({ in.string(), "bool", in.string() });
      }
   };

   template <>
   struct action<t_lit_char>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operandStack.push({ in.string(), "char", in.string() });
      }
   };

   template <>
   struct action<t_lit_string>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operandStack.push({ in.string(), "string", in.string() });
      }
   };

   // Expressions

   template <>
   struct action<expr_paropen>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<expr_parclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         while (state.operatorStack.top() != "(")
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }

         // Stack top will always be "(" at this point
         state.operatorStack.pop();
      }
   };

   template <>
   struct action<a1_expr>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && state.operatorStack.top() == "or")
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   template <>
   struct action<a1_expr_L7>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && state.operatorStack.top() == "and")
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   template <>
   struct action<a1_expr_L6>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && (state.operatorStack.top() == "!=" || state.operatorStack.top() == "=="))
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   template <>
   struct action<a1_expr_L5>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && (state.operatorStack.top() == ">=" || state.operatorStack.top() == "<=" || state.operatorStack.top() == ">" || state.operatorStack.top() == "<"))
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   template <>
   struct action<a1_expr_L4>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && (state.operatorStack.top() == "+" || state.operatorStack.top() == "-"))
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   template <>
   struct action<a1_expr_L3>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && (state.operatorStack.top() == "*" || state.operatorStack.top() == "/" || state.operatorStack.top() == "%"))
         {
            try
            {
               addBinaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   template <>
   struct action<a1_expr_L2>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.operatorStack.empty() && state.operatorStack.top() == "!")
         {
            try
            {
               addUnaryOperation(state);
            }
            catch (std::string msg)
            {
               throw pegtl::parse_error(msg, in);
            };
         }
      }
   };

   // Assignment

   template <>
   struct action<var_def>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {

         if (state.operatorStack.empty() || state.operatorStack.top() != "=")
         {
            return;
         }

         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();
         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         try
         {
            state.sc.query(op1.type, op2.type, "=");
            state.quadruples.push_back({ "=", op2.address, "", op1.address });
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<assignment>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {

         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();

         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         std::string op1_type_with_dims = addDimsToType(op1.type, op1.dims, op1.currDimPosition);
         std::string op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);

         try
         {
            if (operation == "*=" || operation == "/=" || operation == "+=" || operation == "-=" || operation == "%=")
            {
               operation = operation.substr(0, 1);

               std::string temp_type = state.sc.query(op1_type_with_dims, op2_type_with_dims, operation);

               int allocatedAddress = state.memory.allocMemory(memory::TEMP, state.sc.getType(temp_type), state.sc.getTypeSize(temp_type));
               std::string address_str = '&' + std::to_string(allocatedAddress);
               Quadruple quad = { operation, op1.address, op2.address, address_str };
               state.quadruples.push_back(quad);

               op2 = { address_str, temp_type, address_str };

               op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);
            }

            state.sc.query(op1_type_with_dims, op2_type_with_dims, "=");

            Quadruple quad = { "=", op2.address, "", op1.address };
            state.quadruples.push_back(quad);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   // Conditionals

   template <>
   struct action<a1_conditional>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         Symbol expr_result = state.operandStack.top();
         state.operandStack.pop();

         if (expr_result.type != "bool")
         {
            throw pegtl::parse_error("Error: Expected boolean inside \"if-statement\"", in);
         }

         state.quadruples.push_back({ "gotof", expr_result.address, "", "" });
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a2_conditional>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         size_t conditional_end = state.jumpStack.top();
         state.jumpStack.pop();

         state.quadruples[conditional_end].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   template <>
   struct action<a3_conditional>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.quadruples.push_back({ "goto", "", "", "" });
         size_t false_jump = state.jumpStack.top();
         state.jumpStack.pop();
         state.jumpStack.push(state.quadruples.size() - 1);
         state.quadruples[false_jump].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   // While Loops

   template <>
   struct action<a1_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.jumpStack.push(state.quadruples.size());
      }
   };

   template <>
   struct action<a2_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         Symbol expr_result = state.operandStack.top();
         state.operandStack.pop();

         if (expr_result.type != "bool")
         {
            throw pegtl::parse_error("Error: Expected boolean inside \"while-loop\" condition", in);
         }

         state.quadruples.push_back({ "gotof", expr_result.address, "", "" });
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         size_t loop_jump_point = state.jumpStack.top();
         state.jumpStack.pop();
         size_t loop_start = state.jumpStack.top();
         state.jumpStack.pop();

         state.quadruples.push_back({ "goto", "", "", std::to_string(loop_start) });
         state.quadruples[loop_jump_point].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   // For Loops

   template <>
   struct action<a1_for_range>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         if (op1.type != "int" || op2.type != "int")
         {
            throw pegtl::parse_error("Error: Expected int in for-loop range", in);
         }

         state.operandStack.push(op1);
         state.operandStack.push(op2);
      }
   };

   template <>
   struct action<a1_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         Symbol loop_iterator = state.operandStack.top();

         if (loop_iterator.type != "int")
         {
            throw pegtl::parse_error("Error: Non-int iterators are not (yet) supported.", in);
         }
      }
   };

   template <>
   struct action<a2_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         Symbol range_to = state.operandStack.top();
         state.operandStack.pop();
         Symbol range_from = state.operandStack.top();
         state.operandStack.pop();
         Symbol loop_iterator = state.operandStack.top(); // Doesn't need pop, we need it on a3

         state.quadruples.push_back({ "=", range_from.address, "", loop_iterator.address });

         std::string temp_type = "int";
         int type_code = state.sc.getType(temp_type);

         int allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code));

         std::string address_str = '&' + std::to_string(allocatedAddress);
         state.quadruples.push_back({ "<=", loop_iterator.address, range_to.address, address_str });
         state.jumpStack.push(state.quadruples.size() - 1);

         state.quadruples.push_back({ "gotof", address_str, "", "" });
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         Symbol loop_iterator = state.operandStack.top();
         state.operandStack.pop();

         std::string temp_type = "int";
         int type_code = state.sc.getType(temp_type);

         int allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code));
         std::string address_str = '&' + std::to_string(allocatedAddress);

         state.quadruples.push_back({ "+", loop_iterator.address, "1", address_str });
         state.quadruples.push_back({ "=", address_str, "", loop_iterator.address });

         size_t for_false_jump = state.jumpStack.top();
         state.jumpStack.pop();

         size_t for_jump = state.jumpStack.top();
         state.jumpStack.pop();
         state.quadruples.push_back({ "goto", "", "", std::to_string(for_jump) });
         state.quadruples[for_false_jump].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   // Arrays

   template <>
   struct action<a_type_closearr>
   {

      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         int dimSize = std::stoi(state.operandStack.top().id);
         if (dimSize <= 0)
         {
            throw std::string("Declared array dimension with a non-positive size");
         }

         state.currDims.push_back({ dimSize, 1 });
         state.operandStack.pop();
      }
   };

   template <>
   struct action<a_var_bracketclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {

         const Symbol& indexed_value = state.operandStack.top();
         state.operandStack.pop();

         if (indexed_value.type != "int")
         {
            throw std::string("Array index is not an integer, got " + indexed_value.type);
         }

         const Symbol& operand = state.operandStack.top();

         if (operand.dims.empty() || operand.currDimPosition >= operand.dims.size())
         {
            throw std::string("Subscripted value is not an array");
         }

         Dim currDim = operand.dims[operand.currDimPosition];

         state.quadruples.push_back({ "ver", indexed_value.address, std::to_string(currDim.size), operand.address });

         int type_code = state.sc.getType("int");
         int allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code));
         std::string address_str = '&' + std::to_string(allocatedAddress);
         state.quadruples.push_back({ "*", indexed_value.id, std::to_string(currDim.displacement_size), address_str });

         // Alloc memory of type & (pointer)
         // int pointerAddress = state.memory.allocMemory(memory::TEMP, state.sc.getType("&"), state.sc.getTypeSize(type_code));
         // std::string pointerAddress_str = "&" + std::to_string(pointerAddress);
         // state.quadruples.push_back({ "&save", address_str, operand.address, pointerAddress_str });

         // Get value from displaced address and store in temp 
         // state.quadruples.push_back("&get", pointerAddress_str, "", )

         // Increase currDimPosition
         state.currDimPosition++;
      }
   };

   template <>
   struct action<a_var_dot>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.currDimPosition = 0;
      }
   };

   template <>
   struct action<var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.currDimPosition = 0;
      }
   };

   // Classes

   template <>
   struct action<t_this>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         if (!state.isInClass && state.isInFunction)
         {
            throw std::string("Invalid use of 'this' outside of a member function");
         }

         // TODO
      }
   };

   // Functions

   template <>
   struct action<t_fn>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.isInFunction = true;
      }
   };

   template <>
   struct action<funcdef>
   {

      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.isInFunction = false;
      }
   };

   template <>
   struct action<params_def>
   {

      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         // TODO
      }
   };

   // END

   template <>
   struct action<main_grammar>
   {
      template <typename ActionInput>
      static void apply(const ActionInput& in, State& state)
      {
         state.quadruples.push_back({ "end", "", "", "" });
      }
   };
}