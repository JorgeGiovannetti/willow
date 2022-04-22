#include <tao/pegtl.hpp>
#include "tokens.cpp"

using namespace tao::pegtl;

// Imports

struct imports
    : seq<t_import, one<'('>, list<t_lit_string, t_comma>, one<')'>>
{
};

struct block;
struct expr;

// Vars

struct var
    : seq<sor<t_id, t_this>, star<sor<seq<t_bracketopen, expr, t_bracketclose>, seq<t_dot, t_id>>>>
{
};

struct var_def


// Expressions



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
    : seq<var, sor<t_assign, t_multassign, t_divassign, t_plusassign, t_minusassign, t_modassign>, expr>
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
    : sor<var_def, expr, assignment, return_stmt, break_stmt, continue_stmt, funcs, conditional, loops, block>
{
};

// Block

struct block
    : seq<t_braceopen, star<statement>, t_braceclose>
{
}; 

// Entry Point

struct grammar_main
    : must<opt<imports>, block>
{
};
