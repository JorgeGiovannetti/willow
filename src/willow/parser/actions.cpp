#include <iostream>

#include <stack>
#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "grammar.cpp"

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

      // TODO: Type-checking via semantic cube
      if (true)
      {
         if (operation == "=")
         {
            Quadruple quad = {operation, op1.address, "", op2.address};
            state.quadruples.push_back(quad);
         }
         else
         {
            std::string tempAddress = 't' + std::to_string(state.tempCounter++);
            Quadruple quad = {operation, op1.address, op2.address, tempAddress};
            state.quadruples.push_back(quad);
            symbols::Type temp_type = op1.type; // TODO: get type from semantic cube
            state.operandStack.push({tempAddress, temp_type});
         }
      }
      else{
         throw "ERROR: type mismatch";
      }
   }

   template <typename Rule>
   struct action
   {
   };

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
   struct action<t_lit_string>
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
         if(state.operatorStack.top() == "or"){
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
         if(state.operatorStack.top() == "and"){
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
         if(state.operatorStack.top() == "!=" || state.operatorStack.top() == "=="){
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
         if(state.operatorStack.top() == ">=" || state.operatorStack.top() == "<=" 
            || state.operatorStack.top() == ">" || state.operatorStack.top() == "<"){
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
         if(state.operatorStack.top() == "+" || state.operatorStack.top() == "-"){
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
         if(state.operatorStack.top() == "*" || state.operatorStack.top() == "/" || state.operatorStack.top() == "%"){
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
         if(state.operatorStack.top() == "-" || state.operatorStack.top() == "!"){
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
         //TO-DO
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

}