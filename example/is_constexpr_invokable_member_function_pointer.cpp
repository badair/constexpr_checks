/*!<-
Copyright (c) 2016 Barrett Adair

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
->*/

#include <type_traits>
#include "constexpr_checks.hpp"

using namespace constexpr_checks;

struct foo {
    constexpr int bar(int) const {
        return 1;
    }
};

using pmf_constant = std::integral_constant<decltype(&foo::bar), &foo::bar>;

// is_constexpr_invokable returns true here because foo::bar
// is constexpr, and the arguments are valid to INVOKE
static_assert(is_constexpr_invokable(pmf_constant{}, foo{}, 0), "");

// is_constexpr_invokable returns false here because even though
// foo::bar is constexpr, the arguments do not obey INVOKE rules
static_assert(!is_constexpr_invokable(pmf_constant{}, foo{}), "");

int main() {}
