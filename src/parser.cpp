/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

namespace boost::parser
{
struct regex_parser;
namespace detail
{
void print_parser(auto const& context, regex_parser const&, auto& ostream,
                  int unused = 0);
} // namespace detail
} // namespace boost::parser

#include <boost/parser/parser.hpp>
#include <cctype>
#include <climits>
#include <debug.hpp>
#include <fstream>
#include <iostream>
#include <parser.hpp>
#include <parser_parts.hpp>
#include <program.hpp>
#include <regex>
#include <vector>

namespace bp = ::boost::parser;
using namespace bp::literals;

size_t smrty::symbolic_parts::counter = 0;

namespace boost::parser
{

struct regex_parser
{
    constexpr regex_parser() : regstrs(), regexs()
    {
    }

    constexpr regex_parser(
        const std::vector<std::tuple<size_t, std::string_view>>& regulars) :
        regstrs(), regexs()
    {
        regstrs.reserve(regulars.size());
        regexs.reserve(regulars.size());
        for (const auto& [i, re] : regulars)
        {
            regstrs.push_back(re);
            regexs.emplace_back(std::make_tuple(i, std::string{re}));
        }
    }

    constexpr regex_parser(
        std::vector<std::tuple<size_t, std::string_view>>&& regulars) :
        regstrs(), regexs()
    {
        regstrs.reserve(regulars.size());
        regexs.reserve(regulars.size());
        for (const auto& [i, re] : regulars)
        {
            regstrs.push_back(re);
            regexs.emplace_back(std::make_tuple(i, std::string{re}));
        }
    }

    void set_regulars(
        const std::vector<std::tuple<size_t, std::string_view>>& regulars)
    {
        regexs.clear();
        regstrs.clear();
        regstrs.reserve(regulars.size());
        regexs.reserve(regulars.size());
        for (const auto& [i, re] : regulars)
        {
            regstrs.push_back(re);
            regexs.emplace_back(std::make_tuple(i, std::string{re}));
        }
        lg::debug("set_regulars({})\n", regstrs);
    }

    template <typename Iter, typename Sentinel, typename Context,
              typename SkipParser>
    auto call(Iter& first, Sentinel last, Context const& context,
              SkipParser const& skip, detail::flags flags, bool& success) const
    {
        /*
        if constexpr (std::is_same_v<detail::remove_cv_ref_t<decltype(*first)>,
                                     char32_t>)
        {
        */
        std::tuple<size_t, std::cmatch> retval;
        call(first, last, context, skip, flags, success, retval);
        return retval;
        /*
        }
        else
        {
            std::tuple<size_t, std::wcmatch> retval;
            call(first, last, context, skip, flags, success, retval);
            return retval;
        }
        */
    }

    template <typename Iter, typename Sentinel, typename Context,
              typename SkipParser, typename Attribute>
    void call(Iter& first, Sentinel last, Context const& context,
              SkipParser const& skip, detail::flags flags, bool& success,
              Attribute& retval) const
    {
        [[maybe_unused]] auto _ =
            detail::scoped_trace(*this, first, last, context, flags, retval);

        if (first == last)
        {
            success = false;
            return;
        }
        lg::debug("regex_parser::call({})\n", std::string_view{first, last});

        for (size_t i = 0; i < regexs.size(); i++)
        {
            const auto& [indx, re] = regexs[i];

            lg::debug("    test re {} /{}/\n", indx, regstrs[i]);
            typename std::tuple_element<1, Attribute>::type tmp_match{};
            if (bool found = std::regex_search(first, last, tmp_match, re);
                found)
            {
                lg::debug("    regex_search has a match: {}", tmp_match);
                if (tmp_match.ready() && tmp_match[0].first == first)
                {
                    success = true;
                    first = tmp_match[0].second;
                    retval = std::make_tuple(indx, tmp_match);
                    return;
                }
            }
        }
        success = false;
    }

