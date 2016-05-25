/*<-
Copyright Barrett Adair 2016
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http ://boost.org/LICENSE_1_0.txt)
->*/

#include <type_traits>
#include "constexpr_checks.hpp"

using constexpr_checks::is_constexpr;

//this is a constexpr function object (non-templated)
struct zero {

    constexpr auto operator()() const {
        return 0;
    }
};

static_assert(is_constexpr<zero>(), "");
static_assert(is_constexpr(zero{}), "");



//this is a constexpr function object (templated)
struct subtract {

    // For callable_traits::is_constexpr, generic function objects
    // only need to be SFINAE-friendly if the body of the operator()
    // function accesses member names besides "type" and "value".
    // Unary/binary operators and constructor calls are okay to use.
    template<typename T1, typename T2>
    constexpr auto operator()(T1, T2) const {
        return T1{} - T2{};
    }
};

static_assert(is_constexpr<subtract>(), "");
static_assert(is_constexpr(subtract{}), "");



//this is NOT a constexpr function object
struct add {
    template<typename T1, typename T2>
    auto operator()(T1, T2) const {
        return T1{} + T2{};
    }
};

static_assert(!is_constexpr<add>(), "");
static_assert(!is_constexpr(add{}), "");

auto multiply = [](auto t1, auto t2) -> decltype(t1.value * t2.value) {
    return t1.value * t2.value;
};

static_assert(!is_constexpr<decltype(multiply)>(), "");
static_assert(!is_constexpr(multiply), "");


// is_constexpr will always return std::false_type when the argument
// is either not a literal type, or is not default constructible. Below,
// divide is not default constructible, so is_constexpr returns
// std::false_type. For literal types that are default constructible, a
// constexpr default constructor is assumed.

struct divide {

    divide() = delete;
    constexpr divide(int){};

    template<typename T1, typename T2>
    constexpr auto operator()(T1, T2) const {
        return T1{} / T2{};
    }
};

static_assert(!is_constexpr<divide>(), "");
static_assert(!is_constexpr(divide{0}), "");

int main() {}
