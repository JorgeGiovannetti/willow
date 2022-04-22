#include <tao/pegtl.hpp>
#include "tokens.cpp"

using namespace tao::pegtl;

struct block;
struct expr;

// Imports

struct imports
    : seq<t_import, one<'('>, list<t_lit_string, t_comma>, one<')'>>
{
};

// Type

struct type
    : seq<sor<t_int, t_float, t_bool, t_char, t_string, t_id>, star<seq<t_bracketopen, t_bracketclose>>>
{
};

// Literals

struct t_lit_arr
    : seq<t_bracketopen, opt<list<expr, t_comma>>, t_bracketclose> 
{
};

struct literal
    : sor<t_lit_int, t_lit_float, t_lit_bool, t_lit_char, t_lit_string, t_lit_arr>
{
};

// Vars

struct var
    : seq<sor<t_id, t_this>, star<sor<seq<t_bracketopen, expr, t_bracketclose>, seq<t_dot, t_id>>>>
{
};

struct s_var
    : seq<t_id, t_colon, type>
{
};

struct var_def
    : seq<s_var, opt<seq<t_assign, expr>>>
{
};

struct var_def_stmt
    : seq<var_def, t_semicolon>
{
};

// Functions

struct params_def
    : seq<t_paropen, opt<list<var_def, t_comma>>, t_parclose>
{
}; 

struct params
    : seq<t_paropen, opt<list<expr, t_comma>>, t_parclose>
{
};

struct funcdef
    : seq<t_fn, t_id, params_def, opt<t_colon, type>, block>
{
};

struct main_func
    : seq<t_fn, t_main, t_paropen, t_parclose, block>
{
};

struct func_call
    : seq<var, params>
{
};

struct read_func_call
    : seq<t_read, t_paropen, t_parclose>
{
};

struct write_func_call
    : seq<t_write, params>
{
};

struct super_func_call
    : seq<t_super, params>
{
};

struct funcs
    : seq<sor<read_func_call, write_func_call, super_func_call, func_call>, t_semicolon>
{
};

// Classes

struct constructor
    : seq<t_id, params_def, block>
{
};

struct attribute
    : seq<sor<t_plus, t_minus>, sor<var_def_stmt, funcdef>>
{
};

struct classdef
    : seq<t_class, t_id, opt<seq<t_arrow, t_id>>, t_braceopen, plus<constructor>, plus<attribute>, t_paropen>
{
};


// Conditionals

struct conditional;

struct conditional
    : seq<t_if, t_paropen, expr, t_parclose, block, opt<t_else, sor<block, conditional>>>
{
};

// Loops

struct while_loop
    : seq<t_while, t_paropen, expr, t_parclose, block>
{
};

struct for_range
    : seq<expr, t_rangedot, expr>
{
};

struct for_loop
    : seq<t_for, t_paropen, s_var, t_arrow, for_range, t_parclose, block>
{
};

struct loops
    : sor<while_loop, for_loop>
{
};

// Statements

struct assignment
    : seq<var, sor<t_assign, t_multassign, t_divassign, t_plusassign, t_minusassign, t_modassign>, expr, t_semicolon>
{
};

struct break_stmt
    : seq<t_break, t_semicolon>
{
};

struct continue_stmt
    : seq<t_continue, t_semicolon>
{
};

struct return_stmt
    : seq<t_return, opt<expr>, t_semicolon>
{
};

struct statement
    : sor<var_def_stmt, expr, assignment, return_stmt, break_stmt, continue_stmt, funcs, conditional, loops, block>
{
};

// Expressions

struct expr_L1
    : sor<seq<t_paropen, expr, t_parclose>, var, literal, func_call, read_func_call>
{
};


struct expr_L2
    : seq<opt<sor<t_minus, t_not>>, expr_L1>
{
};


struct expr_L3
    : seq<expr_L2, star<sor<t_mult, t_div, t_mod>, expr_L2>>
{
};


struct expr_L4
    : seq<expr_L3, star<sor<t_plus, t_minus>, expr_L3>>
{
};

struct expr_L5
    : seq<expr_L4, opt<sor<t_greater, t_lesser, t_geq, t_leq>, expr_L4>>
{
};

struct expr_L6
    : seq<expr_L5, star<sor<t_neq, t_eq>, expr_L5>>
{
};

struct expr_L7
    : seq<expr_L6, star<t_and, expr_L6>>
{
};

struct expr
    : seq<expr_L7, star<t_or, expr_L7>>
{
};

// Block

struct block
    : seq<t_braceopen, star<statement>, t_braceclose>
{
}; 

// Entry Point

struct grammar_main
    : must<sor<imports, seq<opt<imports>, plus<sor<funcdef, classdef, var_def, main_func>>>>>
{
};