    std::vector<std::string_view> regstrs;
    std::vector<std::tuple<size_t, std::regex>> regexs;
};

constexpr auto regex() noexcept
{
    return parser_interface{regex_parser()};
}

template <typename R>
constexpr auto regex(R&& regulars) noexcept
{
    return parser_interface{regex_parser(regulars)};
}

namespace detail
{
void print_parser(auto const& context, regex_parser const& regex, auto& oss,
                  int)
{
    oss << "regex( ";
    for (const auto& re : regex.regstrs)
    {
        oss << "/" << re << "/ ";
    }
    oss << ")";
}
} // namespace detail

} // namespace boost::parser

namespace smrty
{

namespace parser
{

// anonymous namespace to hide all this shit that can't be in a class
namespace
{

// this struct will be passed into the parsers
struct global_state
{
    int base = 10;
    bool allow_commas = true;
};

global_state globals{};

// rule declarations
// bp::rule<class string, std::string> const string_r = "string";
bp::symbols<int> op_functions{};
bp::symbols<int> functions{};
bp::symbols<int> paren_op{};
bp::symbols<int> if_sub_words{{"elif", 1}, {"else", 2}, {"endif", 3}};
//]
//[ self_filling_symbol_table_parser
bp::rule<class uinteger, single_number_parts> const uinteger =
    "unsigned integer";
bp::rule<class integer, single_number_parts> const integer = "integer";
bp::rule<class ufloating, single_number_parts> const ufloating =
    "unsigned floating point";
bp::rule<class floating, single_number_parts> const floating = "floating point";
bp::rule<class rati0nal, two_number_parts> const rati0nal = "rational";
bp::rule<class c0mplex, two_number_parts> const c0mplex = "complex";
bp::rule<class hex_int, single_number_parts> const hex_int =
    "hexadecimal integer";
bp::rule<class oct_int, single_number_parts> const oct_int = "octal integer";
bp::rule<class bin_int, single_number_parts> const bin_int = "binary integer";
bp::rule<class number_r, number_parts> const number_r = "number";
bp::rule<class matrix, compound_parts> const matrix = "matrix";
bp::rule<class list, compound_parts> const list = "list";
bp::rule<class time, time_parts> const time = "time";
bp::rule<class duration, time_parts> const duration = "duration";

bp::rule<class if_elif, if_elif_statement> const if_elif =
    "if/then[/elif/else]/endif statement";

bp::rule<class program_r, program> const program_r = "program";
bp::rule<class user_input, program> const user_input = "user input";

bp::rule<class simple_instruction_r, simple_instruction> const
    simple_instruction_r = "simple instruction";
bp::rule<class instruction_r, instruction> const instruction_r = "instruction";

bp::rule<class re_fn, function_parts> const re_fn = "regex function";
bp::rule<class function, function_parts> const function = "function";
bp::rule<class operators, function_parts> const operators = "operators";

bp::rule<class variable, symbolic_parts_ptr> const variable = "Variable";
bp::rule<class paren_expr, symbolic_parts_ptr> const paren_expr =
    "Parenthetic expression";
bp::rule<class paren_fn, function_parts> const paren_fn = "symbolic function";
bp::rule<class paren_fn_call, symbolic_parts_ptr> const paren_fn_call =
    "Function call";
bp::rule<class expr_atomic, symbolic_parts_ptr> const expr_atomic =
    "Atomic expression";
bp::rule<class expon, symbolic_parts_ptr> const expon = "Exponentiation";
bp::rule<class negation, symbolic_parts_ptr> const negation = "Negation";
bp::rule<class multdiv, symbolic_parts_ptr> const multdiv =
    "Multiplication or division";
bp::rule<class addsub, symbolic_parts_ptr> const addsub =
    "Addition or subtraction";
bp::rule<class symbolic_r, symbolic_parts_ptr> const symbolic_r =
    "Symbolic expression";
/*
auto const add_symbol = [](auto& ctx) {
    using namespace bp::literals;
    // symbols::insert() requires a string, not a single character.
    char chars[2] = {_attr(ctx)[0_c], 0};
    symbols.insert(ctx, chars, _attr(ctx)[1_c]);
};
*/
#define print_ctx_types(name)                                                  \
    do                                                                         \
    {                                                                          \
        void* attr_addr = nullptr;                                             \
        if constexpr (!same_type_v<decltype(attr), bp::none>)                  \
        {                                                                      \
            attr_addr = (void*)(&attr);                                        \
        }                                                                      \
        lg::debug("{}: attr is a {} @{:p}, val is a {} @{:p}\n", #name,        \
                  DEBUG_TYPE(attr), attr_addr, DEBUG_TYPE(val),                \
                  ((void*)(&val)));                                            \
    } while (0)

#define print_ctx_val_type(name)                                               \
    lg::debug("{}: val is a {} @{:p}\n", #name, DEBUG_TYPE(val),               \
              ((void*)(&val)))

auto capture_mantissa_sign = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(capture_mantissa_sign);
    val.mantissa_sign = (attr == '-') ? -1 : 1;
    // lg::debug("capture_mantissa_sign: val={}, @{:x}\n", val,
    //           (unsigned long)((void*)(&val)));
};

auto capture_exponent_sign = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(capture_exponent_sign);
    val.exponent_sign = (attr == '-') ? -1 : 1;
};

auto parse_int_mantissa = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_int_mantissa);
    val.mantissa = attr;
    // lg::debug("parse_int_mantissa: val={}, @{:x}\n", val,
    //           (unsigned long)((void*)(&val)));
};

