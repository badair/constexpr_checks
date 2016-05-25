/*!<-
Copyright (c) 2016 Barrett Adair

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
->*/

#include <type_traits>
#include "constexpr_checks.hpp"

using namespace constexpr_checks;

constexpr int seven(int) {
    return 7;
}

using seven_c = std::integral_constant<decltype(&seven), &seven>;

// The first call to is_constexpr_invokable returns std::true_type
// because `seven` is a constexpr function, and valid INVOKE arguments
// are passed. The second call to is_constexpr_invokable returns
// std::false_type, because the arguments are not valid to INVOKE
static_assert(is_constexpr_invokable(seven_c{}, 0), "");
static_assert(!is_constexpr_invokable(seven_c{}, nullptr), "");

int eight(int) {
    return 7;
}

using eight_c = std::integral_constant<decltype(&eight), &eight>;

// `eight` is NOT a constexpr function, so is_constexpr_invokable
// returns `std::false_type` even for valid INVOKE arguments.
static_assert(!is_constexpr_invokable(eight_c{}, 0), "");
static_assert(!is_constexpr_invokable(eight_c{}, nullptr), "");

int main() {}
