/*!
@file

@copyright Barrett Adair 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

#ifndef CALLABLE_TRAITS_DETAIL_GENERALIZED_CLASS_HPP
#define CALLABLE_TRAITS_DETAIL_GENERALIZED_CLASS_HPP

#include <callable_traits/detail/utility.hpp>
#include <type_traits>
#include <memory>


namespace callable_traits {

    template<typename T>
    struct generalized_class {
        using type = typename std::remove_reference<T>::type;
        using original_type = T;
    };
}

#endif