auto parse_floating_mantissa = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_floating_mantissa);
    auto& [p1, p2] = attr;
    if (p1)
    {
        val.mantissa = *p1 + p2;
    }
    else
    {
        val.mantissa = p2;
    }
    val.base = 0; // set flag for floating point
};

auto parse_exponent = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_exponent);
    val.exponent = attr;
};

auto parse_ufloating = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // attr is a tuple<optional<variant<string, char>>, string,
    // optional<tuple<string, string>> >
    // print_ctx_types(parse_ufloating);
    val.full = {attr.begin(), attr.end()};
};

auto parse_floating = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // attr is a tuple<optional<variant<string, char>>, string,
    // optional<tuple<string, string>> >
    // print_ctx_types(parse_floating);
    attr.mantissa_sign = val.mantissa_sign;
    val = attr;
};

auto parse_uinteger = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_uinteger);
    val.full = {attr.begin(), attr.end()};
    // print_ctx_types(parse_uinteger);
    // lg::debug("parse_uinteger: val={}, @{:x}\n", val,
    //           (unsigned long)((void*)(&val)));
};

auto parse_integer = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_integer);
    // val.full = {attr.begin(), attr.end()};
    // lg::debug("parse_integer: val={}, attr={}\n", val, attr);
    attr.mantissa_sign = val.mantissa_sign;
    val = attr;
};

auto const parse_instruction = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_instruction);
    // parse_instruction: attr is a instruction (maybe not same order?)
    //                     val is a instruction
    std::visit([&val](auto& a) { val = a; }, attr);
};

auto const parse_simple_instruction = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_simple_instruction);
    // parse_simple_instruction: attr = simple_instruction (maybe diff order?)
    //                           val = simple_instruction
    std::visit([&val](auto& a) { val = a; }, attr);
};

auto const parse_program = [](auto& ctx) {
    // parse_program: attr is a vector<instruction>, val is a program
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_program);
    // _val(ctx).instructions.push_back(_attr(ctx));
    // parse_program: attr is a vector<instruction>, val is a program
    val.standalone = false;
    val.body = attr;
};

auto const parse_standalone_program = [](auto& ctx) {
    // parse_standalone_program: attr is a vector<instruction>, val is a program
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_standalone_program);
    val.standalone = true;
    val.body = attr;
};

auto const parse_if_cond = [](auto& ctx) {
    // parse_if_cond: attr = vector<simple_instruction>, val = if_elif_statement
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_if_cond);
    val.branches.emplace_back(
        std::make_tuple(true, simple_program{attr}, program{}));
};

