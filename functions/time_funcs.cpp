/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <chrono>
#include <function.hpp>

namespace function
{

struct unix_ts : public CalcFunction
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
            "    return a unix timestamp with sub-second precision based\n"
            "    on the system clock"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        time_ now(std::chrono::system_clock::now());
        calc.stack.emplace_front(now, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        return true;
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
            static_cast<long long>(
                (helper::numerator(t->value) * mpz(1'000'000'000ll)) /
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
        static constexpr auto dow = std::to_array<const char*>(
            {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"});
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
        unsigned num_weeks = ceil((1 + first_dom + last_day) / 7.0);
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
        ui->out("{}\n", fmt::join(dow, " "));
        int dom = 0 - first_dom;
        for (unsigned w = 0; w < num_weeks; w++)
        {
            std::vector<std::string> days{};
            for (int di = 0; di < 7 && dom <= static_cast<int>(last_day);
                 di++, dom++)
            {
                if (dom <= 0)
                {
                    days.emplace_back("  ");
                }
                else
                {
                    days.push_back(fmt::format("{:2d}", dom));
                }
            }
            ui->out("{}\n", fmt::join(days, " "));
        }
        return true;
    }
};

} // namespace function

register_calc_fn(unix_ts);
register_calc_fn(calendar);
