/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <calculator.hpp>
#include <numeric.hpp>
#include <symbolic.hpp>

namespace smrty
{

/*
 6/7+3*8*-x^(y+2)*sin(5*z)
           sym('+')
          /        \
   sym('/')      sym('*')
     /   \        /     \
  num(6) num(7) num(3) sym('*')
                       /    \
                    num(8)  sym('*')
                             /    \
                        num(-1)  sym('*')
                                 /     \
                           sym(exp)   sym(sin)(style=paren)
                            /     \         \
                       var(x)  sym()(paren) sym('*')
                                /           /     \
                           sym('+')      num(5)  var(z)
                            /     \
                         var(y)  num(2)
*/

symbolic::symbolic() : ptr(std::make_shared<symbolic_actual>(*this))
{
}
symbolic::symbolic(const symbolic& o) :
    ptr(std::make_shared<symbolic_actual>(*this, *o))
{
}
// moves to improve performance
symbolic::symbolic(symbolic&& o) : ptr(o.ptr)
{
    // eliminate the symbolic_actual from o
    o.ptr.reset();
    // set new symbolic to be creator of moved symbolic_actual
    ptr->box = std::ref(*this);
}
symbolic::~symbolic()
{
}
symbolic::symbolic(const mpx& o) :
    ptr(std::make_shared<symbolic_actual>(*this, o))
{
}

symbolic::symbolic(const std::string& o) :
    ptr(std::make_shared<symbolic_actual>(*this, o))
{
}

symbolic& symbolic::operator=(const symbolic& o)
{
    ptr = std::make_shared<symbolic_actual>(*this, *o);
    return *this;
}
symbolic& symbolic::operator=(symbolic&& o)
{
    ptr = o.ptr;
    // eliminate the symbolic_actual from o
    o.ptr.reset();
    // set new symbolic to be creator of moved symbolic_actual
    ptr->box = std::ref(*this);
    return *this;
}
symbolic_actual& symbolic::operator*() const
{
    return *ptr;
}

symbolic symbolic::operator+(const symbolic& o) const
{
    lg::verbose("symbolic '{}' + '{}'\n", *this, o);
    return *ptr + *o;
}
symbolic symbolic::operator-(const symbolic& o) const
{
    lg::verbose("symbolic '{}' - '{}'\n", *this, o);
    return *ptr - *o;
}
symbolic symbolic::operator*(const symbolic& o) const
{
    lg::verbose("symbolic '{}' * '{}'\n", *this, o);
    return *ptr * *o;
}
symbolic symbolic::operator/(const symbolic& o) const
{
    lg::verbose("symbolic '{}' / '{}'\n", *this, o);
    return *ptr / *o;
}
symbolic symbolic::operator%(const symbolic& o) const
{
    lg::verbose("symbolic '{}' % '{}'\n", *this, o);
    return *ptr / *o;
}

void symbolic::eval(Calculator& calc) const
{
    ptr->eval(calc);
}

symbolic_actual::symbolic_actual(symbolic& creator) :
    box(std::ref(creator)), fn_ptr(invalid_function),
    fn_style(symbolic_op::none)
{
}

symbolic_actual::symbolic_actual(symbolic& creator, const symbolic_actual& o) :
    box(std::ref(creator)), fn_ptr(o.fn_ptr), fn_style(o.fn_style)
{
    auto get_arg = [](const auto& a) -> symbolic_operand {
        if constexpr (same_type_v<decltype(a), std::string> ||
                      same_type_v<decltype(a), mpx>)
        {
            lg::verbose("arg is variable or numeric\n");
            return a;
        }
        else if constexpr (same_type_v<decltype(a), symbolic>)
        {
            lg::verbose("arg is symbolic\n");
            return symbolic{a};
        }
        else
        {
            lg::verbose("arg is monostate\n");
            return std::monostate();
        }
    };
    left = std::visit(get_arg, o.left);
    right = std::visit(get_arg, o.right);
}

symbolic_actual::~symbolic_actual()
{
}

symbolic_actual::symbolic_actual(symbolic& creator, const mpx& o) :
    box(std::ref(creator)), fn_ptr(invalid_function),
    fn_style(symbolic_op::none), left(o)
{
}

symbolic_actual::symbolic_actual(symbolic& creator, const std::string& o) :
    box(std::ref(creator)), fn_ptr(invalid_function),
    fn_style(symbolic_op::none), left(o)
{
}

void symbolic_actual::eval(Calculator& calc) const
{
    auto push = [&calc](numeric&& v) {
        stack_entry e;
        e.base = calc.config.base;
        e.precision = calc.config.precision;
        e.fixed_bits = calc.config.fixed_bits;
        e.is_signed = calc.config.is_signed;
        e.value(std::move(v));
        calc.stack.push_front(std::move(e));
    };
    // TODO: how this gonna work, dumbass?
    //
    // 1. turn the tree into a program and execute that
    // 2. add a 'symbolic' mode to functions that can take operate on symbolics
    // 3. push stuff onto the stack, perform the op, and pop an item back off
    //
    // depth first evaluation of symbolic tree
    if (!std::get_if<std::monostate>(&left))
    {
        if (auto s = std::get_if<symbolic>(&left); s)
        {
            s->eval(calc);
        }
        else if (auto s = std::get_if<mpx>(&left); s)
        {
            std::visit(push, *s);
        }
        else if (auto s = std::get_if<std::string>(&left); s)
        {
            if (auto v = calc.get_var(*s); v)
            {
                push(std::move(*v));
            }
            else
            {
                push(symbolic{*s});
            }
        }
    }
    if (!std::get_if<std::monostate>(&right))
    {
        if (auto s = std::get_if<symbolic>(&right); s)
        {
            s->eval(calc);
        }
        else if (auto s = std::get_if<mpx>(&right); s)
        {
            std::visit(push, *s);
        }
        else if (auto s = std::get_if<std::string>(&right); s)
        {
            if (auto v = calc.get_var(*s); v)
            {
                push(std::move(*v));
            }
            else
            {
                push(symbolic{*s});
            }
        }
    }
    if (fn_ptr != invalid_function)
    {
        fn_ptr->op(calc);
        // the Eval function pops the final item back off the stack
    }
}

fn_prio symbolic_actual::prio() const
{
    static const CalcFunction::ptr add_id = fn_get_fn_ptr_by_name("+");
    static const CalcFunction::ptr sub_id = fn_get_fn_ptr_by_name("-");
    static const CalcFunction::ptr mul_id = fn_get_fn_ptr_by_name("*");
    static const CalcFunction::ptr div_id = fn_get_fn_ptr_by_name("/");
    static const CalcFunction::ptr mod_id = fn_get_fn_ptr_by_name("%");
    static const CalcFunction::ptr pow_id = fn_get_fn_ptr_by_name("^");
    static const CalcFunction::ptr fct_id = fn_get_fn_ptr_by_name("!");
    // paren functions are atomic, so high prio?
    // only need to do this for infix, prefix or postfix
    if (fn_style == symbolic_op::prefix)
    {
        if (fn_ptr == sub_id)
        {
            return fn_prio::negate;
        }
        throw std::runtime_error(std::format("unexpected prefix operator '{}'",
                                             fn_get_name(fn_ptr)));
    }
    if (fn_style == symbolic_op::infix)
    {
        if (fn_ptr == add_id || fn_ptr == sub_id)
        {
            return fn_prio::addsub;
        }
        if (fn_ptr == mul_id || fn_ptr == div_id || fn_ptr == mod_id)
        {
            return fn_prio::multdiv;
        }
        if (fn_ptr == pow_id)
        {
            return fn_prio::exponent;
        }
        throw std::runtime_error(
            std::format("unexpected infix operator '{}'", fn_get_name(fn_ptr)));
    }
    if (fn_style == symbolic_op::postfix)
    {
        if (fn_ptr == fct_id)
        {
            return fn_prio::factorial;
        }
        throw std::runtime_error(
            std::format("unexpected infix operator '{}'", fn_get_name(fn_ptr)));
    }
    if (fn_style == symbolic_op::paren)
    {
        return fn_prio::atomic;
    }
    if (fn_style == symbolic_op::none)
    {
        if (fn_ptr == invalid_function)
        {
            return fn_prio::atomic;
        }
        throw std::runtime_error(std::format(
            "unexpected symbolic_op::none function '{}'", fn_get_name(fn_ptr)));
    }
    throw std::runtime_error(
        "unable to classify function priority for symbolic");
}

symbolic symbolic_actual::operator+(const symbolic_actual& o) const
{
    symbolic s{};
    symbolic_actual& sum = *s;
    sum.fn_ptr = fn_get_fn_ptr_by_name("+");
    sum.fn_style = symbolic_op::infix;
    sum.left = box.get();
    sum.right = o.box.get();
    lg::verbose("symbolic addition: {}\n", s);
    return s;
}

symbolic symbolic_actual::operator-(const symbolic_actual& o) const
{
    symbolic d{};
    symbolic_actual& diff = *d;
    diff.fn_ptr = fn_get_fn_ptr_by_name("-");
    diff.fn_style = symbolic_op::infix;
    diff.left = box.get();
    diff.right = o.box.get();
    lg::verbose("symbolic subtraction: {}\n", d);
    return d;
}

symbolic symbolic_actual::operator*(const symbolic_actual& o) const
{
    symbolic p{};
    symbolic_actual& prod = *p;
    prod.fn_ptr = fn_get_fn_ptr_by_name("*");
    prod.fn_style = symbolic_op::infix;
    prod.left = box.get();
    prod.right = o.box.get();
    lg::verbose("symbolic multiplication: {}\n", p);
    return p;
}

symbolic symbolic_actual::operator/(const symbolic_actual& o) const
{
    symbolic d{};
    symbolic_actual& div = *d;
    div.fn_ptr = fn_get_fn_ptr_by_name("/");
    div.fn_style = symbolic_op::infix;
    div.left = box.get();
    div.right = o.box.get();
    lg::verbose("symbolic division: {}\n", d);
    return d;
}

symbolic symbolic_actual::operator%(const symbolic_actual& o) const
{
    symbolic d{};
    symbolic_actual& div = *d;
    div.fn_ptr = fn_get_fn_ptr_by_name("%");
    div.fn_style = symbolic_op::infix;
    div.left = box.get();
    div.right = o.box.get();
    lg::verbose("symbolic divmod: {}\n", d);
    return d;
}

// functions on symbolics
symbolic floor(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("floor");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic floor: {}\n", f);
    return f;
}

symbolic ceil(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("ceil");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic ceil: {}\n", f);
    return f;
}

symbolic round(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("round");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic round: {}\n", f);
    return f;
}

symbolic lcm(const symbolic& a, const symbolic& b)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("lcm");
    fn.fn_style = symbolic_op::paren;
    fn.left = a;
    fn.right = b;
    lg::verbose("symbolic lcm: {}\n", f);
    return f;
}