auto const parse_if_body = [](auto& ctx) {
    // parse_if_body: attr = vector<instruction>, val = if_elif_statement
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_if_body);
    std::get<program>(val.branches.back()).body = attr;
};

auto const parse_else = [](auto& ctx) {
    // parse_else: attr = vector<instruction>, val = if_elif_statement
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_else);
    // new branch with an empty condition
    val.branches.emplace_back(
        std::make_tuple(true, simple_program{}, program{}));
    std::get<program>(val.branches.back()).body = attr;
};

auto const parse_real = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_real);
    val.first = attr;
    val.polar = false;
    val.number_type = two_number_parts::type::cmplx;
};

auto const set_polar_mode = [](auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(set_polar_mode);
    val.polar = true;
    val.number_type = two_number_parts::type::cmplx;
};

auto const parse_angle_or_imag = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_angle_or_imag);
    attr.mantissa_sign = val.second.mantissa_sign;
    val.second = attr;
    val.number_type = two_number_parts::type::cmplx;
};

auto capture_imaginary_sign = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(capture_imaginary_sign);
    val.second.mantissa_sign = (attr == '-') ? -1 : 1;
};

auto const parse_complex = [](auto& ctx) {
    [[maybe_unused]] auto& attr = _attr(ctx);
    [[maybe_unused]] auto& val = _val(ctx);
    // print_ctx_types(parse_complex);
    // lg::debug("parse_complex: val={}, attr={}\n", val, attr);
};

auto const parse_numerator = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_numerator);
    val.first = attr;
    val.number_type = two_number_parts::type::rtnl;
};

auto const parse_denominator = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_denominator);
    val.second = attr;
    val.number_type = two_number_parts::type::rtnl;
};

auto const parse_rational = [](auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    [[maybe_unused]] auto& val = _val(ctx);
    // print_ctx_types(parse_rational);
};

auto const parse_function = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_function);
    val.fn_index = attr;
};

auto const parse_regulars = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_regulars);
    val.fn_index = std::get<0>(attr);
    // turn the cmatches into vector of strings
    auto& matches = std::get<1>(attr);
    std::vector<std::string> args;
    args.reserve(matches.size());
    for (size_t i = 0; i < matches.size(); i++)
    {
        args.emplace_back(matches.str(i));
    }
    val.re_args = std::move(args);
};

auto const compound_append = [](auto& ctx) {
    auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(compound_append);
    val.items.push_back(attr);
};

auto const parse_matrix = [](auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    [[maybe_unused]] auto& val = _val(ctx);
    // print_ctx_types(parse_matrix);
    // val = attr;
};

auto const count_cols = [](auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(count_cols);
    // setting cols 'makes' it a matrix
    val.cols = val.items.size();
};

auto const parse_list = [](auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    [[maybe_unused]] auto& val = _val(ctx);
    // print_ctx_types(parse_list);
    // val = attr;
};

auto const save_year = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_year);
    val.year = attr;
    val.absolute = true;
};

auto const save_month = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_month);
    val.month = attr;
    val.absolute = true;
};

auto const save_day = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_day);
    val.day = attr;
    val.absolute = true;
};

auto const save_hour = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_hour);
    val.h = attr;
};

auto const save_minute = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_minute);
    val.m = attr;
};

auto const save_second = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_second);
    val.s = attr;
};

auto const save_sub_second = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_sub_second);
    val.sub = attr;
};

auto const save_timezone = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_timezone);
    val.tz = attr;
};

auto const save_full_time = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_full_time);
    val.full = attr;
};

auto const save_duration = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_duration);
    val.duration = attr;
};

auto const save_time_suffix = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(save_time_suffix);
    val.suffix = attr;
};

auto const set_binary = [](auto& ctx) {
    auto& val = _val(ctx);
    print_ctx_val_type(set_binary);
    val.base = 2;
};

auto const set_octal = [](auto& ctx) {
    auto& val = _val(ctx);
    print_ctx_val_type(set_octal);
    val.base = 8;
};

