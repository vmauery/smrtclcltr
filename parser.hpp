#pragma once

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <typeindex>
#include <vector>

struct Matrix
{
    constexpr Matrix() = default;
    explicit Matrix(const std::string&)
    {
    }
};
struct List
{
    constexpr List() = default;
    explicit List(const std::string&)
    {
    }
};
struct Cmd
{
    constexpr Cmd() = default;
    explicit Cmd(const std::string&)
    {
    }
};
struct Var
{
    constexpr Var() = default;
    explicit Var(const std::string&)
    {
    }
};

struct Token;

class Parser
{
  public:
    using Iterator = std::string_view::const_iterator;
    using Rule = boost::spirit::qi::rule<Iterator, std::string()>;
    using SymRule = boost::spirit::qi::rule<Iterator, size_t()>;
    using Symbols = boost::spirit::qi::symbols<char, size_t>;

    static const std::type_index mpz_type;
    static const std::type_index mpf_type;
    static const std::type_index mpc_type;
    static const std::type_index mpq_type;
    static const std::type_index time_type;
    static const std::type_index matrix_type;
    static const std::type_index list_type;
    static const std::type_index cmd_type;
    static const std::type_index var_type;
    static const std::type_index null_type;
    static const std::type_index eol_type;
    static const std::type_index parse_error_type;
    static std::map<std::type_index, std::string> expr_types;

  public:
    Parser(int base, const std::vector<std::string_view>& function_names,
           const std::map<size_t, std::shared_ptr<std::regex>>& fn_regexps);
    void rebuild(int base);

    std::tuple<bool,                   // whether or not parsing was last ok
               std::shared_ptr<Token>, // what came out of the parsing
               std::string_view        // the remaining bits to parse
               >
        parse_next(const std::string_view& input);

  protected:
    const std::vector<std::string_view>& fns;
    const std::map<size_t, std::shared_ptr<std::regex>>& regexps;

    Rule b2digit;
    Rule ub2int;
    Rule b8digit;
    Rule ub8int;
    Rule ub10int;
    Rule ub16int;
    Rule sign;
    Rule exp;
    Rule expuint;
    Rule uinteger;
    Rule integer;
    Rule ufloating;
    Rule floating;
    Rule simple_number;
    Rule simple_unumber;
    Rule ij;
    Rule imag;     // [+-]?number?[ij]
    Rule complex1; // [+-]?number[+-]number[ij]
    Rule complex2; // (number,number)
    Rule complex;  // any imaginary or complex number
    Rule rational;
    Rule number;
    Rule spaced_numbers;
    Rule matrix_row;
    Rule matrix;
    Rule list;
    Rule dd;
    Rule date;
    Rule any_simple_number;
    Rule duration;
    Rule ttime;
    Rule op;
    Symbols fn_syms;
    SymRule function;
    Rule var;
};

struct Token
{
    Token() : type(Parser::null_type), value(), matches{}
    {
    }
    Token(const std::type_index& type) : type(type), value(), matches{}
    {
    }
    Token(const std::type_index& type, const std::string_view& value) :
        type(type), value(value), matches{}
    {
    }
    Token(const std::string_view& value, std::vector<std::string>& match) :
        type(Parser::cmd_type), value(value), matches(match)
    {
    }
    Token(const std::type_index& type, const std::string_view& value,
          const std::vector<std::string>& matches) :
        type(type),
        value(value), matches(matches)
    {
    }

    std::type_index type;
    std::string value;
    std::vector<std::string> matches;
};

template <>
struct std::formatter<Token>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Token& t, FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        std::string v{};
        std::string m{};
        out = std::format_to(out, "{{{}", Parser::expr_types[t.type]);
        if (t.value.size())
        {
            out = std::format_to(out, ": '{}'", t.value);
        }
        if (t.matches.size())
        {
            out = std::format_to(out, " ({})", t.matches[0]);
        }
        *out++ = '}';
        return out;
    }
};
