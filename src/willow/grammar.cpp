#include <iostream>

#include <tao/pegtl.hpp>
#include "tokens.cpp"

using namespace tao::pegtl;

struct tipo
    : sor<t_int, t_float>
{
};

struct vars2
    : star<seq<t_comma, t_id>>
{
};

struct vars1
    : plus<seq<t_id, vars2, t_colon, tipo, t_semicolon>>
{
};

struct vars
    : seq<t_var, vars1>
{
};

struct var_cte
    : sor<t_id, t_cte_i, t_cte_f>
{
};

struct factor1
    : seq<opt<sor<t_plus, t_minus>>, var_cte>
{
};

struct expresion;

struct factor
    : sor<seq<t_paropen, expresion, t_parclose>, factor1>
{
};

struct termino1
    : star<seq<sor<t_mult, t_div>, factor>>
{
};

struct termino
    : seq<factor, termino1>
{
};

struct exp
    : seq<termino, star<seq<sor<t_plus, t_minus>, termino>>>
{
};

struct expresion
    : seq<exp, opt<seq<sor<t_greater, t_lesser, t_neq>, exp>>>
{
};

struct asignacion
    : seq<t_id, t_eq, expresion, t_semicolon>
{
};

struct escritura2
    : sor<expresion, t_cte_str>
{
};

struct escritura1
    : seq<escritura2, opt<seq<t_comma, escritura2>>>
{
};

struct escritura
    : seq<t_print, t_paropen, escritura1, t_parclose, t_semicolon>
{
};

struct bloque;

struct condicion
    : seq<t_if, t_paropen, expresion, t_parclose, bloque, opt<t_else, bloque>, t_semicolon>
{
};

struct estatuto
    : sor<asignacion, condicion, escritura>
{
};

struct bloque
    : seq<t_bracketopen, star<estatuto>, t_bracketclose>
{
};
struct programa
    : must<t_program, t_id, t_semicolon, opt<vars>, bloque>
{
};
