#include <tao/pegtl.hpp>
#include "grammar.cpp"

using namespace tao::pegtl;

namespace willow::parser
{

    // clang-format off

    template< typename > inline constexpr const char* error_message = nullptr;

    template<> inline constexpr auto error_message<t_semicolon> = "expected a ';'";
    template<> inline constexpr auto error_message<a_eof> = "unexpected character before end of file";

    template<> inline constexpr auto error_message<params_def> = "unexpected character inside function declaration parameters";
    template<> inline constexpr auto error_message<funcdef> = "unexpected character inside function declaration";
    template<> inline constexpr auto error_message<params> = "unexpected character inside function parameters";
    template<> inline constexpr auto error_message<main_func> = "unexpected character inside main";
    template<> inline constexpr auto error_message<read_func_call> = "unexpected character inside call to read function";
    template<> inline constexpr auto error_message<write_func_call> = "unexpected character inside call to write function";
    template<> inline constexpr auto error_message<writeln_func_call> = "unexpected character inside call to writeln function";
    template<> inline constexpr auto error_message<length_func_call> = "unexpected character inside call to length function";
    template<> inline constexpr auto error_message<funcs> = "unexpected character inside function call";
    template<> inline constexpr auto error_message<classdef> = "unexpected character inside class definition";
    template<> inline constexpr auto error_message<conditional> = "unexpected character inside \"if\" statement";
    template<> inline constexpr auto error_message<while_loop> = "unexpected character inside \"while\"-loop statement";
    template<> inline constexpr auto error_message<for_loop> = "unexpected character inside \"for\"-loop statement";
    template<> inline constexpr auto error_message<assignment> = "unexpected character inside assignment";
    template<> inline constexpr auto error_message<return_stmt> = "unexpected character inside return statement";
    
    template<> inline constexpr auto error_message<grammar> = "unexpected character";
    
    // clang-format on

    // As must_if<> can not take error_message as a template parameter directly, we need to wrap it.
    struct error
    {
        template <typename Rule>
        static constexpr auto message = error_message<Rule>;
    };

    template <typename Rule>
    using control = must_if<error>::control<Rule>;
}