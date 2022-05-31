#include <iostream>

#include <stack>
#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "grammar.cpp"

namespace pegtl = tao::pegtl;

using willow::codegen::Quadruple;
using willow::symbols::SymbolTable;

namespace willow::parser
{

   struct operand
   {
      std::string id;
      willow::symbols::Type type;
   };

   struct state
   {
      std::stack<operand> operandStack;
      std::stack<std::string> operatorStack;
      willow::symbols::ScopeKind currScopeKind = willow::symbols::LOCAL;
      int tempCounter = 0;
      std::vector<Quadruple> quadruples;
      std::shared_ptr<SymbolTable> st = SymbolTable::instance();
   };

   template <typename Rule>
   struct action
   {
   };

   template <>
   struct action<a_open_scope>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         std::cout << "Creating scope" << std::endl;
         state.st->createScope(state.currScopeKind);
      }
   };

   template <>
   struct action<t_braceclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         std::cout << "Exiting scope" << std::endl;
         state.st->exitScope();
      }
   };

   template <>
   struct action<identifier>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.push({in.string(), willow::symbols::NONE_TYPE});
      }
   };

   template <>
   struct action<s_var_type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.top().type = {in.string()};
      }
   };

   template <>
   struct action<s_var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
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
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("!");
      }
   };

   template <>
   struct action<t_mult>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("*");
      }
   };

   template <>
   struct action<t_div>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("/");
      }
   };

   template <>
   struct action<t_mod>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("%");
      }
   };

   template <>
   struct action<t_plus>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("+");
      }
   };

   template <>
   struct action<t_minus>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("-");
      }
   };

   template <>
   struct action<t_lesser>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("<");
      }
   };

   template <>
   struct action<t_greater>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push(">");
      }
   };

   template <>
   struct action<t_leq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("<=");
      }
   };

   template <>
   struct action<t_geq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push(">=");
      }
   };

   template <>
   struct action<t_eq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("==");
      }
   };

   template <>
   struct action<t_neq>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("!=");
      }
   };

   template <>
   struct action<t_and>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("and");
      }
   };

   template <>
   struct action<t_or>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("or");
      }
   };

   template <>
   struct action<t_assign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("=");
      }
   };

   template <>
   struct action<t_multassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_divassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_modassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_plusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         // TODO
      }
   };

   template <>
   struct action<t_minusassign>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         // TODO
      }
   };

   // Literals

   template <>
   struct action<t_lit_int>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.push({in.string(), {"int"}});
      }
   };

   template <>
   struct action<t_lit_float>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.push({in.string(), {"float"}});
      }
   };

   template <>
   struct action<t_lit_bool>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.push({in.string(), {"bool"}});
      }
   };

   template <>
   struct action<t_lit_char>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.push({in.string(), {"char"}});
      }
   };

   template <>
   struct action<t_lit_string>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.operandStack.push({in.string(), {"string"}});
      }
   };

   // Expressions

   template <>
   struct action<var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
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
      static void apply(const ActionInput &in, state &state)
      {
         state.operatorStack.push("(");
      }
   };

   template <>
   struct action<expr_parclose>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         while (state.operatorStack.top() != "(")
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
               std::string tempAddress = std::to_string(state.tempCounter++);
               Quadruple quad = Quadruple(operation, op1.id, op2.id, tempAddress);
               state.quadruples.push_back(quad);
               std::cout << "Added quad: " << quad.to_string() << std::endl;
            }
         }
      }
   };

   template <>
   struct action<expr>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         while (!state.operatorStack.empty())
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
               std::string tempAddress = std::to_string(state.tempCounter++);
               Quadruple quad = Quadruple(operation, op1.id, op2.id, tempAddress);
               state.quadruples.push_back(quad);
               std::cout << "Added quad: " << quad.to_string() << std::endl;
            }
         }
      }
   };

   // Assignment

   template <>
   struct action<assignment>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         // TODO
      }
   };

}