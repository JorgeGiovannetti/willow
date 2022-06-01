#include <tao/pegtl.hpp>
#include "tokens.cpp"

using namespace tao::pegtl;

namespace willow::parser
{

    // clang-format off

    struct block;
    struct expr;
    struct statement;

    struct a_open_scope : seps {};
    struct block_noscopeopen : seq<t_braceopen, star<statement>, t_braceclose> {};

    struct identifier : t_id {};
    
    // Imports

    struct imports : seq<t_import, seps, one<'('>, seps, t_lit_string, seps, star<t_comma, seps>, one<')'>, seps> {};

    // Type

    struct basic_type : sor<t_int, t_float, t_bool, t_char, t_string> {};
    struct type : seq<sor<basic_type, t_id>, star<t_bracketopen, seps, t_lit_int, seps, t_bracketclose>> {};

    // Literals

    struct literal : sor<t_lit_int, t_lit_float, t_lit_bool, t_lit_char, t_lit_string> {};

    // Vars

    struct var : seq<sor<identifier, t_this>, star<sor<seq<t_bracketopen, expr, t_bracketclose>, seq<t_dot, identifier>>>> {};
    struct s_var_type : type {};
    struct s_var_basic_type : basic_type {};
    struct s_var : seq<identifier, seps, t_colon, seps, s_var_type> {};
    struct s_var_basic : seq<identifier, seps, t_colon, seps, s_var_basic_type> {};
    struct var_def : seq<s_var, opt<seps, t_assign, seps, expr>> {};
    struct var_def_stmt : seq<var_def, seps, t_semicolon> {};

    // Functions

    struct params_def : seq<t_paropen, seps, opt<s_var_basic, star<seps, t_comma, seps, s_var_basic>, seps>, t_parclose> {};
    struct params : seq<t_paropen, seps, opt<expr, star<seps, t_comma, seps, expr>, seps>, t_parclose> {};
    struct funcdef : seq<t_fn, seps, identifier, a_open_scope, seps, params_def, seps, opt<t_colon, seps, type>, seps, block_noscopeopen> {};
    struct main_func : seq<t_fn, sepp, t_main, seps, t_paropen, seps, t_parclose, seps, block> {};
    struct func_call : seq<var, params> {};
    struct read_func_call : seq<t_read, t_paropen, t_parclose> {};
    struct write_func_call : seq<t_write, params> {};
    struct funcs : seq<sor<read_func_call, write_func_call, func_call>, t_semicolon> {};

    // Classes

    struct memberaccess : sor<t_plus, t_minus> {};
    struct classattr : seq<memberaccess, seps, var_def_stmt> {};
    struct classmethod : seq<memberaccess, seps, funcdef> {};
    struct classmembers : seq<star<classattr, seps>, star<classmethod, seps>> {};

    struct classdef : seq<t_class, sepp, identifier, seps, opt<t_arrow, seps, identifier, seps>, t_braceopen, a_open_scope, seps, classmembers, seps, t_braceclose> {};

    // Conditionals

    struct conditional;
    struct conditional : seq<t_if, seps, t_paropen, seps, expr, seps, t_parclose, seps, block, seps, opt<t_else, seps, sor<block, conditional>, seps>> {};

    // Loops

    struct while_loop : seq<t_while, seps, t_paropen, seps, expr, seps, t_parclose, seps, block> {};
    struct for_range : seq<expr, t_rangedot, expr> {};
    struct for_loop : seq<t_for, seps, t_paropen, a_open_scope, seps, s_var, seps, t_arrow, seps, for_range, seps, t_parclose, seps, block_noscopeopen> {};
    struct loops : sor<while_loop, for_loop> {};

    // Statements

    struct assignment : seq<var, sor<t_assign, t_multassign, t_divassign, t_plusassign, t_minusassign, t_modassign>, expr, seps, t_semicolon> {};

    struct break_stmt : seq<t_break, seps, t_semicolon> {};
    struct continue_stmt : seq<t_continue, seps, t_semicolon> {};
    struct return_stmt : seq<t_return, sepp, opt<expr>, seps, t_semicolon> {};

    struct statement : seq<sor<var_def_stmt, expr, assignment, return_stmt, break_stmt, continue_stmt, funcs, conditional, loops, block>, seps> {};

    // Expressions

    struct expr_paropen : t_paropen {};
    struct expr_parclose : t_parclose {};

    struct expr_L1 : seq<sor<seq<expr_paropen, expr, expr_parclose>, var, literal, func_call, read_func_call>, seps> {};
    struct expr_L2 : seq<opt<sor<t_minus, t_not>, seps>, expr_L1> {};
    struct expr_L3 : seq<expr_L2, seps, star<sor<t_mult, t_div, t_mod>, seps, expr_L2>> {};
    struct expr_L4 : seq<expr_L3, seps, star<sor<t_plus, t_minus>, seps, expr_L3>> {};
    struct expr_L5 : seq<expr_L4, opt<sor<t_greater, t_lesser, t_geq, t_leq>, seps, expr_L4>> {};
    struct expr_L6 : seq<expr_L5, star<sor<t_neq, t_eq>, seps, expr_L5>> {};
    struct expr_L7 : seq<expr_L6, star<sep, t_and, sepp, expr_L6>>{};
    struct expr : seq<expr_L7, star<sep, t_or, sepp, expr_L7>> {};

    // Block

    struct block : seq<t_braceopen, a_open_scope, star<statement>, t_braceclose> {};

    // Entry Point

    struct top_levels : sor<var_def_stmt, funcdef, classdef, main_func> {};
    struct grammar : must<seps, if_must_else<imports, star<top_levels, seps>, plus<top_levels, seps>>, eof> {};

    // clang-format on
}