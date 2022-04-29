#include <iostream>

#include <stack>
#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "grammar.cpp"

namespace pegtl = tao::pegtl;

using willow::symbols::SymbolTable;

namespace willow::parser
{

   struct state
   {
      std::stack<std::string> idStack;
      std::stack<willow::symbols::Type> typeStack;
      willow::symbols::ScopeKind currScopeKind = willow::symbols::LOCAL;
      std::shared_ptr<SymbolTable> st = SymbolTable::instance();
   };

   template <typename Rule>
   struct action
   {
   };

   template <>
   struct action<t_braceopen>
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
         state.idStack.push(in.string());
      }
   };

   template <>
   struct action<type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.typeStack.push({in.string()});
      }
   };

   template <>
   struct action<s_var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         std::cout << "Inserting id: " << state.idStack.top() << " with type: " << state.typeStack.top().name << std::endl;
         state.st->insert(state.idStack.top(), state.typeStack.top());
      }
   };

   template <>
   struct action<assignment>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, std::shared_ptr<SymbolTable> &state)
      {
         // TODO
      }
   };

}