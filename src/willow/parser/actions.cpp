#include <iostream>
#include <filesystem>
#include <stack>

#include <tao/pegtl.hpp>

#include <willow/willow.hpp>

#include "grammar_errors.cpp"

namespace pegtl = tao::pegtl;

using willow::codegen::Quadruple;
using willow::symbols::Attribute;
using willow::symbols::ClassSignature;
using willow::symbols::Dim;
using willow::symbols::FunctionSignature;
using willow::symbols::Symbol;

namespace willow::parser
{

   std::string addDimsToType(std::string type, std::vector<Dim> dims, int currDim)
   {
      for (int i = currDim; i < dims.size(); i++)
      {
         type += "[" + std::to_string(dims[i].size) + "]";
      }
      return type;
   }

   void addUnaryOperation(State &state)
   {
      // Get operator
      std::string operation = state.operatorStack.top();
      state.operatorStack.pop();

      // Get operand
      Symbol op1 = state.operandStack.top();
      state.operandStack.pop();

      std::cout << "Popping operand of unary operation " << operation << ": " << op1.address << std::endl;

      // If operand is pointer, get its value
      if (state.memory.isPointer(op1.address))
      {
         // Alloc temp to store pointed value
         std::string valueAddress_str = state.memory.allocMemory(state.sc, memory::TEMP, op1.type, 1, false);
         state.quadruples.push_back({"&get", op1.address, "", valueAddress_str});

         // Replace operand with pointed value
         op1.id = valueAddress_str;
         op1.address = valueAddress_str;
      }

      // Get operation result type
      std::string result_type = state.sc.query(op1.type, "none", operation);

      // Store operation result in temp address
      std::string address_str = state.memory.allocMemory(state.sc, memory::TEMP, result_type, 1, false);
      state.quadruples.push_back({operation, op1.address, "", address_str});

      // Push result operand
      std::cout << "Pushing operand result of unary operation " << operation << ": " << address_str << std::endl;
      state.operandStack.push({address_str, result_type, address_str});
   }

   void addBinaryOperation(State &state)
   {
      // Get operator
      std::string operation = state.operatorStack.top();
      state.operatorStack.pop();

      // Get right operand
      Symbol op2 = state.operandStack.top();
      state.operandStack.pop();

      if (state.memory.isPointer(op2.address))
      {
         // Alloc temp to store pointed value
         std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, op2.type, 1, false);
         state.quadruples.push_back({"&get", op2.address, "", valueAddress});

         // Replace operand with pointed value
         op2.id = valueAddress;
         op2.address = valueAddress;
      }

      // Get left operand
      Symbol op1 = state.operandStack.top();
      state.operandStack.pop();

      // If operand is pointer, get its value
      if (state.memory.isPointer(op1.address))
      {
         // Alloc temp to store pointed value
         std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, op1.type, 1, false);
         state.quadruples.push_back({"&get", op1.address, "", valueAddress});