auto const set_hexadecimal = [](auto& ctx) {
    auto& val = _val(ctx);
    print_ctx_val_type(set_hexadecimal);
    val.base = 16;
};

auto const set_no_commas = [](auto& ctx) {
    auto& g = globals;
    g.allow_commas = false;
};

auto const set_commas_ok = [](auto& ctx) {
    auto& g = globals;
    g.allow_commas = true;
};

auto const allow_commas = [](auto& ctx) {
    auto& g = globals;
    return g.allow_commas;
};

auto const ufloating_def = bp::raw
    [bp::lexeme[bp::skip(bp::if_(allow_commas)[","_l])[(
                    -(bp::char_('1', '9') >> *bp::digit | "0"_l) >>
                    (bp::char_('.') >> +bp::digit))[parse_floating_mantissa]] >>
                -(bp::omit[bp::char_("eE")] >>
                  -bp::char_("+-")[capture_exponent_sign] >>
                  +bp::digit)[parse_exponent]]][parse_ufloating];
auto const floating_def = bp::lexeme[-bp::char_('-')[capture_mantissa_sign] >>
                                     ufloating][parse_floating];

auto const uinteger_def =
    bp::raw[bp::lexeme[bp::skip(bp::if_(allow_commas)[","_l])[(
                           (bp::char_('1', '9') >> *bp::digit) |
                           bp::string("0"))[parse_int_mantissa]] >>
                       -(bp::omit[bp::char_("eE")] >>
                         -bp::char_("+-")[capture_exponent_sign] >>
                         +bp::digit)[parse_exponent]]][parse_uinteger];
auto const integer_def = bp::lexeme[-bp::char_('-')[capture_mantissa_sign] >>
                                    uinteger][parse_integer];

auto base_2 = [](auto& ctx) { return globals.base == 2; };
auto base_8 = [](auto& ctx) { return globals.base == 8; };
auto base_16 = [](auto& ctx) { return globals.base == 16; };

auto const bin_int_def = bp::lexeme[("0b"_l | bp::eps(base_2))[set_binary] >>
                                    (+bp::char_("01"))[parse_int_mantissa]];
auto const oct_int_def =
    bp::lexeme[("0"_l | bp::eps(base_8))[set_octal] >>
               (+bp::char_("01234567"))[parse_int_mantissa]];
auto const hex_int_def =
    bp::lexeme[("0x"_l | bp::eps(base_16))[set_hexadecimal] >>
               (+bp::hex_digit)[parse_int_mantissa]];

auto const rati0nal_def =
    bp::lexeme[(floating | integer)[parse_numerator] >> '/'_l >>
               (ufloating | uinteger)[parse_denominator]][parse_rational];
auto const c0mplex_def =
    bp::lexeme[(-bp::char_("-")[capture_imaginary_sign] >>
                (ufloating | uinteger)[parse_angle_or_imag] >> bp::char_("ij"))]
              [parse_complex] |
    ('('_l[set_no_commas] >> (floating | integer)[parse_real] >> ','_l >>
     -'<'_l[set_polar_mode] >> (floating | integer)[parse_angle_or_imag] >>
     ')'_l[set_commas_ok]) |
    bp::lexeme[(floating | integer)[parse_real] >>
               bp::char_("+-")[capture_imaginary_sign] >>
               (ufloating | uinteger)[parse_angle_or_imag] >> bp::char_("ij")]
              [parse_complex];

auto const number_r_def =
    bin_int | hex_int | oct_int | c0mplex | rati0nal | floating | integer;

auto const matrix_def =
    ("["_l > "["_l > +number_r[compound_append] > "]"_l[count_cols] >
     (*("["_l > +number_r[compound_append] > "]"_l) |
      (+number_r[compound_append])) > "]"_l)[parse_matrix];

auto const list_def = ("{"_l > +number_r[compound_append] > "}"_l)[parse_list];

