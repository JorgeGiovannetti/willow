#include <iostream>
#include <filesystem>

#include <stack>
#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "grammar_errors.cpp"

namespace pegtl = tao::pegtl;

using willow::codegen::Quadruple;
using namespace willow;

namespace willow::parser
{

   void addBinaryOperation(State &state)
   {

      std::string operation = state.operatorStack.top();
      state.operatorStack.pop();

      operand op2 = state.operandStack.top();
      state.operandStack.pop();

      operand op1 = state.operandStack.top();
      state.operandStack.pop();

      std::cout << "Adding binary operation " << op1.id << " " << operation << " " << op2.id << std::endl;

      // TODO: Type-checking via semantic cube
      if (true)
      {
         std::string tempAddress = 't' + std::to_string(state.tempCounter++);
         Quadruple quad = {operation, op1.id, op2.id, tempAddress};
         state.quadruples.push_back(quad);
         symbols::Type temp_type = op1.type; // TODO: get type from semantic cube
         state.operandStack.push({tempAddress, temp_type});
      }
      else
      {
         throw "ERROR: type mismatch";
      }
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
         std::string filepath = state.operandStack.top().id;
         state.operandStack.pop();

         assert(!state.filepathStack.empty());

         std::string currDirectory = state.filepathStack.top();
         std::string importedDirectory = std::filesystem::path(filepath).parent_path().string();

         state.filepathStack.push(currDirectory + "/" + importedDirectory);

         try
         {
            pegtl::file_input importedIn(currDirectory + "/" + filepath);
            pegtl::parse_nested<main_grammar, action>(in, importedIn, state);
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
            state.st->createScope(state.currScopeKind);
         }
         catch (const char *msg)
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
         state.st->exitScope();
      }
   };

   // Vars

   template <>
   struct action<identifier>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Pushing identifier " << in.string() << std::endl;
         state.operandStack.push({in.string(), willow::symbols::NONE_TYPE});
      }
   };

   template <>
   struct action<s_var_type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Adding type to identifier " << in.string() << std::endl;
         state.operandStack.top().type = {in.string()};
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
            state.st->insert(state.operandStack.top().id, state.operandStack.top().type);
         }
         catch (const char *msg)
         {
            throw pegtl::parse_error(msg, in);
         }
         std::cout << "Added s_var to symbol table!" << std::endl;
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
         std::cout << "Pushing int literal " << in.string() << std::endl;
         state.operandStack.push({in.string(), {"int"}});
      }
   };

   template <>
   struct action<t_lit_float>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Pushing float literal" << in.string() << std::endl;
         state.operandStack.push({in.string(), {"float"}});
      }
   };

   template <>
   struct action<t_lit_bool>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), {"bool"}});
      }
   };

   template <>
   struct action<t_lit_char>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), {"char"}});
      }
   };

   template <>
   struct action<a_lit_string>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), {"string"}});
      }
   };

   // Expressions

   template <>
   struct action<var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO: Object Attributes and Arrays
         // Probably should refactor var to just ids and add . and [] operators
         // try
         // {
         //    symbols::Symbol symbol = state.st->lookup(in.string());
         //    state.operandStack.push({symbol.id, symbol.type});
         // }
         // catch (const char *msg)
         // {
         //    throw pegtl::parse_error(msg, in);
         // }
      }
   };

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
            addBinaryOperation(state);
         }

         // Stack top will always be "(" at this point
         state.operatorStack.pop();
      }
   };

   template <>
   struct action<expr>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == "or")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L7>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == "and")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L6>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == "!=" || state.operatorStack.top() == "==")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L5>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == ">=" || state.operatorStack.top() == "<=" || state.operatorStack.top() == ">" || state.operatorStack.top() == "<")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L4>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == "+" || state.operatorStack.top() == "-")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L3>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == "*" || state.operatorStack.top() == "/" || state.operatorStack.top() == "%")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L2>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         if (state.operatorStack.top() == "-" || state.operatorStack.top() == "!")
         {
            addBinaryOperation(state);
         }
      }
   };

   template <>
   struct action<a1_expr_L1>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TO-DO
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
            return;
         }

         std::cout << "Assignment inside var_def!" << std::endl;

         operand op2 = state.operandStack.top();
         state.operandStack.pop();
         operand op1 = state.operandStack.top();
         state.operandStack.pop();
         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         // TODO: Type-checking via semantic cube
         if (true)
         {
            Quadruple quad = {operation, op2.id, "", op1.id};
            state.quadruples.push_back(quad);
         }
      }
   };

   template <>
   struct action<assignment>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Assignment!" << std::endl;

         operand op2 = state.operandStack.top();
         state.operandStack.pop();
         operand op1 = state.operandStack.top();
         state.operandStack.pop();
         std::string operation = state.operatorStack.top();
         state.operatorStack.pop();

         if (operation == "*=" || operation == "/=" || operation == "+=" || operation == "-=" || operation == "%=")
         {
            // TODO: Type-checking via semantic cube
            if (true)
            {
               std::string tempAddress = 't' + std::to_string(state.tempCounter++);
               Quadruple quad = {operation.substr(0, 1), op1.id, op2.id, tempAddress};
               state.quadruples.push_back(quad);
               symbols::Type temp_type = op1.type; // TODO: get type from semantic cube
               state.operandStack.push({tempAddress, temp_type});
               op2 = state.operandStack.top();

               operation = "=";
            }
            else
            {
               throw "ERROR: type mismatch";
            }
         }

         // TODO: Type-checking via semantic cube
         if (operation == "=" && true)
         {
            Quadruple quad = {operation, op2.id, "", op1.id};
            state.quadruples.push_back(quad);
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
         operand expr_result = state.operandStack.top();
         state.operandStack.pop();

         if (expr_result.type.name != "bool")
         {
            throw "Error: Expected boolean inside \"if-statement\"";
         }

         state.quadruples.push_back({"GOTOF", expr_result.id, "", ""});
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
         state.quadruples.push_back({"GOTO", "", "", ""});
         size_t false_jump = state.jumpStack.top();
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
         operand expr_result = state.operandStack.top();
         state.operandStack.pop();

         if (expr_result.type.name != "bool")
         {
            throw "Error: Expected boolean inside \"while-loop\" condition";
         }

         state.quadruples.push_back({"GOTOF", expr_result.id, "", ""});
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

         state.quadruples.push_back({"GOTO", "", "", std::to_string(loop_start)});
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
         operand op2 = state.operandStack.top();
         state.operandStack.pop();
         operand op1 = state.operandStack.top();
         state.operandStack.pop();

         if (op1.type.name != "int" || op2.type.name != "int")
         {
            throw "Error: Expected int in for-loop range";
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
         operand loop_iterator = state.operandStack.top();

         if (loop_iterator.type.name != "int")
         {
            throw "Error: Non-int iterators are not (yet) supported.";
         }
      }
   };

   template <>
   struct action<a2_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         operand range_to = state.operandStack.top();
         state.operandStack.pop();
         operand range_from = state.operandStack.top();
         state.operandStack.pop();
         operand loop_iterator = state.operandStack.top();

         state.quadruples.push_back({"=", range_from.id, "", loop_iterator.id});

         state.jumpStack.push(state.quadruples.size());

         std::string tempVar = "t" + std::to_string(state.tempCounter++);
         state.quadruples.push_back({"<", loop_iterator.id, range_to.id, tempVar});
         state.quadruples.push_back({"GOTOF", tempVar, "", ""});
         state.jumpStack.push(state.quadruples.size() - 1);
      }
   };

   template <>
   struct action<a3_for_loop>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         operand loop_iterator = state.operandStack.top();
         state.operandStack.pop();

         std::string tempVar1 = "t" + std::to_string(state.tempCounter++);
         state.quadruples.push_back({"+", loop_iterator.id, "1", tempVar1}); // TODO: Check how to display constants in quads
         state.quadruples.push_back({"=", tempVar1, "", loop_iterator.id});
         state.jumpStack.push(state.quadruples.size() - 1);

         size_t for_jump = state.jumpStack.top();
         state.jumpStack.pop();

         state.quadruples.push_back({"GOTO", "", "", std::to_string(for_jump)});

         size_t for_false_jump = state.jumpStack.top();
         state.jumpStack.pop();
         state.quadruples[for_jump].targetAddress = std::to_string(state.quadruples.size());
      }
   };
}