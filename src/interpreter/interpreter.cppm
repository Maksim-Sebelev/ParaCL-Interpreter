module;

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <type_traits>
#include <ostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <stdexcept>
#include <memory>
#include <string>
#include <sstream>
#include <stack>
#include <vector>

#include "create-basic-node.hpp"

#define LOGINFO(...)
#define LOGERR(...)

export module interpreter;

import nametable;
import thelast;
// import ast_read;


//-----------------------------------------------------------------------------

namespace last::node
{

using executable_expression                     = int(interpreter::nametable::Nametable&);
using executable_statement                      = void(interpreter::nametable::Nametable&);
using printable_string                          = std::string_view();
using executable_if_with_return_codition_status = bool(interpreter::nametable::Nametable&);

static_assert(not std::is_same_v<executable_expression, executable_statement>                       , "visit specializations must be different");
static_assert(not std::is_same_v<executable_expression, printable_string>                           , "visit specializations must be different");
static_assert(not std::is_same_v<executable_statement, printable_string>                            , "visit specializations must be different");
static_assert(not std::is_same_v<executable_if_with_return_codition_status, executable_statement>   , "visit specializations must be different");
static_assert(not std::is_same_v<executable_if_with_return_codition_status, executable_expression>  , "visit specializations must be different");
static_assert(not std::is_same_v<executable_if_with_return_codition_status, printable_string>       , "visit specializations must be different");

} /* namespace last::node */

//-----------------------------------------------------------------------------

namespace last::node
{

decltype(auto) execute_expsession(BasicNode const & node, interpreter::nametable::Nametable& nametable)
{
    return visit<int, interpreter::nametable::Nametable&>(node, nametable);
}

decltype(auto) execute_statement(BasicNode const & node, interpreter::nametable::Nametable& nametable)
{
    return visit<void, interpreter::nametable::Nametable&>(node, nametable);
}

decltype(auto) print_string(BasicNode const & node)
{
    return visit<std::string_view>(node);
}

decltype(auto) execute_if_with_return_codition_status(BasicNode const & node, interpreter::nametable::Nametable& nametable)
{
    return visit<bool, interpreter::nametable::Nametable&>(node, nametable);
}
} /* namespace */

//-----------------------------------------------------------------------------

