/*!
Copyright (c) 2016 Barrett Adair

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/

#include "constexpr_checks.hpp"

#ifndef CC_ASSERT
#define CC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)
#endif //CC_ASSERT

struct foo {

    constexpr int operator()(int&) & {
        return 1;
    }

    constexpr int operator()(const int&) & {
        return 2;
    }

    constexpr int operator()(int&&) & {
        return 3;
    }

    constexpr int operator()(int&) && {
        return 4;
    }

    constexpr int operator()(const int&) && {
        return 5;
    }

    constexpr int operator()(int&&) && {
        return 6;
    }

    constexpr int operator()(int&) const & {
        return 7;
    }

    constexpr int operator()(const int&) const & {
        return 8;
    }

    constexpr int operator()(int&&) const & {
        return 9;
    }

    constexpr int operator()(int&) const && {
        return 10;
    }

    constexpr int operator()(const int &) const && {
        return 11;
    }

    constexpr int operator()(int&&) const && {
        return 12;
    }
};


CC_ASSERT(1 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&)));
CC_ASSERT(2 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const int&)));
CC_ASSERT(3 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&&)));
CC_ASSERT(3 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int)));

CC_ASSERT(4 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&)));
CC_ASSERT(4 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&)));
CC_ASSERT(5 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const int&)));
CC_ASSERT(5 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const int&)));
CC_ASSERT(6 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&&)));
CC_ASSERT(6 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&&)));
CC_ASSERT(6 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int)));
CC_ASSERT(6 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int)));

CC_ASSERT(7 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&)));
CC_ASSERT(8 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const int&)));
CC_ASSERT(9 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&&)));
CC_ASSERT(9 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int)));

CC_ASSERT(10 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&)));
CC_ASSERT(10 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&)));
CC_ASSERT(11 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const int&)));
CC_ASSERT(11 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const int&)));
CC_ASSERT(12 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&&)));
CC_ASSERT(12 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int&&)));
CC_ASSERT(12 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo&&)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int)));
CC_ASSERT(12 == CONSTEXPR_CHECKS_MAKE_CONSTEXPR(const foo)(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(int)));

int main() { return 0; }
