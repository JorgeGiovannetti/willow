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

      operand op1 = state.operandStack.top();
      state.operandStack.pop();

      operand op2 = state.operandStack.top();
      state.operandStack.pop();

      // TODO: Type-checking via semantic cube
      if (true)
      {
         if (operation == "=")
         {
            Quadruple quad = {operation, op1.id, "", op2.id};
            state.quadruples.push_back(quad);
         }
         else
         {
            std::string tempAddress = 't' + std::to_string(state.tempCounter++);
            Quadruple quad = {operation, op1.id, op2.id, tempAddress};
            state.quadruples.push_back(quad);
            symbols::Type temp_type = op1.type; // TODO: get type from semantic cube
            state.operandStack.push({tempAddress, temp_type});
         }
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

         std::cout << "Importing file: " << filepath << std::endl;

         std::string currDirectory = state.filepathStack.top();
         std::string importedDirectory = std::filesystem::path(filepath).parent_path().string();

         state.filepathStack.push(currDirectory + "/" + importedDirectory);

         pegtl::file_input importedIn(currDirectory + "/" + filepath);
         pegtl::parse_nested<main_grammar, action>(in, importedIn, state);
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
         std::cout << "Creating scope" << std::endl;
         state.st->createScope(state.currScopeKind);
      }
   };

   template <>
   struct action<t_braceclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Exiting scope" << std::endl;
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
         state.operandStack.push({in.string(), willow::symbols::NONE_TYPE});
      }
   };

   template <>
   struct action<s_var_type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.top().type = {in.string()};
      }
   };

   template <>
   struct action<s_var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         std::cout << "Inserting id: " << state.operandStack.top().id << " with type: " << state.operandStack.top().type.name << std::endl;
         state.st->insert(state.operandStack.top().id, state.operandStack.top().type);
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
         // TODO
      }
   };

   template <>
   struct action<t_divassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_modassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_plusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_minusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO
      }
   };

   // Literals

   template <>
   struct action<t_lit_int>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         state.operandStack.push({in.string(), {"int"}});
      }
   };

   template <>
   struct action<t_lit_float>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
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
         symbols::Symbol symbol = state.st->lookup(in.string());
         state.operandStack.push({symbol.id, symbol.type});
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
         while (!state.operatorStack.empty())
         {
            addBinaryOperation(state);
         }
      }
   };

   // Assignment

   template <>
   struct action<assignment>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, State &state)
      {
         // TODO
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