// iso 8601 date format
auto const time_def = bp::lexeme
    [bp::merge[(bp::digit >> bp::digit >> bp::digit >> bp::digit)[save_year] >>
               bp::string("-") >> (bp::digit >> bp::digit)[save_month] >>
               bp::string("-") >> (bp::digit >> bp::digit)[save_day] >>
               bp::omit[-(
                   bp::string("T") >> (bp::digit >> bp::digit)[save_hour] >>
                   bp::string(":") >> (bp::digit >> bp::digit)[save_minute] >>
                   bp::string(":") >> (bp::digit >> bp::digit)[save_second] >>
                   -(bp::char_(".") >> +bp::digit)[save_sub_second] >>
                   -(bp::char_("+-") >>
                     (bp::digit >> bp::digit >> bp::char_(":") >> bp::digit >>
                      bp::digit))[save_timezone])]][save_full_time]];

auto const duration_def =
    bp::lexeme[(floating | integer)[save_duration] >>
               (bp::string("d") | bp::string("h") | bp::string("m") |
                bp::string("s") | bp::string("ns") | bp::string("us") |
                bp::string("ms"))[save_time_suffix]];

auto function_def = functions[parse_function];
auto operators_def = op_functions[parse_function];

auto re_fn_def = bp::regex()[parse_regulars];

auto const simple_instruction_r_def =
    (re_fn | function | time | duration | number_r | matrix | list | program_r |
     operators)[parse_simple_instruction];

auto const instruction_r_def =
    (if_elif | simple_instruction_r | symbolic_r)[parse_instruction];

auto const if_elif_def =
    "if"_l > (+simple_instruction_r)[parse_if_cond] > "then"_l >
    (+(instruction_r - if_sub_words))[parse_if_body] >
    *("elif"_l > (+(simple_instruction_r - if_sub_words))[parse_if_cond] >
      "then"_l > (+(instruction_r - if_sub_words))[parse_if_body]) >
    -("else"_l > (+(instruction_r - if_sub_words))[parse_else]) > "endif"_l;

auto const program_r_def = "$("_l > (*instruction_r)[parse_standalone_program] >
                           ")"_l;

auto const user_input_def = (*instruction_r)[parse_program];

auto const parse_variable = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_variable);
    lg::debug("parse_variable: val={}, attr={}\n", val, attr);
    val().left = attr;
    lg::debug("                val={}, attr={}\n", val, attr);
};

auto const parse_expr_passthru_n = [](auto& ctx, int n) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_expr_passthru);
    lg::debug("parse_expr_passthru({}): val={} <<- attr={}\n", n, val, attr);
    if (val().fn_index == smrty::invalid_function)
    {
        val = attr;
    }
    else
    {
        val().left = attr;
    }
    lg::debug("                        val={} <<- attr={}\n", val, attr);
};

auto const parse_expr_left_n = [](auto& ctx, int n) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_expr_left);
    lg::debug("parse_expr_left({}): val={} left <<- attr={}\n", n, val, attr);
    // if left is an atomic (number or variable) we need to copy it in
    // from the .left or .right
    // otherwise, we pull in the whole symbolic_parts
    if (attr().fn_index == smrty::invalid_function)
    {
        if (std::get_if<std::monostate>(&(attr().left)))
        {
            val().left = attr().right;
        }
        else
        {
            val().left = attr().left;
        }
    }
    else
    {
        val().left = attr;
    }
    lg::debug("                    val={} left <<- attr={}\n", val, attr);
};

auto const parse_expr_right_n = [](auto& ctx, int n) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_expr_right);
    lg::debug("parse_expr_right({}): val={} right <<- attr={}\n", n, val, attr);
    if (attr().fn_index == smrty::invalid_function)
    {
        if (std::get_if<std::monostate>(&(attr().right)))
        {
            val().right = attr().left;
        }
        else
        {
            val().right = attr().right;
        }
    }
    else
    {
        val().right = attr;
    }
    lg::debug("                  val={} right <<- attr={}\n", val, attr);
};

