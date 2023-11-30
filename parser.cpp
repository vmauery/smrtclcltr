#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <format>
#include <map>
#include <numeric.hpp>
#include <parser.hpp>
#include <string>
#include <string_view>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <vector>

const std::type_index Parser::mpz_type{typeid(mpz)};
const std::type_index Parser::mpf_type{typeid(mpf)};
const std::type_index Parser::mpc_type{typeid(mpc)};
const std::type_index Parser::mpq_type{typeid(mpq)};
const std::type_index Parser::time_type{typeid(time_)};
const std::type_index Parser::matrix_type{typeid(Matrix)};
const std::type_index Parser::list_type{typeid(List)};
const std::type_index Parser::cmd_type{typeid(Cmd)};
const std::type_index Parser::var_type{typeid(Var)};
const std::type_index Parser::null_type{typeid(nullptr)};
const std::type_index Parser::eol_type{typeid("\n")};
const std::type_index Parser::parse_error_type{typeid(std::runtime_error)};

template <typename T>
void handle_numeric(Token& t, const std::string& s)
{
    auto type = std::type_index(typeid(T));
    // std::print("Parsed {} as '{}'\n", expr_types[type], s);
    t = Token(type, s);
}
void handle_command(const std::vector<std::string_view>& fns, Token& t,
                    size_t cmd)
{
    // std::print("Parsed operation as '{}' -> {}\n", cmd, fns[cmd]);
    auto type = Parser::cmd_type;
    t = Token(type, fns[cmd]);
}

void handle_var(Token& t, const std::string& var)
{
    // std::print("Parsed variable as '{}'\n", var);
    auto type = Parser::var_type;
    t = Token(type, var);
}

std::map<std::type_index, std::string> Parser::expr_types{
    {Parser::mpz_type, "mpz"},   {Parser::mpf_type, "mpf"},
    {Parser::mpc_type, "mpc"},   {Parser::mpq_type, "mpq"},
    {Parser::time_type, "time"}, {Parser::matrix_type, "Matrix"},
    {Parser::list_type, "List"}, {Parser::cmd_type, "Cmd"},
    {Parser::var_type, "Var"},   {Parser::null_type, "Empty"},
    {Parser::eol_type, "eol"},   {Parser::parse_error_type, "parse_error"},
};

Parser::Parser(
    int base, const std::vector<std::string_view>& function_names,
    const std::map<size_t, std::shared_ptr<std::regex>>& fn_regexps) :
    fns(function_names),
    regexps(fn_regexps)
{
    rebuild(base);
}

void Parser::rebuild(int base)
{
    using namespace boost::spirit;
    using ascii::char_;
    using ascii::digit;
    using ascii::space;
    using ascii::string;
    using ascii::xdigit;

    // carefully build number parsers to be aware of current base
    // numbers in other bases require a prefix
    b2digit = +char_('0', '1');
    if (base == 2)
    {
        ub2int = no_skip[qi::hold[-string("0b")] >> +b2digit];
    }
    else
    {
        ub2int = no_skip[string("0b") >> +b2digit];
    }
    b8digit = +char_('0', '7');
    if (base == 8)
    {
        ub8int = no_skip[-char_('0') >> +b8digit];
    }
    else
    {
        ub8int = no_skip[char_('0') >> +b8digit];
    }
    if (base == 10)
    {
        ub10int = no_skip[qi::hold[-string("0d")] >> +digit];
    }
    else
    {
        ub10int = no_skip[string("0d") >> +digit];
    }
    if (base == 16)
    {
        ub16int = no_skip[qi::hold[-string("0x")] >> +xdigit];
    }
    else
    {
        ub16int = no_skip[string("0x") >> +xdigit];
    }
    sign = char_('+') | char_('-');
    exp = (char_('e') | char_('E'));
    expuint = no_skip[ub10int >> exp >> ub10int];
    uinteger = qi::hold[ub16int] | qi::hold[expuint] | qi::hold[ub10int] |
               qi::hold[ub8int] | qi::hold[ub2int];
    integer = no_skip[-sign >> uinteger];
    ufloating = no_skip[qi::hold[(ub10int >> exp >> char_('-') >> ub10int)] |
                        ((qi::hold[(-ub10int >> char_('.') >> ub10int)] |
                          qi::hold[(ub10int >> char_('.'))]) >>
                         -(exp >> -sign >> ub10int))];
    floating = no_skip[-sign >> ufloating];
    ij = (char_('i') | char_('j'));
    simple_number = qi::hold[floating] | qi::hold[integer];
    simple_unumber = ufloating | uinteger;
    imag = no_skip[(-sign >> -simple_unumber >> ij)];
    complex1 = no_skip[(-simple_number >> sign >> -simple_unumber >> ij)];
    complex2 = no_skip[(char_('(') >> *space >> simple_number >> *space >>
                        char_(',') >> *space >> simple_number >> *space >>
                        char_(')'))];
    complex = qi::hold[imag] | qi::hold[complex1] | complex2;
    rational = integer >> char_('/') >> uinteger;
    any_simple_number =
        qi::hold[floating] | qi::hold[rational] | qi::hold[integer];
    number = qi::hold[complex] | qi::hold[floating] | qi::hold[rational] |
             qi::hold[integer];
    spaced_simple_numbers =
        *space >> any_simple_number >> *(+space >> any_simple_number) >> *space;
    matrix_row = char_('[') >> spaced_simple_numbers >> char_(']');
    matrix = char_('[') >> matrix_row >>
             *(matrix_row | spaced_simple_numbers) >> char_(']');
    list = char_('{') >> *(*space >> number >> *space) >> char_('}');

    // time literals ns, us, ms, s, m, h, d, or
    // absolute times of the ISO8601 format: yyyy-mm-dd[Thh:mm:ss]
    dd = digit >> digit;
    date = dd >> dd >> char_('-') >> dd >> char_('-') >> dd >>
           -(char_('T') >> dd >> char_(':') >> dd >> char_(':') >> dd);
    duration = no_skip[any_simple_number >>
                       (char_('d') | char_('h') | char_('m') | char_('s') |
                        string("ns") | string("us") | string("ms"))];
    ttime = qi::hold[duration] | qi::hold[date];

    // compound = matrix | list;
    size_t idx = 0;
    for (auto iter = fns.begin(); iter != fns.end(); iter++, idx++)
    {
        fn_syms.add(*iter, idx);
    }
    function = fn_syms;
    var = char_('\'') >> +char_('a', 'z') >> char_('\'');
}