symbolic lcm(const mpz& a, const symbolic& b)
{
    return lcm(symbolic{a}, b);
}

symbolic lcm(const symbolic& a, const mpz& b)
{
    return lcm(a, symbolic{b});
}

symbolic gcd(const symbolic& a, const symbolic& b)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("gcd");
    fn.fn_style = symbolic_op::paren;
    fn.left = a;
    fn.right = b;
    lg::verbose("symbolic gcd: {}\n", f);
    return f;
}

symbolic gcd(const mpz& a, const symbolic& b)
{
    return gcd(symbolic{a}, b);
}

symbolic gcd(const symbolic& a, const mpz& b)
{
    return gcd(a, symbolic{b});
}

symbolic pow(const symbolic& a, const symbolic& b)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("^");
    fn.fn_style = symbolic_op::infix;
    fn.left = a;
    fn.right = b;
    lg::verbose("symbolic pow: {}\n", f);
    return f;
}

symbolic pow(const mpx& a, const symbolic& b)
{
    return pow(symbolic{a}, b);
}

symbolic pow(const symbolic& a, const mpx& b)
{
    return pow(a, symbolic{b});
}

symbolic exp(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("exp");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic exp: {}\n", f);
    return f;
}

symbolic powul(const symbolic& a, const symbolic& b)
{
    return pow(a, b);
}

