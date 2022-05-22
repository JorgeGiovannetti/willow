#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace willow::parser
{
    // Ignored

    struct t_comment
        : seq<one<'#'>, until<eolf>>
    {
    };

    struct t_multicomment
        : seq<string<'#', '*'>, until<string<'*', '#'>>>
    {
    };

    struct t_ignored
        : sor<t_multicomment, t_comment, space>
    {
    };

    // Punctuation

    struct t_braceopen
        : pad<one<'{'>, t_ignored>
    {
    };

    struct t_braceclose
        : pad<one<'}'>, t_ignored>
    {
    };

    struct t_paropen
        : pad<one<'('>, t_ignored>
    {
    };

    struct t_parclose
        : pad<one<')'>, t_ignored>
    {
    };

    struct t_bracketopen
        : pad<one<'['>, t_ignored>
    {
    };

    struct t_bracketclose
        : pad<one<']'>, t_ignored>
    {
    };

    struct t_semicolon
        : pad<one<';'>, t_ignored>
    {
    };

    struct t_comma
        : pad<one<','>, t_ignored>
    {
    };

    struct t_dot
        : pad<one<'.'>, t_ignored>
    {
    };

    struct t_colon
        : pad<one<':'>, t_ignored>
    {
    };

    struct t_arrow
        : pad<string<'-', '>'>, t_ignored>
    {
    };

    struct t_rangedot
        : pad<string<'.', '.'>, t_ignored>
    {
    };

    // Operators

    struct t_not
        : pad<one<'!'>, t_ignored>
    {
    };

    struct t_mult
        : pad<one<'*'>, t_ignored>
    {
    };

    struct t_div
        : pad<one<'/'>, t_ignored>
    {
    };

    struct t_mod
        : pad<one<'%'>, t_ignored>
    {
    };

    struct t_plus
        : pad<one<'+'>, t_ignored>
    {
    };

    struct t_minus
        : pad<one<'-'>, t_ignored>
    {
    };

    struct t_lesser
        : pad<one<'<'>, t_ignored>
    {
    };

    struct t_greater
        : pad<one<'>'>, t_ignored>
    {
    };

    struct t_leq
        : pad<string<'<', '='>, t_ignored>
    {
    };

    struct t_geq
        : pad<string<'>', '='>, t_ignored>
    {
    };

    struct t_eq
        : pad<string<'=', '='>, t_ignored>
    {
    };

    struct t_neq
        : pad<string<'!', '='>, t_ignored>
    {
    };

    struct t_and
        : pad<string<'a', 'n', 'd'>, t_ignored>
    {
    };

    struct t_or
        : pad<string<'o', 'r'>, t_ignored>
    {
    };

    struct t_assign
        : pad<one<'='>, t_ignored>
    {
    };

    struct t_multassign
        : pad<string<'*', '='>, t_ignored>
    {
    };

    struct t_divassign
        : pad<string<'/', '='>, t_ignored>
    {
    };

    struct t_modassign
        : pad<string<'%', '='>, t_ignored>
    {
    };

    struct t_plusassign
        : pad<string<'+', '='>, t_ignored>
    {
    };

    struct t_minusassign
        : pad<string<'-', '='>, t_ignored>
    {
    };

    // Reserved keywords

    struct t_break
        : pad<string<'b', 'r', 'e', 'a', 'k'>, t_ignored>
    {
    };

    struct t_class
        : pad<string<'c', 'l', 'a', 's', 's'>, t_ignored>
    {
    };

    struct t_continue
        : pad<string<'c', 'o', 'n', 't', 'i', 'n', 'u', 'e'>, t_ignored>
    {
    };

    struct t_bool
        : pad<string<'b', 'o', 'o', 'l'>, t_ignored>
    {
    };

    struct t_char
        : pad<string<'c', 'h', 'a', 'r'>, t_ignored>
    {
    };

    struct t_else
        : pad<string<'e', 'l', 's', 'e'>, t_ignored>
    {
    };

    struct t_false
        : pad<string<'f', 'a', 'l', 's', 'e'>, t_ignored>
    {
    };

    struct t_float
        : pad<string<'f', 'l', 'o', 'a', 't'>, t_ignored>
    {
    };

    struct t_fn
        : pad<string<'f', 'n'>, t_ignored>
    {
    };

    struct t_for
        : pad<string<'f', 'o', 'r'>, t_ignored>
    {
    };

    struct t_if
        : pad<string<'i', 'f'>, t_ignored>
    {
    };

    struct t_import
        : pad<string<'i', 'm', 'p', 'o', 'r', 't'>, t_ignored>
    {
    };

    struct t_int
        : pad<string<'i', 'n', 't'>, t_ignored>
    {
    };

    struct t_main
        : pad<string<'m', 'a', 'i', 'n'>, t_ignored>
    {
    };

    struct t_read
        : pad<string<'r', 'e', 'a', 'd'>, t_ignored>
    {
    };

    struct t_return
        : pad<string<'r', 'e', 't', 'u', 'r', 'n'>, t_ignored>
    {
    };

    struct t_super
        : pad<string<'s', 'u', 'p', 'e', 'r'>, t_ignored>
    {
    };

    struct t_string
        : pad<string<'s', 't', 'r', 'i', 'n', 'g'>, t_ignored>
    {
    };

    struct t_this
        : pad<string<'t', 'h', 'i', 's'>, t_ignored>
    {
    };

    struct t_true
        : pad<string<'t', 'r', 'u', 'e'>, t_ignored>
    {
    };

    struct t_while
        : pad<string<'w', 'h', 'i', 'l', 'e'>, t_ignored>
    {
    };

    struct t_write
        : pad<string<'w', 'r', 'i', 't', 'e'>, t_ignored>
    {
    };

    struct reserved
        : sor<
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
              t_super,
              t_string,
              t_this,
              t_true,
              t_while,
              t_write,
              t_and,
              t_or>
    {
    };

    // Basic Type Literals

    struct t_lit_int
        : pad<plus<digit>, t_ignored>
    {
    };

    struct t_lit_float
        : pad<seq<plus<digit>, one<'.'>, plus<digit>>, t_ignored>
    {
    };

    struct t_lit_bool
        : pad<sor<t_true, t_false>, t_ignored>
    {
    };

    struct t_lit_char
        : pad<seq<one<'\''>, any, one<'\''>>, t_ignored>
    {
    };

    struct t_lit_string
        : pad<seq<one<'"'>, until<one<'"'>>>, t_ignored>
    {
    };

    // Identifier

    struct identifier_pattern
        : seq<alpha, star<sor<alnum, one<'_'>>>>
    {
    };

    struct t_id
        : pad<minus<identifier_pattern, reserved>, t_ignored>
    {
    };
}