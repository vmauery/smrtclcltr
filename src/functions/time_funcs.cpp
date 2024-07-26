/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <chrono>
#include <function.hpp>

namespace smrty
{
namespace function
{

struct date_time : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"now"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: now\n"
            "\n"
            "    return a date-time with sub-second precision based\n"
            "    on the system clock"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        auto tp = std::chrono::system_clock::now();
        lg::debug("tp: {} ({})\n", tp, tp.time_since_epoch());
        time_ now(tp);
        calc.stack.emplace_front(now, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct unix_ts : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"unix"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: unix\n"
            "\n"
            "    return a unix timestamp with sub-second precision based\n"
            "    on the system clock"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        auto now = std::chrono::system_clock::now();
        auto now_ns =
            std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
        auto value = now_ns.time_since_epoch();
        mpq ts(value.count(), 1ul * 1000 * 1000 * 1000);
        calc.stack.emplace_front(ts, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct to_date_time : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"2date"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x 2date\n"
            "\n"
            "    return a date-time based on the unix timestamp x"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // required arg guaranteed by num_args
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            throw units_prohibited();
        }
        mpq ts;
        if (const mpq* v = std::get_if<mpq>(&e.value()); v)
        {
            ts = *v;
        }
        else if (const mpz* v = std::get_if<mpz>(&e.value()); v)
        {
            ts = static_cast<mpq>(*v);
        }
        else if (const mpf* v = std::get_if<mpf>(&e.value()); v)
        {
            ts = make_quotient(*v);
        }
        else
        {
            throw std::invalid_argument("Requires a real number");
        }
        calc.stack.pop_front();

        time_ t{ts, true};
        calc.stack.emplace_front(std::move(t), calc.config.base,
                                 calc.config.fixed_bits, calc.config.precision,
                                 calc.config.is_signed, calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct to_unix_ts : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"2unix"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x 2unix\n"
            "\n"
            "    return a unix timestamp based on the date-time x"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // single required arg provided by num_args
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            throw units_prohibited();
        }
        const time_* v = std::get_if<time_>(&e.value());
        if (!v)
        {
            throw std::invalid_argument("requires a time type");
        }
        calc.stack.pop_front();

        mpq ts = v->value;
        calc.stack.emplace_front(ts, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct calendar : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"cal"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: D cal\n"
            "\n"
            "    Print a month calendar containing the absolute time D"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 1)
        {
            throw std::invalid_argument("Requires 1 argument");
        }
        stack_entry a = calc.stack.front();
        const time_* t = std::get_if<time_>(&a.value());
        if (!t || !t->absolute)
        {
            throw std::invalid_argument("Value must be an absolute time type");
        }
        calc.stack.pop_front();
        // ymd is not time-zone aware
        auto tz_offset_nanos = []() {
            time_t gmt, ltime = time(NULL);
            struct tm* ptm;
            struct tm gmbuf;
            ptm = gmtime_r(&ltime, &gmbuf);
            ptm->tm_isdst = -1;
            gmt = mktime(ptm);
            return static_cast<long long>(difftime(ltime, gmt)) *
                   1'000'000'000ll;
        };
        // generate a calendar
        // get year and month: time_ -> duration -> time_point -> year_month_day
        long long nanos =
            static_cast<long long>((helper::numerator(t->value) * one_billion) /
                                   helper::denominator(t->value)) +
            tz_offset_nanos();
        std::chrono::duration d = std::chrono::nanoseconds(nanos);
        std::chrono::time_point<std::chrono::system_clock> tp(d);
        const std::chrono::year_month_day ymd{
            std::chrono::floor<std::chrono::days>(tp)};
        auto y = int(ymd.year());
        auto m = unsigned(ymd.month()) - 1; // 0-indexed, please

        // for jan/feb, subtract 1 from year and treat them as
        //    months from 'next year'
        if (m == 0 || m == 1)
        {
            y--;
        }
        // century anchor: 2022 100 / floor 4 % 5 * 2 + 7 %
        int ca = (((y / 100) % 4) * 5 + 2) % 7;
        // year anchor: 2022 100 % dup 4 / floor + <ca> + 7 %
        int ya = (y % 100 + (y % 100) / 4 + ca) % 7;
        // first doomsday by the month
        static constexpr auto doomsdays =
            std::to_array<int>({2, 6, 7, 4, 2, 6, 4, 1, 5, 3, 7, 5});
        static constexpr auto dow{"Su Mo Tu We Th Fr Sa"};
        static constexpr auto moy = std::to_array<const char*>(
            {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
             "Oct", "Nov", "Dec"});
        int first_dom = (ya - doomsdays[m]) % 7;
        if (first_dom < 0)
        {
            first_dom += 7;
        }
        lg::debug("ca = {}, ya = {}, doomsdays[{}] = {}, first_dom = {}\n", ca,
                  ya, m, doomsdays[m], first_dom);
        auto ui = ui::get();
        auto last_day =
            unsigned((ymd.year() / ymd.month() / std::chrono::last).day());
        unsigned num_weeks = std::ceil((1 + first_dom + last_day) / 7.0);
        /*
        Make it loook like this:
        ------------------------
        2022 / Jul
        Su Mo Tu We Th Fr Sa
                        1  2
         3  4  5  6  7  8  9
        10 11 12 13 14 15 16
        17 18 19 20 21 22 23
        24 25 26 27 28 29 30
        31
        */
        ui->out("{} / {} / {}\n", int(ymd.year()), moy[m], unsigned(ymd.day()));
        ui->out("{}\n", dow);
        int dom = 0 - first_dom;
        for (unsigned w = 0; w < num_weeks; w++)
        {
            std::string days{};
            days.reserve(24);
            for (int di = 0; di < 7 && dom <= static_cast<int>(last_day);
                 di++, dom++)
            {
                if (dom <= 0)
                {
                    days.append("  ");
                }
                else
                {
                    days.append(std::format("{:2d}", dom));
                }
            }
            ui->out("{}\n", days);
        }
        return true;
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(unix_ts);
register_calc_fn(date_time);
register_calc_fn(to_unix_ts);
register_calc_fn(to_date_time);
register_calc_fn(calendar);