         // Replace operand with pointed value
         op1.id = valueAddress;
         op1.address = valueAddress;
      }

      // Add dims to type string for error messages
      std::string op1_type_with_dims = addDimsToType(op1.type, op1.dims, op1.currDimPosition);
      std::string op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);

      // Get operation result type
      std::string result_type = state.sc.query(op1_type_with_dims, op2_type_with_dims, operation);

      // Store operation result in temp address
      std::string resultAddress = state.memory.allocMemory(state.sc, memory::TEMP, result_type, 1, false);
      state.quadruples.push_back({operation, op1.address, op2.address, resultAddress});

      // Push result operand
      std::cout << "Pushing operand result of binary operation " << operation << ": " << resultAddress << std::endl;
      state.operandStack.push({resultAddress, result_type, resultAddress});
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
      static void apply(const ActionInput &in, State &state)
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
         catch (std::filesystem::filesystem_error &e)
         {
            throw pegtl::parse_error("Error: Failed to find file with path " + e.path1().string(), in);
         }
      }
   };

   template <>
   struct action<a_eof>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            state.currScopeKind = symbols::LOCAL;
            state.st->createScope(state.currScopeKind);
            state.memory.cacheCurrentMemstate(false);
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
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            state.st->exitScope();
            state.currScopeKind = state.st->getCurrentScopeKind();
            state.memory.deallocMemory();
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   // Vars

   template <>
   struct action<identifier>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            state.operandStack.push(state.st->lookup(in.string()));
         }
         catch (std::string msg)
         {
            state.operandStack.push({in.string(), willow::symbols::NONE_TYPE});
         }
      }
   };

   template <>
   struct action<type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.currType = in.string();
      }
   };

   template <>
   struct action<s_var_basic_type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.currType = in.string();
      }
   };

   template <>
   struct action<structured_type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {

         for (int i = state.currDims.size() - 2; i >= 0; i--)
         {
            state.currDims[i].displacement_size = state.currDims[i + 1].size * state.currDims[i + 1].displacement_size;
         }

         std::cout << "state currtype " << state.currType << std::endl;

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

         std::cout << "alloccing memory in segment " << memSegment << " with type " << type_code << " and dims_size " << dims_size << std::endl;
         int allocatedAddress = state.memory.allocMemory(memSegment, type_code, type_size * dims_size, false);
         std::string address_str = '&' + std::to_string(allocatedAddress);
         std::cout << "allocced " << address_str << std::endl;

         state.operandStack.top().address = address_str;
      }
   };

   template <>
   struct action<s_var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            const Symbol &operand = state.operandStack.top();
            state.st->insert(operand.id, operand.type, operand.address, operand.dims);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<s_var_basic>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            Symbol &operand = state.operandStack.top();
            operand.type = state.currType;
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
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_mult>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_div>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_mod>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_plus>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_minus>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_lesser>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_greater>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_leq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_geq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_eq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_neq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_and>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_or>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_assign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_multassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_divassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_modassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_plusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<t_minusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   // Literals

   template <>
   struct action<t_lit_int>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), "int", in.string()});
      }
   };

   template <>
   struct action<t_lit_float>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), "float", in.string()});
      }
   };

   template <>
   struct action<t_lit_bool>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), "bool", in.string()});
      }
   };

   template <>
   struct action<t_lit_char>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), "char", in.string()});
      }
   };

   template <>
   struct action<t_lit_string>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), "string", in.string()});
      }
   };

   // Expressions

   template <>
   struct action<expr_paropen>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operatorStack.push(in.string());
      }
   };

   template <>
   struct action<expr_parclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         while (state.operatorStack.top() != "(")
         {
            try
            {
               if (state.operatorStack.top() == "!")
               {
                  addUnaryOperation(state);
               }
               else
               {
                  addBinaryOperation(state);
               }
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
      {

         if (state.operatorStack.empty() || state.operatorStack.top() != "=")
         {
            state.operandStack.pop();
            return;
         }

         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();

         if (state.memory.isPointer(op2.address))
         {
            // Get value from pointer address and store in temp
            int valueType = state.sc.getType(op2.type);
            int valueAddress = state.memory.allocMemory(memory::TEMP, valueType, state.sc.getTypeSize(valueType), false);
            std::string valueAddress_str = "&" + std::to_string(valueAddress);
            state.quadruples.push_back({"&get", op2.address, "", valueAddress_str});

            op2 = {valueAddress_str, op2.type, valueAddress_str};
         }

         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         try
         {
            state.sc.query(op1.type, op2.type, "=");

            if (state.memory.isPointer(op1.address))
            {
               state.quadruples.push_back({"&save", op2.address, "", op1.address});
            }
            else
            {
               state.quadruples.push_back({"=", op2.address, "", op1.address});
            }
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
      static void apply(const ActionInput &in, State &state)
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

               int allocatedAddress = state.memory.allocMemory(memory::TEMP, state.sc.getType(temp_type), state.sc.getTypeSize(temp_type), false);
               std::string address_str = '&' + std::to_string(allocatedAddress);
               Quadruple quad = {operation, op1.address, op2.address, address_str};
               state.quadruples.push_back(quad);

               op2 = {address_str, temp_type, address_str};

               op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);
            }

            state.sc.query(op1_type_with_dims, op2_type_with_dims, "=");

            if (state.memory.isPointer(op1.address))
            {
               state.quadruples.push_back({"&save", op2.address, "", op1.address});
            }
            else
            {
               state.quadruples.push_back({"=", op2.address, "", op1.address});
            }
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
      static void apply(const ActionInput &in, State &state)
      {
         Symbol expr_result = state.operandStack.top();
         state.operandStack.pop();

         if (expr_result.type != "bool")
         {
            throw pegtl::parse_error("Error: Expected boolean inside \"if-statement\"", in);
         }

         state.quadruples.push_back({"gotof", expr_result.address, "", ""});
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a2_conditional>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
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
      static void apply(const ActionInput &in, State &state)
      {
         state.quadruples.push_back({"goto", "", "", ""});
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
      static void apply(const ActionInput &in, State &state)
      {
         state.jumpStack.push(state.quadruples.size());
      }
   };

   template <>
   struct action<a2_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol expr_result = state.operandStack.top();
         state.operandStack.pop();

         if (expr_result.type != "bool")
         {
            throw pegtl::parse_error("Error: Expected boolean inside \"while-loop\" condition", in);
         }

         state.quadruples.push_back({"gotof", expr_result.address, "", ""});
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         size_t loop_jump_point = state.jumpStack.top();
         state.jumpStack.pop();
         size_t loop_start = state.jumpStack.top();
         state.jumpStack.pop();

         state.quadruples.push_back({"goto", "", "", std::to_string(loop_start)});
         state.quadruples[loop_jump_point].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   // For Loops

   template <>
   struct action<a1_for_range>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.jumpStack.push(state.quadruples.size());
      }
   };

   template <>
   struct action<a2_for_range>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();

         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         if (state.jumpStack.top() == state.quadruples.size())
         {
            state.jumpStack.pop();
            state.jumpStack.push(state.quadruples.size() + 1);
         }

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
      static void apply(const ActionInput &in, State &state)
      {
         Symbol loop_iterator = state.operandStack.top();

         if (loop_iterator.type != "int")
         {
            throw pegtl::parse_error("Error: Non-int iterators are not (yet) supported", in);
         }
      }
   };

   template <>
   struct action<a2_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol range_to = state.operandStack.top();
         state.operandStack.pop();
         Symbol range_from = state.operandStack.top();
         state.operandStack.pop();
         Symbol loop_iterator = state.operandStack.top(); // Doesn't need pop, we need it on a3

         state.quadruples.push_back({"=", range_from.address, "", loop_iterator.address});

         int type_code = state.sc.getType("bool");
         int allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code), false);
         std::string address_str = '&' + std::to_string(allocatedAddress);

         state.quadruples.push_back({"<=", loop_iterator.address, range_to.address, address_str});

         state.quadruples.push_back({"gotof", address_str, "", ""});
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol loop_iterator = state.operandStack.top();
         state.operandStack.pop();

         int type_code = state.sc.getType("int");

         int allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code), false);
         std::string address_str = '&' + std::to_string(allocatedAddress);

         state.quadruples.push_back({"+", loop_iterator.address, "1", address_str});
         state.quadruples.push_back({"=", address_str, "", loop_iterator.address});

         size_t for_false_jump = state.jumpStack.top();
         state.jumpStack.pop();

         size_t for_jump = state.jumpStack.top();
         state.jumpStack.pop();
         state.quadruples.push_back({"goto", "", "", std::to_string(for_jump)});
         state.quadruples[for_false_jump].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   // Arrays

   template <>
   struct action<a_type_closearr>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         int dimSize = std::stoi(state.operandStack.top().id);
         if (dimSize <= 0)
         {
            throw pegtl::parse_error("Declared array dimension with a non-positive size", in);
         }

         state.currDims.push_back({dimSize, 1});
         state.operandStack.pop();
      }
   };

   template <>
   struct action<a_var_bracketclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {

         const Symbol &indexed_value = state.operandStack.top();
         state.operandStack.pop();

         if (indexed_value.type != "int")
         {
            throw std::string("Array index is not an integer, got " + indexed_value.type);
         }

         Symbol operand = state.operandStack.top();

         // Pop operand
         state.operandStack.pop();

         if (operand.dims.empty() || operand.currDimPosition >= operand.dims.size())
         {
            throw std::string("Subscripted value is not an array");
         }

         Dim currDim = operand.dims[operand.currDimPosition];

         state.quadruples.push_back({"ver", indexed_value.address, std::to_string(currDim.size), operand.address});

         // Calculate pointer displacement for array indexing

         int type_code = state.sc.getType("int");
         int allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code), false);
         std::string address_str = '&' + std::to_string(allocatedAddress);
         state.quadruples.push_back({"*", indexed_value.id, std::to_string(currDim.displacement_size), address_str});

         if (operand.currDimPosition > 0)
         {
            Symbol runningIndexing = state.operandStack.top();
            state.operandStack.pop();

            std::string address_tmp = address_str;
            allocatedAddress = state.memory.allocMemory(memory::TEMP, type_code, state.sc.getTypeSize(type_code), false);
            address_str = '&' + std::to_string(allocatedAddress);
            state.quadruples.push_back({"+", runningIndexing.address, address_tmp, address_str});
         }

         state.operandStack.push({address_str, "int", address_str});

         // Increase currDimPosition
         operand.currDimPosition++;

         if (operand.currDimPosition >= operand.dims.size())
         {

            // Alloc memory of type & (pointer)
            int valueType = state.sc.getType(operand.type);
            int pointerAddress = state.memory.allocMemory(memory::TEMP, state.sc.getTypeSize(valueType), state.sc.getTypeSize(type_code), true);
            std::string pointerAddress_str = "&" + std::to_string(pointerAddress);
            state.quadruples.push_back({"&disp", address_str, operand.address, pointerAddress_str});

            // Remove running indexing sum from operand stack
            state.operandStack.pop();

            // Push indexed-value address
            state.operandStack.push({pointerAddress_str, operand.type, pointerAddress_str});
         }
         else
         {
            // Push back in operand if more dims remain
            state.operandStack.push(operand);
         }
      }
   };

   template <>
   struct action<a_var_dotid>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();

         std::cout << "accessing attribute " << in.string() << " from variable " << operand.id << std::endl;
         std::cout << "address of variable is " << operand.address << std::endl;

         ClassSignature var_class = state.classdir.lookup(operand.type);

         if (!var_class.attributes.count(in.string()))
         {
            throw pegtl::parse_error("Attribute " + in.string() + " is not a part of type " + operand.type, in);
         }

         Attribute attr = var_class.attributes[in.string()];

         if (attr.access == "-")
         {
            throw pegtl::parse_error("Cannot access private attribute " + in.string() + " outside of its class " + operand.type, in);
         }

         // Alloc memory of type & (pointer)
         int valueType = state.sc.getType(attr.type);

         int pointerAddress = state.memory.allocMemory(memory::TEMP, state.sc.getTypeSize(valueType), state.sc.getTypeSize(valueType), true);
         std::string pointerAddress_str = "&" + std::to_string(pointerAddress);
         state.quadruples.push_back({"&disp", std::to_string(attr.position), operand.address, pointerAddress_str});

         std::cout << "alloccing pointer of type " << attr.type << " from variable address " << operand.address << std::endl;

         // Push indexed-value address
         state.operandStack.push({pointerAddress_str, attr.type, pointerAddress_str});
      }
   };

   // Classes

   template <>
   struct action<memberaccess>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.currMemberAccess = in.string();
      }
   };

   template <>
   struct action<classattr>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol attrop = state.operandStack.top();
         state.operandStack.pop();

         std::cout << "adding attribute " << attrop.id << " of type " << attrop.type << std::endl;

         Attribute attr;

         attr.access = state.currMemberAccess;
         attr.address = attrop.address;
         attr.currDimPosition = attrop.currDimPosition;
         attr.dims = attrop.dims;
         attr.id = attrop.id;
         attr.type = attrop.type;

         state.classdir.addAttribute(state.sc.typeManager, state.isInClass, attr);
      }
   };

   template <>
   struct action<classdef>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.sc.newType(state.isInClass, state.classdir.lookup(state.isInClass).size);
         state.memory.addType();
         state.isInClass = "";
      }
   };

   template <>
   struct action<a1_classdef>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.isInClass = in.string();
         state.classdir.insert({in.string()});
         state.st->createScope(symbols::CLASS);
         state.currScopeKind = symbols::CLASS;
      }
   };

   template <>
   struct action<a2_classdef>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         ClassSignature parentClass = state.classdir.lookup(in.string());

         for (const auto &[key, attr] : parentClass.attributes)
         {
            state.classdir.addAttribute(state.sc.typeManager, state.isInClass, attr);
         }
      }
   };

   template <>
   struct action<t_this>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.isInClass != "" && state.isInFunction)
         {
            throw std::string("Invalid use of 'this' outside of a member function");
         }

         // TODO
      }
   };

   // Built-in Functions

   template <>
   struct action<read_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         int allocatedAddress = state.memory.allocMemory(memory::TEMP, state.sc.getType("string"), state.sc.getTypeSize("string"), false);
         std::string address_str = '&' + std::to_string(allocatedAddress);
         Quadruple quad = {"read", "", "", address_str};
         state.quadruples.push_back(quad);

         state.operandStack.push({address_str, "string", address_str});
      }
   };

   template <>
   struct action<writeln_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         if (state.memory.isPointer(op1.address))
         {
            // Get value from pointer address and store in temp
            int valueType = state.sc.getType(op1.type);
            int valueAddress = state.memory.allocMemory(memory::TEMP, valueType, state.sc.getTypeSize(valueType), false);
            std::string valueAddress_str = "&" + std::to_string(valueAddress);
            state.quadruples.push_back({"&get", op1.address, "", valueAddress_str});

            op1 = {valueAddress_str, op1.type, valueAddress_str};
         }

         state.quadruples.push_back({"writeln", "", "", op1.address});
      }
   };

   template <>
   struct action<length_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         if (op1.dims.size() == 0 || op1.currDimPosition >= op1.dims.size())
         {
            throw pegtl::parse_error("Expected array in length function", in);
         }

         int valueType = state.sc.getType("int");
         int valueAddress = state.memory.allocMemory(memory::TEMP, valueType, state.sc.getTypeSize(valueType), false);
         std::string valueAddress_str = "&" + std::to_string(valueAddress);
         state.quadruples.push_back({"=", std::to_string(op1.dims[op1.currDimPosition].size), "", valueAddress_str});

         state.operandStack.push({valueAddress_str, "int", valueAddress_str});
      }
   };

   template <>
   struct action<write_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         if (state.memory.isPointer(op1.address))
         {
            // Get value from pointer address and store in temp
            int valueType = state.sc.getType(op1.type);
            int valueAddress = state.memory.allocMemory(memory::TEMP, valueType, state.sc.getTypeSize(valueType), false);
            std::string valueAddress_str = "&" + std::to_string(valueAddress);
            state.quadruples.push_back({"&get", op1.address, "", valueAddress_str});

            op1 = {valueAddress_str, op1.type, valueAddress_str};
         }

         state.quadruples.push_back({"write", "", "", op1.address});
      }
   };

   // Functions

   template <>
   struct action<t_fn>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.isInFunction = true;
      }
   };

   template <>
   struct action<funcdef>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.isInFunction = false;
      }
   };

   template <>
   struct action<params_def>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO
      }
   };

   // END

   template <>
   struct action<main_grammar>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO: Add goto -> main before ending run
         // state.quadruples.push_back({"goto", "", "", "main"});
         state.quadruples.push_back({"end", "", "", ""});
      }
   };
}