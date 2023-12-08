#pragma once
#include <variant>

template <typename... T>
static inline auto operator<=>(const std::variant<T...>& a,
                               const std::variant<T...>& b)
{
    return std::visit(
        [](const auto& a, const auto& b) {
            if constexpr (same_type_v<decltype(a), decltype(b)>)
            {
                if (a == b)
                {
                    return std::strong_ordering::equal;
                }
                if (a < b)
                {
                    return std::strong_ordering::less;
                }
                return std::strong_ordering::greater;
            }
            else if constexpr (same_type_v<decltype(a), mpc> ||
                               same_type_v<decltype(b), mpc>)
            {
                auto aa = static_cast<mpc>(a);
                auto bb = static_cast<mpc>(b);
                auto c = aa.real() * aa.imag();
                auto d = bb.real() * bb.imag();
                if (c == d)
                {
                    return std::strong_ordering::equal;
                }
                if (c < d)
                {
                    return std::strong_ordering::less;
                }
                return std::strong_ordering::greater;
            }
            else if constexpr (same_type_v<decltype(a), mpf> ||
                               same_type_v<decltype(b), mpf>)
            {
                auto c = static_cast<mpf>(a);
                auto d = static_cast<mpf>(b);
                if (c == d)
                {
                    return std::strong_ordering::equal;
                }
                if (c < d)
                {
                    return std::strong_ordering::less;
                }
                return std::strong_ordering::greater;
            }
            else if constexpr (same_type_v<decltype(a), mpq> ||
                               same_type_v<decltype(b), mpq>)
            {
                auto c = static_cast<mpq>(a);
                auto d = static_cast<mpq>(b);
                if (c == d)
                {
                    return std::strong_ordering::equal;
                }
                if (c < d)
                {
                    return std::strong_ordering::less;
                }
                return std::strong_ordering::greater;
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
        },
        a, b);
}

// cannot use operator== because it does not do casting
template <typename... T>
static inline bool equals(const std::variant<T...>& a,
                          const std::variant<T...>& b)
{
    return std::visit(
        [](const auto& a, const auto& b) {
            if constexpr (same_type_v<decltype(a), decltype(b)>)
            {
                return a == b;
            }
            else if constexpr (same_type_v<decltype(a), mpc> ||
                               same_type_v<decltype(b), mpc>)
            {
                return static_cast<mpc>(a) == static_cast<mpc>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpf> ||
                               same_type_v<decltype(b), mpf>)
            {
                return static_cast<mpf>(a) == static_cast<mpf>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpq> ||
                               same_type_v<decltype(b), mpq>)
            {
                return static_cast<mpq>(a) == static_cast<mpq>(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
            return false;
        },
        a, b);
}

template <typename... T>
static inline std::variant<T...> operator+(const std::variant<T...>& a,
                                           const std::variant<T...>& b)
{
    return std::visit(
        [](const auto& a, const auto& b) -> std::variant<T...> {
            if constexpr (same_type_v<decltype(a), decltype(b)>)
            {
                return a + b;
            }
            else if constexpr (same_type_v<decltype(a), mpc> ||
                               same_type_v<decltype(b), mpc>)
            {
                return static_cast<mpc>(a) + static_cast<mpc>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpf> ||
                               same_type_v<decltype(b), mpf>)
            {
                return static_cast<mpf>(a) + static_cast<mpf>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpq> ||
                               same_type_v<decltype(b), mpq>)
            {
                return static_cast<mpq>(a) + static_cast<mpq>(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
        },
        a, b);
}

template <typename... T>
static inline std::variant<T...> operator-(const std::variant<T...>& a,
                                           const std::variant<T...>& b)
{
    return std::visit(
        [](const auto& a, const auto& b) -> std::variant<T...> {
            if constexpr (same_type_v<decltype(a), decltype(b)>)
            {
                return a - b;
            }
            else if constexpr (same_type_v<decltype(a), mpc> ||
                               same_type_v<decltype(b), mpc>)
            {
                return static_cast<mpc>(a) - static_cast<mpc>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpf> ||
                               same_type_v<decltype(b), mpf>)
            {
                return static_cast<mpf>(a) - static_cast<mpf>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpq> ||
                               same_type_v<decltype(b), mpq>)
            {
                return static_cast<mpq>(a) - static_cast<mpq>(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
        },
        a, b);
}

template <typename... T>
static inline std::variant<T...> operator*(const std::variant<T...>& a,
                                           const std::variant<T...>& b)
{
    return std::visit(
        [](const auto& a, const auto& b) -> std::variant<T...> {
            if constexpr (same_type_v<decltype(a), decltype(b)>)
            {
                return a * b;
            }
            else if constexpr (same_type_v<decltype(a), mpc> ||
                               same_type_v<decltype(b), mpc>)
            {
                return static_cast<mpc>(a) * static_cast<mpc>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpf> ||
                               same_type_v<decltype(b), mpf>)
            {
                return static_cast<mpf>(a) * static_cast<mpf>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpq> ||
                               same_type_v<decltype(b), mpq>)
            {
                return static_cast<mpq>(a) * static_cast<mpq>(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
        },
        a, b);
}

template <typename... T>
static inline std::variant<T...> operator/(const std::variant<T...>& a,
                                           const std::variant<T...>& b)
{
    return std::visit(
        [](const auto& a, const auto& b) -> std::variant<T...> {
            if constexpr (same_type_v<decltype(a), mpz> &&
                          same_type_v<decltype(b), mpz>)
            {
                return mpq{a, b};
            }
            else if constexpr (same_type_v<decltype(a), decltype(b)>)
            {
                return a / b;
            }
            else if constexpr (same_type_v<decltype(a), mpc> ||
                               same_type_v<decltype(b), mpc>)
            {
                return static_cast<mpc>(a) / static_cast<mpc>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpf> ||
                               same_type_v<decltype(b), mpf>)
            {
                return static_cast<mpf>(a) / static_cast<mpf>(b);
            }
            else if constexpr (same_type_v<decltype(a), mpq> ||
                               same_type_v<decltype(b), mpq>)
            {
                return static_cast<mpq>(a) / static_cast<mpq>(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
        },
        a, b);
}
