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
    struct block_noscopeopen : seq<t_braceopen, seps, star<statement>, t_braceclose> {};

    struct identifier : t_id {};
    
    // Imports

    struct a_imports : seps {};
    struct imports : seq<t_import, seps, one<'('>, seps, t_lit_string, a_imports, star<t_comma, seps, t_lit_string, a_imports>, one<')'>, seps> {};

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

    struct params_def : if_must<t_paropen, seps, opt<s_var_basic, seps, star<t_comma, seps, s_var_basic, seps>>, t_parclose> {};
    struct params : seq<t_paropen, seps, opt<expr, star<seps, t_comma, seps, expr>, seps>, t_parclose> {};
    struct funcdef : if_must<t_fn, sepp, identifier, a_open_scope, params_def, seps, opt<t_colon, seps, type>, seps, block_noscopeopen> {};
    struct main_func : seq<t_fn, sepp, if_must<t_main, seps, t_paropen, seps, t_parclose, seps, block>> {};
    struct func_call : seq<var, params> {};
    struct read_func_call : if_must<t_read, t_paropen, t_parclose> {};
    struct write_func_call : if_must<t_write, params> {};
    struct funcs : seq<sor<read_func_call, write_func_call, func_call>, t_semicolon> {};

    // Classes

    struct memberaccess : sor<one<'+'>, one<'-'>> {};
    struct classattr : seq<memberaccess, seps, var_def_stmt> {};
    struct classmethod : seq<memberaccess, seps, funcdef> {};
    struct classmembers : seq<star<classattr, seps>, star<classmethod, seps>> {};

    struct classdef : if_must<t_class, sepp, identifier, seps, opt<t_arrow, seps, identifier, seps>, t_braceopen, a_open_scope, classmembers, t_braceclose> {};

    // Conditionals

    struct conditional;
    struct a1_conditional : seps {};
    struct a2_conditional : seps {};
    struct a3_conditional : seps {};
    struct conditional : seq<t_if, seps, t_paropen, seps, expr, seps, t_parclose, a1_conditional, block, opt<seps, t_else, a3_conditional, sor<block, conditional>>, a2_conditional> {};

    // Loops

    struct a1_while_loop : seps {};
    struct a2_while_loop : seps {};
    struct a3_while_loop : seps {};
    struct while_loop : if_must<t_while, seps, t_paropen, a1_while_loop, expr, seps, t_parclose, a2_while_loop, block, a3_while_loop> {};
    
    struct a1_for_range : seps {};
    struct for_range : seq<expr, t_rangedot, expr, a1_for_range> {};
    
    struct a1_for_loop : seps {};
    struct a2_for_loop : seps {};
    struct a3_for_loop : seps {};
    struct for_loop : if_must<t_for, seps, t_paropen, a_open_scope, seps, s_var, a1_for_loop, t_arrow, seps, for_range, a2_for_loop, t_parclose, seps, block_noscopeopen, a3_for_loop> {};
    struct loops : sor<while_loop, for_loop> {};

    // Statements

    struct assignment_operators : sor<t_assign, t_multassign, t_divassign, t_plusassign, t_minusassign, t_modassign> {};
    struct assignment : seq<var, seps, if_must<assignment_operators, seps, expr, seps, t_semicolon>> {};

    struct break_stmt : seq<t_break, seps, t_semicolon> {};
    struct continue_stmt : seq<t_continue, seps, t_semicolon> {};
    struct return_stmt : if_must<t_return, sepp, opt<expr>, seps, t_semicolon> {};

    struct statement : seq<sor<seq<at<assignment>, assignment>, var_def_stmt, return_stmt, break_stmt, continue_stmt, funcs, conditional, loops, block>, seps> {};

    // Expressions

    struct expr_paropen : t_paropen {};
    struct expr_parclose : t_parclose {};

    struct a1_expr_L1 : seps{};
    struct expr_L1 : seq<sor<seq<expr_paropen, seps, expr, seps, expr_parclose>, seq<at<func_call>, func_call>, var, literal, read_func_call>, seps> {};

    struct a1_expr_L2 : seps{};
    struct expr_L2 : seq<a1_expr_L2, opt<sor<t_minus, t_not>, seps>, expr_L1> {};

    struct a1_expr_L3 : seps{};
    struct expr_L3 : seq<expr_L2, a1_expr_L3, star<sor<t_mult, t_div, t_mod>, seps, expr_L2, a1_expr_L3>> {};

    struct a1_expr_L4 : seps{};
    struct expr_L4 : seq<expr_L3, a1_expr_L4, star<sor<t_plus, t_minus>, seps, expr_L3, a1_expr_L4>> {};

    struct a1_expr_L5 : seps{};
    struct expr_L5 : seq<expr_L4, a1_expr_L5, star<sor<t_geq, t_leq, t_greater, t_lesser>, seps, expr_L4, a1_expr_L5>> {};

    struct a1_expr_L6 : seps{};
    struct expr_L6 : seq<expr_L5, a1_expr_L6, star<sor<t_neq, t_eq>, seps, expr_L5, a1_expr_L6>> {};

    struct a1_expr_L7 : seps{};
    struct expr_L7 : seq<expr_L6, a1_expr_L7, star<t_and, sepp, expr_L6, a1_expr_L7>>{};

    struct a1_expr : seps{};
    struct expr : seq<expr_L7, a1_expr, star<t_or, sepp, expr_L7, a1_expr>> {};

    // Block

    struct block : seq<t_braceopen, a_open_scope, star<statement>, t_braceclose> {};

    // Entry Point

    struct top_levels : sor<var_def_stmt, main_func, funcdef, classdef> {};
    struct a_eof : eof {};
    struct main_grammar : must<seps, sor<seq<imports, star<top_levels, seps>>, plus<top_levels, seps>>, seps, a_eof> {};

    // clang-format on
}