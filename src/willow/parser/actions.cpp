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

   void enterScope(State &state)
   {
      // Create local scope
      state.currScopeKind = symbols::LOCAL;
      state.st->createScope(state.currScopeKind);

      // Cache current memory state
      state.memory.cacheCurrentMemstate(false);
   }

   void exitScope(State &state)
   {
      // Exit scope
      state.st->exitScope();
      state.currScopeKind = state.st->getCurrentScopeKind();

      // Return to cached memory state from outer scope
      state.memory.deallocMemory();
   }

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
      std::cout << "Popping right operand of binary operation " << operation << ": " << op2.address << std::endl;

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
      std::cout << "Popping left operand of binary operation " << operation << ": " << op1.address << std::endl;

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
         std::cout << "Popping operand filepath in import: " << filepath << std::endl;

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
            enterScope(state);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<a_close_scope>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            exitScope(state);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   // Variable declaration

   template <>
   struct action<identifier>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.currId = in.string();
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

         // Store variable at global memory segment, otherwise as local
         int memSegment = state.currScopeKind == symbols::GLOBAL ? memory::GLOBAL : memory::LOCAL;

         // Allocate memory for variable
         std::string address = state.memory.allocMemory(state.sc, memSegment, state.currType, 1, false);

         // Assign type, dimensions, and address to variable
         std::cout << "pushing operand when defining an s_var_basic: " << address << std::endl;
         state.operandStack.push({state.currId, state.currType, address});
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
         for (Dim dim : state.currDims)
         {
            dims_size *= dim.size;
         }

         // Store variable at global memory segment, otherwise as local
         int memSegment = state.currScopeKind == symbols::GLOBAL ? memory::GLOBAL : memory::LOCAL;

         // Allocate memory for variable
         std::string address = state.memory.allocMemory(state.sc, memSegment, state.currType, dims_size, false);

         // Assign type, dimensions, and address to variable
         std::cout << "pushing operand when defining an s_var: " << address << std::endl;
         state.operandStack.push({state.currId, state.currType, address, state.currDims});

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
            std::cout << "inserting to symbol table: " << operand.id << " " << operand.type << " " << operand.address << std::endl;
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
            const Symbol &operand = state.operandStack.top();
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
         std::cout << "Pushing operand of literal int " << in.string() << std::endl;
         state.operandStack.push({in.string(), "int", in.string()});
      }
   };

   template <>
   struct action<t_lit_float>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Pushing operand of literal float " << in.string() << std::endl;
         state.operandStack.push({in.string(), "float", in.string()});
      }
   };

   template <>
   struct action<t_lit_bool>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Pushing operand of literal bool " << in.string() << std::endl;
         state.operandStack.push({in.string(), "bool", in.string()});
      }
   };

   template <>
   struct action<t_lit_char>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Pushing operand of literal char " << in.string() << std::endl;
         state.operandStack.push({in.string(), "char", in.string()});
      }
   };

   template <>
   struct action<t_lit_string>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Pushing operand of literal string " << in.string() << std::endl;
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
            std::cout << "Popping operand of var_def without assignment: " << state.operandStack.top().address << std::endl;
            state.operandStack.pop();
            return;
         }

         // Get operator
         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         // Get right operand
         Symbol op2 = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "Popping right operand of var_def assignment: " << op2.address << std::endl;

         // Get left operand (variable we are assigning to)
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "Popping left operand of var_def assignment: " << op1.address << std::endl;

         if (state.operandStack.empty())
         {
            std::cout << "var def assignment exiting with empty stack" << std::endl;
         }
         else
         {
            std::cout << "there's some leak somewhere, leaving this at top: " << state.operandStack.top().address << std::endl;
         }

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
         try
         {
            // Get operator
            std::string operation = state.operatorStack.top();
            state.operatorStack.pop();

            // Get right operand
            Symbol op2 = state.operandStack.top();
            state.operandStack.pop();
            std::cout << "Popping right operand of assignment operation " << operation << ": " << op2.address << std::endl;

            // Get left operand
            Symbol op1 = state.operandStack.top();
            state.operandStack.pop();
            std::cout << "Popping left operand of assignment operation " << operation << ": " << op1.address << std::endl;

            // Add dims to type string for error messages
            std::string op1_type_with_dims = addDimsToType(op1.type, op1.dims, op1.currDimPosition);
            std::string op2_type_with_dims = addDimsToType(op2.type, op2.dims, op2.currDimPosition);

            if (state.memory.isPointer(op2.address))
            {
               // Get value from pointer address and store in temp
               std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, op2.type, 1, false);
               state.quadruples.push_back({"&get", op2.address, "", valueAddress});

               // Replace operand with pointed value
               op2.id = valueAddress;
               op2.address = valueAddress;
            }

            if (operation == "*=" || operation == "/=" || operation == "+=" || operation == "-=" || operation == "%=")
            {
               // Get operation to be executed before assignment (*, /, +, -, %)
               operation = {operation[0]};

               std::string temp_type = state.sc.query(op1_type_with_dims, op2_type_with_dims, operation);
               std::string address = state.memory.allocMemory(state.sc, memory::TEMP, temp_type, 1, false);

               if (state.memory.isPointer(op1.address))
               {
                  // Get value from pointer address and store in temp
                  std::string valueAddress = state.memory.allocMemory(state.sc, memory::TEMP, op1.type, 1, false);
                  state.quadruples.push_back({"&get", op1.address, "", valueAddress});

                  state.quadruples.push_back({operation, valueAddress, op2.address, address});
               }
               else
               {
                  state.quadruples.push_back({operation, op1.address, op2.address, address});
               }

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
         std::cout << "Popping operand of expression result in a1_conditional: " << expr_result.address << std::endl;

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
         std::cout << "Popping operand of expression result in a2_while_loop: " << expr_result.address << std::endl;

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

         // Range left side
         Symbol range_from = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "Popping operand of range_from in a1_for_range: " << range_from.address << std::endl;

         Symbol loop_iterator = state.operandStack.top(); // Doesn't need pop, we need it on a3

         // Assign range left side to loop iterator
         state.quadruples.push_back({"=", range_from.address, "", loop_iterator.address});

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
         std::cout << "Popping right operand of a2_for_range: " << op2.address << std::endl;

         // Get left operand
         Symbol op1 = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "Popping left operand of a2_for_range: " << op1.address << std::endl;

         // Check both range sides are integers
         if (op1.type != "int" || op2.type != "int")
         {
            throw pegtl::parse_error("Error: Expected int in for-loop range", in);
         }

         std::cout << "for range got left operand with address " << op1.address << std::endl;
         std::cout << "for range got right operand with address " << op2.address << std::endl;
         if (state.operandStack.empty())
         {
            std::cout << "for range left operand stack empty " << std::endl;
         }
         else
         {
            std::cout << "for range left at top " << state.operandStack.top().address << std::endl;
         }
         // Push operands back in their place
         std::cout << "pushing operands of range: " << op1.address << " and " << op2.address << std::endl;
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
         std::cout << "Popping operand of right side of a2_for_loop: " << range_to.address << std::endl;

         Symbol loop_iterator = state.operandStack.top(); // Doesn't need pop, we need it on a3

         // Allocate boolean temp and check if our iterator <= range_to
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "bool", 1, false);
         state.quadruples.push_back({"<", loop_iterator.address, range_to.address, address});

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
         std::cout << "Popping operand of loop_iterator in a3_for_loop: " << loop_iterator.address << std::endl;

         // Increase loop iterator by 1
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);

         std::cout << "loop iterator address is " << loop_iterator.address << std::endl;
         std::cout << "temp iterator is " << address << std::endl;

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

         // Manually exit for scope
         try
         {
            exitScope(state);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   // Arrays

   template <>
   struct action<a_type_closebracket>
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
         std::cout << "Popping operand of a_type_close_arr: " << state.operandStack.top().address << std::endl;
         state.operandStack.pop();
      }
   };

   template <>
   struct action<a_var_id>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            state.operandStack.push(state.st->lookup(in.string()));
            std::cout << "pushing var operand: " << state.operandStack.top().id << " " << state.operandStack.top().address << std::endl;
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<a_var_bracketopen>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Push fake bottom
         state.operatorStack.push("[");
      }
   };

   template <>
   struct action<a_var_bracketclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Pop fake bottom
         state.operatorStack.pop();

         // Get value at index
         const Symbol &index_value = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "Popping operand of value at index in a_var_bracketclose: " << index_value.address << std::endl;

         // Check index is integer
         if (index_value.type != "int")
         {
            throw std::string("Array index is not an integer, got " + index_value.type);
         }

         // Get variable we're indexing
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "Popping operand of variable we're indexing in a_var_bracketclose: " << operand.address << std::endl;

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
            std::cout << "Popping operand of running index in a_var_bracketclose: " << runningIndexing.address << std::endl;

            // Add displacement to running indexing
            std::string address_tmp = address;
            address = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);
            state.quadruples.push_back({"+", runningIndexing.address, address_tmp, address});
         }

         // Push current displacement
         std::cout << "pushing current displacement: " << address << std::endl;
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
            std::cout << "Popping operand of running indexing sum in a_var_bracketclose: " << state.operandStack.top().address << std::endl;
            state.operandStack.pop();

            // Push indexed-value address
            std::cout << "pushing operand indexed-value address: " << pointerAddress << std::endl;
            state.operandStack.push({pointerAddress, operand.type, pointerAddress});
         }
         else
         {
            // Push back in operand if more dims remain
            std::cout << "pushing operand back in because more dims remain in array: " << operand.address << std::endl;
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

         // Clean displacement
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "popping operand in a_var_dot_id" << std::endl;

         for (int i = 0; i < operand.currDimPosition; i++)
         {
            std::cout << "popping currdimposition in a_var_dot_id" << i << std::endl;
            state.operandStack.pop();
         }

         // Push operand back in after cleanup
         std::cout << "pushing operand back in after cleanup" << std::endl;
         state.operandStack.push(operand);

         // TODO: CHECK LATER HOW TO HANDLE METHODS
         try
         {
            // Get operand
            Symbol operand = state.operandStack.top();
            state.operandStack.pop();
            std::cout << "Popping operand of a_var_dotid: " << operand.address << std::endl;

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
            std::cout << "pushing operand indexed-value of object: " << pointerAddress << std::endl;
            state.operandStack.push({pointerAddress, attr.type, pointerAddress});
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {

         // Clean displacement
         Symbol operand = state.operandStack.top();
         state.operandStack.pop();
         std::cout << "popping operand in var" << std::endl;

         for (int i = 0; i < operand.currDimPosition; i++)
         {
            std::cout << "popping currdimposition in var" << i << std::endl;
            state.operandStack.pop();
         }

         // Push operand back in after cleanup
         std::cout << "pushing operand back in after cleanup" << std::endl;
         state.operandStack.push(operand);
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
         std::cout << "Popping operand of attribute in classattr: " << attrop.address << std::endl;

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
         if (state.isInClass != "" && state.isInFunction != "")
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
         std::cout << "pushing operand result of read func call: " << address << std::endl;
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
         std::cout << "Popping operand of writeln_func_call: " << operand.address << std::endl;

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
         std::cout << "Popping operand of length_func_call: " << operand.address << std::endl;

         // Check if operand has dims and the current dimension is valid
         if (operand.dims.size() == 0 || operand.currDimPosition >= operand.dims.size())
         {
            throw pegtl::parse_error("Expected array in length function", in);
         }

         // Store current dimension size in temp address
         std::string address = state.memory.allocMemory(state.sc, memory::TEMP, "int", 1, false);
         state.quadruples.push_back({"=", std::to_string(operand.dims[operand.currDimPosition].size), "", address});

         std::cout << "pushing operand result of length func call: " << address << std::endl;
         state.operandStack.push({address, "int", address});
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
         std::cout << "Popping operand of write_func_call: " << operand.address << std::endl;

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

   // Main function

   template <>
   struct action<t_main>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.mainStart != -1)
         {
            throw pegtl::parse_error("Main already defined", in);
         }

         // Prepare goto before main so it gets executed at end
         state.quadruples.push_back({"goto", "", "", ""});
         state.jumpStack.push(state.quadruples.size() - 1);

         // Set main start point
         state.mainStart = state.quadruples.size();
      }
   };

   template <>
   struct action<main_func>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {

         // Go to end operation
         state.quadruples.push_back({"goto", "", "", ""});
         state.mainEnd = state.quadruples.size() - 1;

         // Get main start point
         size_t jump_main_start = state.jumpStack.top();
         state.jumpStack.pop();

         // Fill in previously set goto
         state.quadruples[jump_main_start].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   // Function definition

   template <>
   struct action<funcdef>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Clear current function
         state.isInFunction = "";

         state.quadruples.push_back({"endfunc", "", "", ""});

         // Get function start point
         size_t jump_func_start = state.jumpStack.top();
         state.jumpStack.pop();

         // Fill in previously set goto
         state.quadruples[jump_func_start].targetAddress = std::to_string(state.quadruples.size());
      }
   };

   template <>
   struct action<a1_funcdef>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {

            // Create function scope
            state.st->createScope(willow::symbols::FUNCTION);

            // Set current function to its identifier
            state.isInFunction = state.currId;

            // Prepare goto before function to avoid calling it on runtime during declaration
            state.quadruples.push_back({"goto", "", "", ""});
            state.jumpStack.push(state.quadruples.size() - 1);

            // Store function position to call later
            std::string func_position = std::to_string(state.quadruples.size());
            if (state.isInClass == "")
            {
               state.funcdir.insert({state.isInFunction, func_position});
            }
            else
            {
               // TODO: Handle class methods
               // state.classdir.addMethod(state.isInClass, {in.string(), func_position});
            }
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<a_params_def>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            Symbol param = state.operandStack.top();
            state.operandStack.pop();
            std::cout << "Popping operand in parameter definition" << std::endl;

            if (state.isInClass == "")
            {
               state.funcdir.addParam(state.sc.typeManager, param.id, param.type);
            }
            else
            {
               // TODO: Handle class methods
            }
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<a2_funcdef>
   {

      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         try
         {
            if (state.isInClass == "")
            {
               state.funcdir.addReturnType(state.sc.typeManager, state.isInFunction, state.currType);
            }
            else
            {
               // TODO: Handle class methods
            }
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   // Function calls

   template <>
   struct action<a1_func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {

         // Check that the function exists
         try
         {
            // TODO: Handle methods

            // Push into func call stacks to manage nested calls
            state.currFuncCall.push(state.funcdir.lookup(state.currId));
            state.currParam.push(0);
         }
         catch (std::string msg)
         {
            throw pegtl::parse_error(msg, in);
         }
      }
   };

   template <>
   struct action<func_call>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // Check all params were covered
         if (state.currParam.top() < state.currFuncCall.top().params.size())
         {
            throw pegtl::parse_error("Provided parameters do not match function signature", in);
         }

         // Call gosub
         state.quadruples.push_back({"gosub", "", "", state.currFuncCall.top().id});

         if (state.currFuncCall.top().return_type != "none")
         {
            // TODO: Copy return value to temp and add to operand stack
         }

         // Pop func call stacks
         state.currFuncCall.pop();
         state.currParam.pop();
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
         if (state.mainStart == -1)
         {
            throw pegtl::parse_error("Finished parsing, but no main function was defined", in);
         }

         state.quadruples.push_back({"goto", "", "", std::to_string(state.mainStart)});

         if (state.mainEnd >= 0 && state.mainEnd < state.quadruples.size())
         {
            state.quadruples[state.mainEnd].targetAddress = std::to_string(state.quadruples.size());
         }

         state.quadruples.push_back({"end", "", "", ""});
      }
   };
}