auto const parse_expr_op = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_expr_op);
    lg::debug("parse_expr_op: val={}, attr={}\n", val, attr);
    std::string_view v{&attr, 1};
    if (val().fn_index == smrty::invalid_function)
    {
        val().fn_style = symbolic_op::infix;
        val().fn_index = smrty::fn_id_by_name(v);
    }
    else
    {
        auto left = val;
        val = symbolic_parts_ptr();
        val().left = left;
        val().fn_style = symbolic_op::infix;
        val().fn_index = smrty::fn_id_by_name(v);
    }
    lg::debug("               val={}, attr={}\n", val, attr);
};

auto const parse_negation = [](auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_negation);
    lg::debug("parse_negation: val={}, attr={}\n", val, attr);
    val().fn_index = smrty::fn_id_by_name("*");
    val().fn_style = symbolic_op::prefix;
    val().left = number_parts{single_number_parts(-1, "1")};
    lg::debug("                val={}, attr={}\n", val, attr);
};

auto const parse_number_expr = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(parse_number_expr);
    lg::debug("parse_number_expr: val={}, attr={}\n", val, attr);
    val().fn_index = smrty::invalid_function;
    val().left = attr;
    lg::debug("                   val={}, attr={}\n", val, attr);
};

auto const store_expr_fn = [](auto& ctx) {
    const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    // print_ctx_types(store_expr_fn);
    lg::debug("store_expr_fn: val={}, attr={}\n", val, attr);
    val().fn_style = symbolic_op::paren;
    val().fn_index = attr.fn_index;
    lg::debug("               val={}, attr={}\n", val, attr);
};

[[maybe_unused]] auto const parse_expr_passthru_1 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 1);
};
[[maybe_unused]] auto const parse_expr_passthru_2 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 2);
};
[[maybe_unused]] auto const parse_expr_passthru_3 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 3);
};
[[maybe_unused]] auto const parse_expr_passthru_4 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 4);
};
[[maybe_unused]] auto const parse_expr_passthru_6 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 6);
};
[[maybe_unused]] auto const parse_expr_passthru_5 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 5);
};
[[maybe_unused]] auto const parse_expr_passthru_7 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 7);
};
[[maybe_unused]] auto const parse_expr_passthru_8 = [](const auto& ctx) {
    return parse_expr_passthru_n(ctx, 8);
};

[[maybe_unused]] auto const parse_expr_left_1 = [](const auto& ctx) {
    return parse_expr_left_n(ctx, 1);
};
[[maybe_unused]] auto const parse_expr_left_2 = [](const auto& ctx) {
    return parse_expr_left_n(ctx, 2);
};
[[maybe_unused]] auto const parse_expr_left_3 = [](const auto& ctx) {
    return parse_expr_left_n(ctx, 3);
};
[[maybe_unused]] auto const parse_expr_right_1 = [](const auto& ctx) {
    return parse_expr_right_n(ctx, 1);
};
[[maybe_unused]] auto const parse_expr_right_2 = [](const auto& ctx) {
    return parse_expr_right_n(ctx, 2);
};
[[maybe_unused]] auto const parse_expr_right_3 = [](const auto& ctx) {
    return parse_expr_right_n(ctx, 3);
};
[[maybe_unused]] auto const parse_expr_right_4 = [](const auto& ctx) {
    return parse_expr_right_n(ctx, 4);
};
[[maybe_unused]] auto const parse_passthru_bump = [](const auto& ctx) {
    [[maybe_unused]] const auto& attr = _attr(ctx);
    auto& val = _val(ctx);
    lg::debug("parse_passthru_bump: val={}\n", val);
};
// 2+3+4
//     +
//   +   4
// 2   3

// all symbolic instruction types make symbolic_parts
auto const variable_def = (+bp::char_('a', 'z'))[parse_variable];
auto const paren_expr_def = "("_l > addsub[parse_expr_passthru_1] > ")"_l;
auto const paren_fn_def = paren_op[parse_function];
auto const paren_fn_call_def =
    paren_fn[store_expr_fn] > paren_expr[parse_expr_passthru_2];
