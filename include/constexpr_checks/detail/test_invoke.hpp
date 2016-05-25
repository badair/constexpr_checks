 /*!
@file

@copyright Barrett Adair 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

#ifndef CALLABLE_TRAITS_DETAIL_TEST_INVOKE_HPP
#define CALLABLE_TRAITS_DETAIL_TEST_INVOKE_HPP

#include <callable_traits/detail/fwd/pmf_fwd.hpp>
#include <callable_traits/detail/fwd/pmd_fwd.hpp>
#include <callable_traits/detail/utility.hpp>
#include <callable_traits/detail/make_constexpr.hpp>
#include <callable_traits/config.hpp>

#include <type_traits>
#include <utility>

namespace callable_traits {

    namespace detail {
        
        template<typename T>
        struct success {};

        template<typename...>
        struct test_invoke;

        template<typename Pmf, typename T, typename... Args>
        struct test_invoke<pmf<Pmf>, T, Args...> {

            using class_t = typename pmf<Pmf>::class_type;

           template<typename P, typename U, typename... Rgs,
                typename Obj = generalize_if_dissimilar<class_t, U&&>>
            auto operator()(P&& p, U&& u, Rgs&&... rgs) const ->
                success<decltype((std::declval<Obj>().*p)(std::forward<Rgs>(rgs)...))>;

            auto operator()(...) const -> substitution_failure;

            static constexpr int arg_count = sizeof...(Args) - 1;
        };

        template<typename Pmd, typename... Args>
        struct test_invoke<pmd<Pmd>, Args...> {

            using class_t = typename pmd<Pmd>::class_type;

            template<typename P, typename U,
                typename Obj = generalize_if_dissimilar<class_t, U&&>>
            auto operator()(P&& p, U&& u) const ->
                success<decltype((std::declval<Obj>().*p))>;

            auto operator()(...) const -> substitution_failure;

            static constexpr int arg_count = 0;
        };

        template<typename F, typename... Args>
        struct test_invoke<F, Args...> {

            template<typename T, typename... Rgs,
                typename U = unwrap_reference<T&&>>
            auto operator()(T&& t, Rgs&&... rgs) const ->
                success<decltype(std::declval<U>()(std::forward<Rgs>(rgs)...))>;

            auto operator()(...) const -> substitution_failure;

            static constexpr int arg_count = sizeof...(Args);
        };


        template<typename IsMemberPointer, typename Traits, typename... Args>
        struct build_invoke_t;

        template<typename Traits, typename... Args>
        struct build_invoke_t<std::false_type, Traits, Args...> {
            using test = detail::test_invoke<Traits, Args...>;
            using original_type = typename Traits::type;

            using result = decltype(test{}(
                ::std::declval<original_type>(),
                ::std::declval<Args>()...
            ));

            using failure = detail::substitution_failure;
            static constexpr bool value = !std::is_same<result, failure>::value;
            static constexpr int arg_count = test::arg_count;
        };

        template<typename Traits, typename... Args>
        struct build_invoke_t<std::true_type, Traits, Args...> {
            using test = test_invoke<Traits, Args...>;
            using original_type = typename Traits::type;
            using invoke_type = typename Traits::invoke_type;

            using result = decltype(test{}(
                ::std::declval<original_type>(),
                ::std::declval<invoke_type>(),
                ::std::declval<Args>()...
            ));

            using failure = detail::substitution_failure;
            static constexpr bool value = !std::is_same<result, failure>::value;
            static constexpr int arg_count = test::arg_count;
        };

        template<typename Traits>
        struct build_invoke_t<std::false_type, Traits, void> {
            using test = test_invoke<Traits>;
            using original_type = typename Traits::type;
            using result = decltype(test{}(static_cast<original_type>(::std::declval<original_type>())));
            using failure = detail::substitution_failure;
            static constexpr bool value = !std::is_same<result, failure>::value;
            static constexpr int arg_count = test::arg_count;
        };

        template<typename Traits>
        struct build_invoke_t<std::true_type, Traits, void> {
            using test = test_invoke<Traits>;
            using original_type = typename Traits::type;
            using invoke_type = typename Traits::invoke_type;

            using result = decltype(test{}(
                ::std::declval<original_type>(),
                static_cast<invoke_type>(::std::declval<invoke_type>())
            ));

            using failure = detail::substitution_failure;
            static constexpr bool value = !std::is_same<result, failure>::value;
            static constexpr int arg_count = test::arg_count;
        };

        template<typename Traits, typename... Args>
        using is_invokable = build_invoke_t<
            typename Traits::is_member_pointer, Traits, Args...>;
    }
}

#endif
