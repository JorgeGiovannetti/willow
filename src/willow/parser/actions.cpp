#include <iostream>

#include <tao/pegtl.hpp>
#include <willow/willow.hpp>
#include "grammar.cpp"

namespace pegtl = tao::pegtl;

using willow::symbols::SymbolTable;

namespace willow::parser
{

   struct state
   {
      std::string id_temp;
      willow::symbols::Type type_temp;
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
         state.id_temp = in.string();
      }
   };

   template <>
   struct action<type>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         state.type_temp.name = in.string();
      }
   };

   template <>
   struct action<s_var>
   {
      template <typename ActionInput>
      static void apply(const ActionInput &in, state &state)
      {
         std::cout << "Inserting id: " << state.id_temp << " with type: " << state.type_temp.name << std::endl;
         state.st->insert(state.id_temp, state.type_temp);
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