symbolic factorial(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("!");
    fn.fn_style = symbolic_op::postfix;
    fn.left = v;
    lg::verbose("symbolic factorial: {}\n", f);
    return f;
}

symbolic tgamma(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("gamma");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic gamma: {}\n", f);
    return f;
}

symbolic zeta(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("zeta");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic zeta: {}\n", f);
    return f;
}

symbolic abs(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("abs");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic abs: {}\n", f);
    return f;
}

symbolic log(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("log");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic log: {}\n", f);
    return f;
}

symbolic ln(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("ln");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic ln: {}\n", f);
    return f;
}

symbolic sqrt(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("sqrt");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic sqrt: {}\n", f);
    return f;
}

symbolic sin(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("sin");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic sin: {}\n", f);
    return f;
}

symbolic cos(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("cos");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic cos: {}\n", f);
    return f;
}

symbolic tan(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("tan");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic tan: {}\n", f);
    return f;
}

symbolic asin(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("asin");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic asin: {}\n", f);
    return f;
}

symbolic acos(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("acos");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic acos: {}\n", f);
    return f;
}

symbolic atan(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("atan");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic atan: {}\n", f);
    return f;
}

symbolic sinh(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("sinh");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic sinh: {}\n", f);
    return f;
}

symbolic cosh(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("cosh");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic cosh: {}\n", f);
    return f;
}

symbolic tanh(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("tanh");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic tanh: {}\n", f);
    return f;
}

symbolic asinh(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("asinh");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic asinh: {}\n", f);
    return f;
}

symbolic acosh(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("acosh");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic acosh: {}\n", f);
    return f;
}

symbolic atanh(const symbolic& v)
{
    symbolic f{};
    symbolic_actual& fn = *f;
    fn.fn_ptr = fn_get_fn_ptr_by_name("atanh");
    fn.fn_style = symbolic_op::paren;
    fn.left = v;
    lg::verbose("symbolic atanh: {}\n", f);
    return f;
}

} // namespace smrty
