#include <iostream>

#include <tao/pegtl.hpp>

using namespace tao::pegtl;

struct t_comment
    : seq<string<'/', '/'>, until<eolf>>
{
};

struct t_ignored
    : sor<space, t_comment>
{
};

struct t_colon
    : pad<one<':'>, t_ignored>
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

struct t_greater
    : pad<one<'>'>, t_ignored>
{
};

struct t_lesser
    : pad<one<'<'>, t_ignored>
{
};

struct t_neq
    : pad<string<'<', '>'>, t_ignored>
{
};

struct t_eq
    : pad<one<'='>, t_ignored>
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

struct t_mult
    : pad<one<'*'>, t_ignored>
{
};

struct t_div
    : pad<one<'/'>, t_ignored>
{
};

struct t_bracketopen
    : pad<one<'{'>, t_ignored>
{
};

struct t_bracketclose
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

struct t_program
    : pad<string<'p', 'r', 'o', 'g', 'r', 'a', 'm'>, t_ignored>
{
};

struct t_if
    : pad<string<'i', 'f'>, t_ignored>
{
};

struct t_else
    : pad<string<'e', 'l', 's', 'e'>, t_ignored>
{
};

struct t_var
    : pad<string<'v', 'a', 'r'>, t_ignored>
{
};

struct t_int
    : pad<string<'i', 'n', 't'>, t_ignored>
{
};

struct t_float
    : pad<string<'f', 'l', 'o', 'a', 't'>, t_ignored>
{
};

struct t_print
    : pad<string<'p', 'r', 'i', 'n', 't'>, t_ignored>
{
};

struct t_cte_i
    : pad<plus<digit>, t_ignored>
{
};

struct t_cte_f
    : pad<seq<plus<digit>, one<'.'>, plus<digit>>, t_ignored>
{
};

struct t_id
    : pad<seq<alpha, star<sor<alnum, one<'_'>>>>, t_ignored>
{
};

struct t_cte_str
    : pad<seq<one<'"'>, until<one<'"'>>>, t_ignored>
{
};