std::tuple<bool,                   // whether or not parsing was last ok
           std::shared_ptr<Token>, // what came out of the parsing
           std::string_view        // the remaining bits to parse
           >
    Parser::parse_next(const std::string_view& input)
{
    auto token1 = std::make_shared<Token>();
    Token& tokref1 = *token1;
    auto f1(input.begin());
    auto l(input.end());

    // phrase_parse will read the next token and the handlers will
    // store the token (as a string) and type into tokref
    bool ok1 = boost::spirit::qi::phrase_parse(
        f1, l,
        (ttime[([&](const auto& v) { handle_numeric<time_>(tokref1, v); })] |
         complex[([&](const auto& v) { handle_numeric<mpc>(tokref1, v); })] |
         floating[([&](const auto& v) { handle_numeric<mpf>(tokref1, v); })] |
         rational[([&](const auto& v) { handle_numeric<mpq>(tokref1, v); })] |
         integer[([&](const auto& v) { handle_numeric<mpz>(tokref1, v); })] |
         matrix[([&](const auto& v) { handle_numeric<Matrix>(tokref1, v); })] |
         list[([&](const auto& v) { handle_numeric<List>(tokref1, v); })]),
        boost::spirit::ascii::space);

    auto token2 = std::make_shared<Token>();
    Token& tokref2 = *token2;
    auto f2(input.begin());
    bool ok2 = boost::spirit::qi::phrase_parse(
        f2, l,
        (function[([&](const auto& v) { handle_command(fns, tokref2, v); })] |
         var[([&](const auto& v) { handle_var(tokref2, v); })]),
        boost::spirit::ascii::space);

    bool ok = ok1 || ok2;
    auto f = (f1 >= f2) ? f1 : f2;
    auto token = (f1 >= f2) ? token1 : token2;
    Token& tokref = *token;

    std::string_view more{f, l};
    // if phrase_parse found a thing, return that here
    // f == l denotes eating the whole input,
    // more != input denotes eating only part, but not none
    if (ok && ((f == l) | (more != input)))
    {
        return std::make_tuple(ok, token, more);
    }
    // split off next 'word'
    auto b = more.find_first_not_of(" \t\n");
    if (b == std::string::npos)
    {
        // all whitespace.... return end-of-line
        // this probably should not ever happen with the space Skipper
        return std::make_tuple(true, token, std::string_view{});
    }
    auto w = more.find_first_of(" \t\n", b);
    auto next = more.substr(0, w);
    // try to find a matching regex
    for (const auto& [n, re] : regexps)
    {
        std::cmatch matches{};
        if (std::regex_match(next.begin(), next.end(), matches, *re))
        {
            std::string_view remaining{};
            if (w != std::string::npos)
            {
                remaining = more.substr(w + 1);
            }
            tokref.type = cmd_type;
            tokref.value = fns[n];
            for (size_t i = 0; i < matches.size(); i++)
            {
                tokref.matches.push_back(matches[i].str());
            }
            return std::make_tuple(true, token, remaining);
        }
    }
    tokref.type = parse_error_type;
    tokref.value = std::format("failed parse at: '{}'", more);
    return std::make_tuple(false, token, more);
}

#ifdef STANDALONE
int main()
{
    using namespace boost::spirit;

    std::vector<std::string_view> fns{
        "help", "mod", "inv", "<<",     ">>",     "+",     "-",     "*",
        "/",    "%",   "^",   "&",      "|",      "!",     "~",     "int_type",
        "sin",  "cos", "tan", "base10", "base16", "base8", "base2",
    };
    std::map<size_t, std::shared_ptr<std::regex>> refns{
        {15, std::make_shared<std::regex>("([us])([1-9][0-9]*)")},
    };
    std::string s;
    Parser p(10, fns, refns);
    while (!std::getline(std::cin, s).eof())
    {
        std::string_view next = s;
        do
        {
            const auto& [parse_ok, t, more] = p.parse_next(next);
            if (parse_ok)
            {
                std::print("{} ", *t);
            }
            if (t->value == "base2")
                p.rebuild(2);
            else if (t->value == "base8")
                p.rebuild(8);
            else if (t->value == "base10")
                p.rebuild(10);
            else if (t->value == "base16")
                p.rebuild(16);

            next = more;
            if (!parse_ok)
            {
                std::print("Failed to parse at '{}'\n", more);
                break;
            }
        } while (next.size());
        std::print("\n");
    }
    return 0;
}
#endif // STANDALONE
