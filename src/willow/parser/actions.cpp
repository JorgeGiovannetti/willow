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
         // Get value from string literal, removing quotations (")
         std::string filepath = state.operandStack.top().id.substr(1, state.operandStack.top().id.length() - 2);
         state.operandStack.pop();

         assert(!state.filepathStack.empty());

         // Current folder/path
         std::string currDirectory = state.filepathStack.top();

         // Get folder/parent path of imported file
         std::string importedDirectory = std::filesystem::path(filepath).parent_path().string();

         state.filepathStack.push(currDirectory + "/" + importedDirectory);

         try
         {
            // Nested parsing of imported file
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
         // Pop filepath on end of file
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
            // Create local scope
            state.currScopeKind = symbols::LOCAL;
            state.st->createScope(state.currScopeKind);

            // Cache current memory state
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
            // Exit scope
            state.st->exitScope();
            state.currScopeKind = state.st->getCurrentScopeKind();

            // Return to cached memory state from outer scope
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
         // TODO: THIS IS VERY SUS, CHECK OUT LATER
         try
         {
            // Pushes operand if the identifier exists in symbol table
            state.operandStack.push(state.st->lookup(in.string()));
         }
         catch (std::string msg)
         {
            // If it doesn't exist in symbol table yet, it pushes an operand without type or address
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

         // Calculate how much to displace per dimension
         for (int i = state.currDims.size() - 2; i >= 0; i--)
         {
            state.currDims[i].displacement_size = state.currDims[i + 1].size * state.currDims[i + 1].displacement_size;
         }

         // Calculate total amount of entries in structure (array / matrix / etc.)
         int dims_size = 1;
         for (Dim dim : state.operandStack.top().dims)
         {
            dims_size *= dim.size;
         }

         // Store variable at global memory segment, otherwise as local
         int memSegment = state.currScopeKind == symbols::GLOBAL ? memory::GLOBAL : memory::LOCAL;

         // Allocate memory for variable
         std::string address = state.memory.allocMemory(state.sc, memSegment, state.currType, dims_size, false);

         // Assign type, dimensions, and address to variable
         state.operandStack.top().type = state.currType;
         state.operandStack.top().dims = state.currDims;
         state.operandStack.top().address = address;

         // Clear currDims in state for next structured variable
         state.currDims.clear();
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
            // Inserts current operand to symbol table
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
            // Inserts current operand to symbol table
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
         // TODO: CHECK THIS OUT, KINDA SUS
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
         // If we don't assign a value, we pop operand and exit action
         if (state.operatorStack.empty() || state.operatorStack.top() != "=")
         {
            state.operandStack.pop();
            return;
         }

         // Get operator
         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         // Get right operand
         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();

         // Get left operand (variable we are assigning to)
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         if (state.memory.isPointer(op2.address))
         {
            // Get value from pointer address and store in temp
            std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, op2.type, 1, false);
            state.quadruples.push_back({"&get", op2.address, "", valueAddress});

            // Replace operand with pointed value
            op2.id = valueAddress;
            op2.address = valueAddress;
         }

         try
         {
            // Validate types match
            state.sc.query(op1.type, op2.type, "=");

            if (state.memory.isPointer(op1.address))
            {
               // If address is a pointer, we use &save to assign to pointed value
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

         // Get operator
         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         // Get right operand
         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();

         // Get left operand
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         // Add dims to type string for error messages
         std::string op1_type_with_dims = addDimsToType(op1.type, op1.dims, op1.currDimPosition);
         std::string op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);

         try
         {
            if (operation == "*=" || operation == "/=" || operation == "+=" || operation == "-=" || operation == "%=")
            {
               // Get operation to be executed before assignment (*, /, +, -, %)
               operation = {operation[0]};

               std::string temp_type = state.sc.query(op1_type_with_dims, op2_type_with_dims, operation);
               std::string address = state.memory.allocMemory(state.sc, memory::TEMP, temp_type, 1, false);

               state.quadruples.push_back({operation, op1.address, op2.address, address});

               op2.id = address;
               op2.address = address;

               // Add dims to type string for error messages
               op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);
            }

            // Verify types match
            state.sc.query(op1_type_with_dims, op2_type_with_dims, "=");

            if (state.memory.isPointer(op1.address))
            {
               // If address is a pointer, we use &save to assign to pointed value
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
         // Get resulting operand from expression
         Symbol expr_result = state.operandStack.top();
         state.operandStack.pop();

         // If expression result is not boolean, throw error
         if (expr_result.type != "bool")
         {
            throw pegtl::parse_error("Error: Expected boolean inside \"if-statement\"", in);
         }

         // Prepare gotof to fill in target address on conditional exit
         state.quadruples.push_back({"gotof", expr_result.address, "", ""});

         // Insert jump point to return to fill in gotof later
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

         // Assign jump point on condition end goto/gotof
         state.quadruples[conditional_end].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   template <>
   struct action<a3_conditional>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Prepare goto on else statement
         state.quadruples.push_back({"goto", "", "", ""});

         // Get jump point on false
         size_t false_jump = state.jumpStack.top();
         state.jumpStack.pop();

         // Add false jump to previously created gotof
         state.quadruples[false_jump].targetAddress = std::to_string(state.quadruples.size());

         // Push current jump point for end
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   // While Loops

   template <>
   struct action<a1_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Push jump point before loop condition
         state.jumpStack.push(state.quadruples.size());
      }
   };

   template <>
   struct action<a2_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Get resulting operand from expression
         Symbol expr_result = state.operandStack.top();
         state.operandStack.pop();

         // If expression result is not boolean, throw error
         if (expr_result.type != "bool")
         {
            throw pegtl::parse_error("Error: Expected boolean inside \"while-loop\" condition", in);
         }

         // Prepare gotof to fill in target address on loop exit
         state.quadruples.push_back({"gotof", expr_result.address, "", ""});

         // Insert jump point to return to fill in gotof later
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_while_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Get loop jump point
         size_t loop_jump_point = state.jumpStack.top();
         state.jumpStack.pop();

         // Get jump point before condition
         size_t loop_start = state.jumpStack.top();
         state.jumpStack.pop();

         // Add goto at end of loop that sends to before the condition
         state.quadruples.push_back({"goto", "", "", std::to_string(loop_start)});

         // Set current position (after loop) as destination for the loop jump point gotof
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
         // Push jump point before range target, after initializing iterator
         state.jumpStack.push(state.quadruples.size());
      }
   };

   template <>
   struct action<a2_for_range>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Get right operand
         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();

         // Get left operand
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();

         // TODO: CHECK THIS OUT, KINDA SUS
         // If the right operand didn't generate any quadruples, move jump point to right after
         if (state.jumpStack.top() == state.quadruples.size())
         {
            state.jumpStack.pop();
            state.jumpStack.push(state.quadruples.size() + 1);
         }

         // Check both range sides are integers
         if (op1.type != "int" || op2.type != "int")
         {
            throw pegtl::parse_error("Error: Expected int in for-loop range", in);
         }

         // Push operands back in their place
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

         // Check loop iterator is int
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
         // Range right side
         Symbol range_to = state.operandStack.top();
         state.operandStack.pop();

         // Range left side
         Symbol range_from = state.operandStack.top();
         state.operandStack.pop();

         Symbol loop_iterator = state.operandStack.top(); // Doesn't need pop, we need it on a3

         // Assign range left side to loop iterator
         state.quadruples.push_back({"=", range_from.address, "", loop_iterator.address});

         // Allocate boolean temp and check if our iterator <= range_to
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "bool", 1, false);
         state.quadruples.push_back({"<=", loop_iterator.address, range_to.address, address});

         // Prepare gotof for end of loop
         state.quadruples.push_back({"gotof", address, "", ""});
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Pop loop_iterator
         const Symbol &loop_iterator = state.operandStack.top();
         state.operandStack.pop();

         // Increase loop iterator by 1
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);
         state.quadruples.push_back({"+", loop_iterator.address, "1", address});
         state.quadruples.push_back({"=", address, "", loop_iterator.address});

         // Get condition false jump
         size_t for_false_jump = state.jumpStack.top();
         state.jumpStack.pop();

         // Get jump before condition
         size_t for_jump = state.jumpStack.top();
         state.jumpStack.pop();
         state.quadruples.push_back({"goto", "", "", std::to_string(for_jump)});

         // Fill in gotof to current position
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
         // Check if array dimension is valid
         int dimSize = std::stoi(state.operandStack.top().id);
         if (dimSize <= 0)
         {
            throw pegtl::parse_error("Declared array dimension with a non-positive size", in);
         }

         // Add dim to currDims
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
         // Get value at index
         const Symbol &index_value = state.operandStack.top();
         state.operandStack.pop();

         // Check index is integer
         if (index_value.type != "int")
         {
            throw std::string("Array index is not an integer, got " + index_value.type);
         }

         // Get variable we're indexing
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();

         // Check if the variable we're indexing is an array / current dimension is valid
         if (operand.dims.empty() || operand.currDimPosition >= operand.dims.size())
         {
            throw std::string("Subscripted value is not an array");
         }

         // Get current dimension and verify if the index value is valid
         const Dim &currDim = operand.dims[operand.currDimPosition];
         state.quadruples.push_back({"ver", index_value.address, std::to_string(currDim.size), operand.address});

         // Calculate pointer displacement for array indexing
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);
         state.quadruples.push_back({"*", index_value.address, std::to_string(currDim.displacement_size), address});

         // Add running displacement for dimensions > 0
         if (operand.currDimPosition > 0)
         {
            // Get running indexing
            Symbol runningIndexing = state.operandStack.top();
            state.operandStack.pop();

            // Add displacement to running indexing
            std::string address_tmp = address;
            address = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);
            state.quadruples.push_back({"+", runningIndexing.address, address_tmp, address});
         }

         // Push current displacement
         state.operandStack.push({address, "int", address});

         // Increase currDimPosition
         operand.currDimPosition++;

         // If this was the last dimension,
         if (operand.currDimPosition >= operand.dims.size())
         {
            // Alloc pointer to variable at index
            std::string pointerAddress = state.memory.allocMemory(state.sc, memory::TEMP, operand.type, 1, true);
            state.quadruples.push_back({"&disp", address, operand.address, pointerAddress});

            // Remove running indexing sum from operand stack
            state.operandStack.pop();

            // Push indexed-value address
            state.operandStack.push({pointerAddress, operand.type, pointerAddress});
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
         // TODO: CHECK LATER HOW TO HANDLE METHODS
         try
         {
            // Get operand
            Symbol operand = state.operandStack.top();
            state.operandStack.pop();

            // Get class signature of operand
            ClassSignature var_class = state.classdir.lookup(operand.type);

            // Check that the attribute exists
            if (!var_class.attributes.count(in.string()))
            {
               throw pegtl::parse_error("Attribute " + in.string() + " is not a part of type " + operand.type, in);
            }

            // Get attribute
            Attribute attr = var_class.attributes[in.string()];

            // Check that the attribute is public
            if (attr.access == "-")
            {
               throw pegtl::parse_error("Cannot access private attribute " + in.string() + " outside of its class " + operand.type, in);
            }

            // Alloc memory of type & (pointer)
            std::string pointerAddress = state.memory.allocMemory(state.sc, memory::TEMP, attr.type, 1, true);
            state.quadruples.push_back({"&disp", std::to_string(attr.position), operand.address, pointerAddress});

            // Push indexed-value address
            state.operandStack.push({pointerAddress, attr.type, pointerAddress});
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
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
         // Get attribute
         Symbol attrop = state.operandStack.top();
         state.operandStack.pop();

         Attribute attr;

         attr.access = state.currMemberAccess;
         attr.address = attrop.address;
         attr.currDimPosition = attrop.currDimPosition;
         attr.dims = attrop.dims;
         attr.id = attrop.id;
         attr.type = attrop.type;

         // Add attribute to class directory
         state.classdir.addAttribute(state.sc.typeManager, state.isInClass, attr);
      }
   };

   template <>
   struct action<classdef>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Add class to semantic cube and memory manager
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
         // Create empty class and set scope to class
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
         try
         {
            // Get parent class
            ClassSignature parentClass = state.classdir.lookup(in.string());

            // Add every attribute to child
            for (const auto &[key, attr] : parentClass.attributes)
            {
               state.classdir.addAttribute(state.sc.typeManager, state.isInClass, attr);
            }

            // Add every method to child
            for (const auto &[key, method] : parentClass.methods)
            {
               state.classdir.addMethod(key, method);
            }
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
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
         // Alloc temp to store return of read
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "string", 1, false);
         Quadruple quad = {"read", "", "", address};
         state.quadruples.push_back(quad);

         // Push result to operand stack
         state.operandStack.push({address, "string", address});
      }
   };

   template <>
   struct action<writeln_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Get operand
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();

         if (state.memory.isPointer(operand.address))
         {
            // Get value from pointer address and store in temp
            std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, operand.type, 1, false);
            state.quadruples.push_back({"&get", operand.address, "", valueAddress});

            operand.id = valueAddress;
            operand.address = valueAddress;
         }

         state.quadruples.push_back({"writeln", "", "", operand.address});
      }
   };

   template <>
   struct action<length_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Get operand
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();

         // Check if operand has dims and the current dimension is valid
         if (operand.dims.size() == 0 || operand.currDimPosition >= operand.dims.size())
         {
            throw pegtl::parse_error("Expected array in length function", in);
         }

         // Store current dimension size in temp address
         std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);
         state.quadruples.push_back({"=", std::to_string(operand.dims[operand.currDimPosition].size), "", valueAddress});

         state.operandStack.push({valueAddress, "int", valueAddress});
      }
   };

   template <>
   struct action<write_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Get operand
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();

         if (state.memory.isPointer(operand.address))
         {
            // Get value from pointer address and store in temp
            std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, operand.type, 1, false);
            state.quadruples.push_back({"&get", operand.address, "", valueAddress});

            operand.id = valueAddress;
            operand.address = valueAddress;
         }

         state.quadruples.push_back({"write", "", "", operand.address});
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