namespace last::node::visit_specializations
{

//-----------------------------------------------------------------------------
// SCAN
//-----------------------------------------------------------------------------
template <>
int visit(Scan const& scan, interpreter::nametable::Nametable& nametable)
{
    int value;
    std::cin >> value;
    LOGINFO("paracl: interpreter: scan value: {}", value);
    return value;
}

//-----------------------------------------------------------------------------

template <>
void visit(Scan const& scan, interpreter::nametable::Nametable& nametable)
{
    (void) visit<Scan, int, interpreter::nametable::Nametable&>(scan, nametable);
}

//-----------------------------------------------------------------------------
// VARIABLE
//-----------------------------------------------------------------------------
template <>
int visit(Variable const& var, interpreter::nametable::Nametable& nametable)
{
    auto&& var_name = var.name();
    auto&& value = nametable.get_variable_value(var_name);
    
    LOGINFO("paracl: interpreter: get variable '{}' = {}", var_name, value);
    return value;
}

template <>
void visit(Variable const& var, interpreter::nametable::Nametable& nametable)
{
    (void) visit<Variable, int, interpreter::nametable::Nametable&>(var, nametable);
}

//-----------------------------------------------------------------------------
// NUMBER LITERAL
//-----------------------------------------------------------------------------
template <>
int visit(NumberLiteral const& num, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: number literal: {}", num.value());
    return num.value();
}

template <>
void visit(NumberLiteral const& num, interpreter::nametable::Nametable& nametable)
{
    (void) visit<NumberLiteral, int, interpreter::nametable::Nametable&>(num, nametable);
}

//-----------------------------------------------------------------------------
// STRING LITERAL
//-----------------------------------------------------------------------------
template <>
std::string_view visit(StringLiteral const& str)
{
    LOGINFO("paracl: interpreter: string literal: \"{}\"", str.value());
    return str.value();
}

//-----------------------------------------------------------------------------
// UNARY OPERATOR
//-----------------------------------------------------------------------------
template <>
int visit(UnaryOperator const& unary, interpreter::nametable::Nametable& nametable)
{
    auto&& arg_value = execute_expsession(unary.arg(), nametable);
    
    switch (unary.type())
    {
        case UnaryOperator::MINUS: return -arg_value;
        case UnaryOperator::PLUS:  return +arg_value;
        case UnaryOperator::NOT:   return !arg_value;
        default: __builtin_unreachable();
    }
}

template <>
void visit(UnaryOperator const& unary, interpreter::nametable::Nametable& nametable)
{
    (void) visit<UnaryOperator, int, interpreter::nametable::Nametable&>(unary, nametable);
}

//-----------------------------------------------------------------------------
// BINARY OPERATOR
//-----------------------------------------------------------------------------
template <>
int visit(BinaryOperator const& bin, interpreter::nametable::Nametable& nametable)
{
    auto&& left = execute_expsession(bin.larg(), nametable);
    auto&& right = execute_expsession(bin.rarg(), nametable);

    switch (bin.type())
    {
        case BinaryOperator::AND:     return left && right; break;
        case BinaryOperator::OR:      return left || right; break;
        case BinaryOperator::ADD:     return left + right; break;
        case BinaryOperator::SUB:     return left - right; break;
        case BinaryOperator::MUL:     return left * right; break;
        case BinaryOperator::DIV:     return left / right; break;
        case BinaryOperator::REM:     return left % right; break;
        case BinaryOperator::ISAB:    return left > right; break;
        case BinaryOperator::ISABE:   return left >= right; break;
        case BinaryOperator::ISLS:    return left < right; break;
        case BinaryOperator::ISLSE:   return left <= right; break;
        case BinaryOperator::ISEQ:    return left == right; break;
        case BinaryOperator::ISNE:    return left != right; break;
        case BinaryOperator::ASGN:
        case BinaryOperator::ADDASGN:
        case BinaryOperator::SUBASGN:
        case BinaryOperator::MULASGN:
        case BinaryOperator::DIVASGN:
        case BinaryOperator::REMASGN:
            throw std::runtime_error("Assignment operators should be handled by statement nodes");
        default: __builtin_unreachable();
    }
}

template <>
void visit(BinaryOperator const& bin, interpreter::nametable::Nametable& nametable)
{
    (void) visit<BinaryOperator, int, interpreter::nametable::Nametable&>(bin, nametable);
}

//-----------------------------------------------------------------------------
// WHILE
//-----------------------------------------------------------------------------
template <>
void visit(While const& while_node, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute WHILE statement");
    
    while (execute_expsession(while_node.condition(), nametable))
        execute_statement(while_node.body(), nametable);
}

//-----------------------------------------------------------------------------
// IF
//-----------------------------------------------------------------------------
template <>
void visit(If const& if_node, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute IF statement");
    if (not execute_expsession(if_node.condition(), nametable)) return;
    execute_statement(if_node.body(), nametable);
}

//-----------------------------------------------------------------------------

template <>
bool visit(If const& if_node, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute IF statement");
    if (not execute_expsession(if_node.condition(), nametable)) return false;
    execute_statement(if_node.body(), nametable);
    return true;
}

//-----------------------------------------------------------------------------
// ELSE
//-----------------------------------------------------------------------------
template <>
void visit(Else const& else_node, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute ELSE statement");
    return execute_statement(else_node.body(), nametable);
}

//-----------------------------------------------------------------------------
// CONDITION
//-----------------------------------------------------------------------------
template <>
void visit(Condition const& condition, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute CONDITION statement");
    
    // Проверяем все if-ы по порядку
    for (auto&& if_node : condition.get_ifs())
        if (execute_if_with_return_codition_status(if_node, nametable)) return;

    return execute_statement(condition.get_else(), nametable);
}

//-----------------------------------------------------------------------------
// PRINT
//-----------------------------------------------------------------------------

template <>
void visit(Print const& print, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute print statement");

    for (auto&& arg : print)
    {
        if (support<printable_string>(arg))
            std::cout << print_string(arg);
        else
            std::cout << execute_expsession(arg, nametable);
    }
    std::cout << std::endl;
}

//-----------------------------------------------------------------------------


template <>
void visit(Scope const& node, interpreter::nametable::Nametable& nametable)
{
    LOGINFO("paracl: interpreter: execute scope statement");

    nametable.new_scope();

    for (auto&& arg : node)
        execute_statement(arg, nametable);

    nametable.leave_scope();
}

//-----------------------------------------------------------------------------
} /* namespace last::ndoe::visit_specializatoins */
//-----------------------------------------------------------------------------


SPECIALIZE_CREATE(last::node::Print          , last::node::executable_statement                        )
SPECIALIZE_CREATE(last::node::Scan           , last::node::executable_expression , last::node::executable_statement)
SPECIALIZE_CREATE(last::node::Variable       , last::node::executable_expression , last::node::executable_statement)
SPECIALIZE_CREATE(last::node::NumberLiteral  , last::node::executable_expression , last::node::executable_statement)
SPECIALIZE_CREATE(last::node::StringLiteral  , last::node::printable_string                            )
SPECIALIZE_CREATE(last::node::UnaryOperator  , last::node::executable_expression                       )
SPECIALIZE_CREATE(last::node::BinaryOperator , last::node::executable_expression                       )
SPECIALIZE_CREATE(last::node::While          , last::node::executable_statement                        )
SPECIALIZE_CREATE(last::node::If             , last::node::executable_statement  , last::node::executable_if_with_return_codition_status)
SPECIALIZE_CREATE(last::node::Else           , last::node::executable_statement                        )
SPECIALIZE_CREATE(last::node::Condition      , last::node::executable_statement                        )
SPECIALIZE_CREATE(last::node::Scope          , last::node::executable_statement                        )

//-----------------------------------------------------------------------------

#define THELAST_READ_AST_NO_INCLUDES
#include "read-ast.hpp"
#undef THELAST_READ_AST_NO_INCLUDES

//-----------------------------------------------------------------------------

namespace interpreter
{

using namespace last::node;

export
void interpret(std::filesystem::path const & ast_txt)
{
    LOGINFO("paracl: interpreter: start");

    auto&& ast = last::read(ast_txt);

    nametable::Nametable nametable;
    nametable.new_scope(); /* global scope */

    visit<void, nametable::Nametable&>(ast.root(), nametable);    

    LOGINFO("paracl: interpreter: end");
}

} /* namespace ParaCL::interpreter */

//-----------------------------------------------------------------------------
