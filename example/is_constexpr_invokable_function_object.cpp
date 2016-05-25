/*<-
Copyright Barrett Adair 2016
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http ://boost.org/LICENSE_1_0.txt)
->*/

#include <type_traits>
#include "constexpr_checks.hpp"

using namespace constexpr_checks;

using T1 = std::integral_constant<int, 3>;
using T2 = std::integral_constant<int, 7>;

//'subtract' is a constexpr function object that
// subtracts std::integral_constant types.
struct subtract {

    // To compile failing cases of is_constexpr_invokable, the function object
    // must have a SFINAE-safe signature. In this case, 'subtract' is made
    // SFINAE-safe with an explicit, trailing return type.
    template<typename T1, typename T2>
    constexpr auto operator()(T1, T2) const -> decltype(T1::value - T2::value) {
        return T1::value - T2::value;
    }
};

// is_constexpr_invokable returns std::true_type in the first case, because
// INVOKE(subtract{}, T1{}, T2{}) is a valid expression, AND 'subtract{}' is
// a constexpr function object.
static_assert(is_constexpr_invokable(subtract{}, T1{}, T2{}), "");
static_assert(!is_constexpr_invokable(subtract{}, 3, 7), "");
static_assert(!is_constexpr_invokable(subtract{}, T1{}), "");

//this is a function object, but is NOT constexpr
struct add {
    template<typename T1, typename T2>
    auto operator()(T1, T2) const -> decltype(T1::value + T2::value) {
        return T1::value + T2::value;
    }
};

// Even though INVOKE(add{}, T1{}, T2{}) is valid, the respective
// is_constexpr_invokable call returns std::false_type because 'add{}'
// is not a constexpr function object.
static_assert(!is_constexpr_invokable(add{}, T1{}, T2{}), "");
static_assert(!is_constexpr_invokable(add{}, 3, 7), "");


// This last section demonstrates that is_constexpr_invokable will always
// return std::false_type when any of the arguments do not decay to literal
// types. (see http://en.cppreference.com/w/cpp/concept/LiteralType).
// Even though 'S' below is a constexpr function object, it is incompatible
// with is_constexpr_invokable because 'S' isn't a literal type. Additionally,
// all arguments must be default constructible.

struct S {
    S() = delete;
    S(int){};
    constexpr int operator()() const { return 0; }
};

S s{0};
static_assert(!is_constexpr_invokable(s), "");


int main() {}
