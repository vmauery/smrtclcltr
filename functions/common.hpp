/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include <function.hpp>

namespace smrty
{
namespace function
{

struct add : public smrty::CalcFunction
{
    virtual const std::string& name() const final;
    virtual const std::string& help() const final;
    virtual bool op(Calculator& calc) const final;
};

struct subtract : public smrty::CalcFunction
{
    virtual const std::string& name() const final;
    virtual const std::string& help() const final;
    virtual bool op(Calculator& calc) const final;
};

struct multiply : public smrty::CalcFunction
{
    virtual const std::string& name() const final;
    virtual const std::string& help() const final;
    virtual bool op(Calculator& calc) const final;
};

struct divide : public smrty::CalcFunction
{
    virtual const std::string& name() const final;
    virtual const std::string& help() const final;
    virtual bool op(Calculator& calc) const final;
};

namespace util
{

mpz factorial(const mpz&);

mpz comb(const mpz& x, const mpz& y);

mpz perm(const mpz& x, const mpz& y);

std::vector<mpz> factor_mpz(const mpz& x);

std::vector<mpz> prime_factor(mpz x);

numeric pow(const mpz& base, const mpz& exponent);

} // namespace util

} // namespace function

} // namespace smrty
