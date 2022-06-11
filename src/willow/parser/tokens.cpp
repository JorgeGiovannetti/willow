#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace willow::parser
{
    // clang-format off

    // Comments

    struct t_simplecomment : seq<one<'#'>, until<eolf>> {};
    struct t_multicomment : seq<string<'#', '*'>, until<string<'*', '#'>>> {};
    struct comment : disable<sor<t_multicomment, t_simplecomment>> {};

    // Separators (spaces/comments)

    struct sep : sor<ascii::space, comment> {};
    struct seps : star<sep> {};
    struct sepp : plus<sep> {};

    // Punctuation

    struct t_braceopen : one<'{'> {};
    struct t_braceclose : one<'}'> {};
    struct t_paropen : one<'('> {};
    struct t_parclose : one<')'> {};
    struct t_bracketopen : one<'['> {};
    struct t_bracketclose : one<']'> {};
    struct t_semicolon : one<';'> {};
    struct t_comma : one<','> {};
    struct t_dot : one<'.'> {};
    struct t_colon : one<':'> {};
    struct t_arrow : string<'-', '>'> {};
    struct t_rangedot : string<'.', '.'> {};

    // Operators

    struct t_not : one<'!'> {};
    struct t_mult : one<'*'> {};
    struct t_div : one<'/'> {};
    struct t_mod : one<'%'> {};
    struct t_plus : one<'+'> {};
    struct t_minus : one<'-'> {};
    struct t_lesser : one<'<'> {};
    struct t_greater : one<'>'> {};
    struct t_leq : string<'<', '='> {};
    struct t_geq : string<'>', '='> {};
    struct t_eq : string<'=', '='> {};
    struct t_neq : string<'!', '='> {};
    struct t_and : TAO_PEGTL_STRING("and") {};
    struct t_or : TAO_PEGTL_STRING("or") {};
    struct t_assign : one<'='> {};
    struct t_multassign : string<'*', '='> {};
    struct t_divassign : string<'/', '='> {};
    struct t_modassign : string<'%', '='> {};
    struct t_plusassign : string<'+', '='> {};
    struct t_minusassign : string<'-', '='> {};

    // Reserved keywords

    struct t_break : TAO_PEGTL_STRING("break") {};
    struct t_class : TAO_PEGTL_STRING("class") {};
    struct t_continue : TAO_PEGTL_STRING("continue") {};
    struct t_bool : TAO_PEGTL_STRING("bool") {};
    struct t_char : TAO_PEGTL_STRING("char") {};
    struct t_else : TAO_PEGTL_STRING("else") {};
    struct t_false : TAO_PEGTL_STRING("false") {};
    struct t_float : TAO_PEGTL_STRING("float") {};
    struct t_fn : TAO_PEGTL_STRING("fn") {};
    struct t_for : TAO_PEGTL_STRING("for") {};
    struct t_if : TAO_PEGTL_STRING("if") {};
    struct t_import : TAO_PEGTL_STRING("import") {};
    struct t_int : TAO_PEGTL_STRING("int") {};
    struct t_main : TAO_PEGTL_STRING("main") {};
    struct t_read : TAO_PEGTL_STRING("read") {};
    struct t_return : TAO_PEGTL_STRING("return") {};
    struct t_string : TAO_PEGTL_STRING("string") {};
    struct t_this : TAO_PEGTL_STRING("this") {};
    struct t_true : TAO_PEGTL_STRING("true") {};
    struct t_while : TAO_PEGTL_STRING("while") {};
    struct t_writeln : TAO_PEGTL_STRING("writeln") {};
    struct t_write : TAO_PEGTL_STRING("write") {};

    struct reserved : sor<
              t_break,
              t_class,
              t_continue,
              t_bool,
              t_char,
              t_else,
              t_false,
              t_float,
              t_fn,
              t_for,
              t_if,
              t_import,
              t_int,
              t_main,
              t_read,
              t_return,
              t_string,
              t_this,
              t_true,
              t_while,
              t_write,
              t_writeln,
              t_and,
              t_or>
    {
    };

    // Basic Type Literals

    struct t_lit_int : plus<digit> {};
    struct t_lit_float : seq<plus<digit>, one<'.'>, plus<digit>> {};
    struct t_lit_bool : sor<t_true, t_false> {};
    struct t_lit_char : seq<one<'\''>, any, one<'\''>> {};
    struct t_lit_string : seq<one<'\"'>, until<one<'\"'>>> {};

    // Identifier

    struct identifier_pattern : seq<alpha, star<sor<alnum, one<'_'>>>> {};
    struct t_id : seq<not_at<reserved>, identifier_pattern> {};

    // clang-format on
}