auto const expr_atomic_def =
    number_r[parse_number_expr] |
    (paren_expr | paren_fn_call | variable)[parse_expr_passthru_3];
auto const expon_def =
    (expr_atomic[parse_expr_left_1] >> bp::char_("^")[parse_expr_op] >>
     expr_atomic[parse_expr_right_1]) |
    expr_atomic[parse_expr_passthru_4];
auto const negation_def =
    (bp::string("-")[parse_negation] >> expon[parse_expr_right_2]) |
    expon[parse_expr_passthru_8];
auto const multdiv_def = (negation[parse_expr_left_3] >>
                          bp::char_("*/%")[parse_expr_op] >>
                          negation[parse_expr_right_3]) >>
                             *(bp::char_("*/%")[parse_expr_op] >>
                               negation[parse_expr_right_4]) |
                         negation[parse_expr_passthru_5];
auto const addsub_def = (multdiv[parse_expr_left_3] >>
                         bp::char_("+-")[parse_expr_op] >>
                         multdiv[parse_expr_right_3]) >>
                            *(bp::char_("+-")[parse_expr_op] >>
                              multdiv[parse_expr_right_4]) |
                        multdiv[parse_expr_passthru_5];
auto const symbolic_r_def = "'"_l > addsub[parse_expr_passthru_6] > "'"_l;

BOOST_PARSER_DEFINE_RULES(uinteger, integer, ufloating, floating, rati0nal,
                          c0mplex, number_r, hex_int, oct_int, bin_int, matrix,
                          list, time, duration, if_elif, simple_instruction_r,
                          instruction_r, program_r, re_fn, function, operators,
                          user_input);
BOOST_PARSER_DEFINE_RULES(variable, paren_expr, paren_fn, paren_fn_call,
                          expr_atomic, expon, negation, multdiv, addsub,
                          symbolic_r);

std::span<std::string_view> function_names;
int current_base_actual = 10;

} // namespace

void set_current_base(int b)
{
    current_base_actual = b;
}

void set_function_lists(
    std::vector<std::string_view>& fn_names,
    const std::vector<std::tuple<size_t, std::string_view>>& regex_functions)
{
    auto all_nonalnum = [](std::string_view s) {
        auto iter = s.begin();
        while (iter != s.end())
        {
            if (std::isalnum(*iter))
            {
                return false;
            }
            iter++;
        }
        return true;
    };
    size_t i = 0;
    for (const auto& f : fn_names)
    {
        // operators might interfere with other stuff, so separate
        // them to keep them at lower priority in the parse stack
        if (all_nonalnum(f))
        {
            op_functions(f, i++);
        }
        else
        {
            paren_op(f, i);
            functions(f, i++);
        }
    }
    re_fn_def.parser_.parser_.set_regulars(regex_functions);
    function_names = {fn_names.begin(), fn_names.end()};
}

std::optional<program> parse_user_input(std::string_view str,
                                        diagnostic_function errors_callback)
{
    // Initialize our globals
    global_state g{current_base_actual, true};
    bp::callback_error_handler error_handler(errors_callback);
    // Make a new parser that includes the globals and error handler.
    auto const parser =
        bp::with_error_handler(bp::with_globals(user_input, g), error_handler);

    auto trace = lg::debug_level >= lg::level::trace
                     ? boost::parser::trace::on
                     : boost::parser::trace::off;
    return bp::parse(str, parser, bp::ws, trace);
}

std::optional<symbolic_parts_ptr>
    parse_symbolic(std::string_view str, diagnostic_function errors_callback)
{
    // Initialize our globals
    global_state g{current_base_actual, true};
    bp::callback_error_handler error_handler(errors_callback);
    // Make a new parser that includes the globals and error handler.
    auto const parser =
        bp::with_error_handler(bp::with_globals(symbolic_r, g), error_handler);

    auto trace = lg::debug_level >= lg::level::trace
                     ? boost::parser::trace::on
                     : boost::parser::trace::off;
    return bp::parse(str, parser, bp::ws, trace);
}

} // namespace parser

} // namespace smrty
