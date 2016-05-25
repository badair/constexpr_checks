/*!
@file

@copyright Barrett Adair 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

#ifndef CONSTEXPR_CHECKS_HPP
#define CONSTEXPR_CHECKS_HPP

#include <callable_traits/args.hpp>
#include <callable_traits/is_invokable.hpp>
#include <callable_traits/qualified_parent_class_of.hpp>
#include <type_traits>
#include <utility>
#include <tuple>

#ifndef CONSTEXPR_CHECKS_VARIADIC_LIMIT
#define CONSTEXPR_CHECKS_VARIADIC_LIMIT 10
#endif

namespace constexpr_checks {

    namespace detail {

#ifdef __cpp_lib_logical_traits


#define CONSTEXPR_CHECKS_NEGATION(...) \
    ::std::negation< __VA_ARGS__ >

#define CONSTEXPR_CHECKS_DISJUNCTION(...) \
    ::std::disjunction< __VA_ARGS__ >

#define CONSTEXPR_CHECKS_CONJUNCTION(...) \
    ::std::conjunction< __VA_ARGS__ >

#else

        // polyfill for C++17 std::negation
        template<typename BoolType>
        using negation = std::integral_constant<bool, !BoolType::value>;

        // polyfill for C++17 std::disjunction (from cppreference.com
        // example implementation)
        template<typename...>
        struct disjunction : std::false_type {};

        template<typename T>
        struct disjunction<T> : T {};

        template<typename T, typename... Ts>
        struct disjunction<T, Ts...> : std::conditional<
            T::value != false, T, disjunction<Ts...>>::type{};

        //polyfill for C++17 std::conjunction (from cppreference.com
        // example implementation)
        template<typename...>
        struct conjunction : std::true_type {};

        template<typename T>
        struct conjunction<T> : T {};

        template<typename T, typename... Ts>
        struct conjunction<T, Ts...> : std::conditional<
            T::value != false, T, conjunction<Ts...>>::type{};

#define CONSTEXPR_CHECKS_NEGATION(...) \
    ::constexpr_checks::detail::negation<__VA_ARGS__>

#define CONSTEXPR_CHECKS_DISJUNCTION(...) \
    ::constexpr_checks::detail::disjunction<__VA_ARGS__>

#define CONSTEXPR_CHECKS_CONJUNCTION(...) \
    ::constexpr_checks::detail::conjunction<__VA_ARGS__>

#endif //#ifdef __cpp_lib_logical_traits

        // a faster version of std::decay_t
        template<typename T>
        using shallow_decay = typename std::remove_cv<
            typename std::remove_reference<T>::type>::type;

        template<typename T, typename U = shallow_decay<T>>
        using is_constexpr_constructible =
            std::integral_constant<bool,
            std::is_literal_type<U>::value
            && std::is_default_constructible<U>::value>;

        template<typename T>
        struct is_integral_constant : std::false_type {};

        template<typename T, T Value>
        struct is_integral_constant<std::integral_constant<T, Value>>
            : std::true_type {
        };

        template<typename Check, typename Result>
        using sfinae_if_not_integral_constant =
            typename std::enable_if<
            is_integral_constant<Check>::value, Result>::type;

        template<typename Check, typename WhenTrue, typename WhenFalse>
        using if_integral_constant = typename std::conditional<
            is_integral_constant<Check>::value, WhenTrue, WhenFalse>::type;

        template<typename T, bool Value>
        struct type_value : T {
            using type = T;
            static constexpr const bool value = Value;
        };

        template<typename Check, typename Result>
        using sfinae_if_integral_constant = typename std::enable_if<!
            is_integral_constant<Check>::value, Result>::type;

        template<typename T>
        struct is_reference_wrapper : std::false_type {};

        template<typename T>
        struct is_reference_wrapper<std::reference_wrapper<T>>
            : std::true_type {
        };

        template<typename T>
        struct can_dereference {

            template<typename>
            struct check {};

            template<typename U>
            static std::true_type test(
                check<typename std::remove_reference<
                decltype(*std::declval<U>())>::type>*);

            template<typename>
            static std::false_type test(...);

            static constexpr const bool value =
                decltype(test<T>(nullptr))::value;
        };

        template<typename T, typename = std::true_type>
        struct generalize_t {
            using type = T;
        };

        template<typename T>
        struct generalize_t<T, std::integral_constant<bool,
            can_dereference<T>::value
            && !is_reference_wrapper<T>::value>>{
            using type = decltype(*std::declval<T>());
        };

        template<typename T>
        struct generalize_t<T, is_reference_wrapper<T>> {
            using type = decltype(std::declval<T>().get());
        };

        // When T is a pointer, generalize<T> is the resulting type of
        // the pointer dereferenced. When T is an std::
        // reference_wrapper, generalize<T> is the underlying reference
        // type. Otherwise, generalize<T> is T.
        template<typename T>
        using generalize = typename generalize_t<T>::type;

        // make_invokable is used to abstract away the member pointer
        // rules of INVOKE for the parent object
        template<typename Base, typename T>
        using make_invokable = typename std::conditional<
            std::is_base_of<Base, shallow_decay<T>>::value
            || std::is_same<Base, shallow_decay<T>>::value,
            T, generalize<T>>::type;

        template<typename T>
        struct arg_tuple_size {
            static constexpr int value = -1;
        };

        template<typename... T>
        struct arg_tuple_size<std::tuple<T...>> {
            static constexpr int value = sizeof...(T);
        };

        template<typename T>
        struct arity {

            template<typename U>
            static callable_traits::args<U> test(int);

            template<typename>
            static std::integral_constant<int, -1> test(...);

            using test_result = decltype(test<T>(0));

            static constexpr int value = CONSTEXPR_CHECKS_DISJUNCTION(
                type_value<if_integral_constant<
                test_result, test_result, std::false_type>,
                is_integral_constant<test_result>::value>,
                type_value<
                std::integral_constant<int, arg_tuple_size<
                test_result>::value>, true>
            )::type::value;
        };

        template<typename T, typename U = std::true_type>
        struct make_constexpr {
            static_assert(sizeof(typename std::decay<T>::type) < 1,
                "Cannot perform constexpr checks with this type, "
                "because either it is not a literal type, or it "
                "is not default constructible.");
        };

        template<typename T>
        struct make_constexpr<T, typename is_constexpr_constructible<
            shallow_decay<T>>::type> {

            using decayed = shallow_decay<T>;

            static constexpr decayed value = decayed{};

            using const_cast_type = typename std::conditional<
                std::is_rvalue_reference<T>::value,
                decayed&&, decayed&>::type;
        };

        // CONSTEXPR_CHECKS_MAKE_CONSTEXPR(T) expands to a reference of a
        // constexpr T object. For this to work, T must be a literal type with
        // a constexpr default constructor.
#define CONSTEXPR_CHECKS_MAKE_CONSTEXPR(T)               \
static_cast<T>(                                          \
    const_cast<typename ::constexpr_checks::detail::     \
        make_constexpr<T>::const_cast_type>(             \
        ::constexpr_checks::detail::                     \
            make_constexpr<T>::value))                   \
/**/

        template<typename T, typename U = shallow_decay<T>>
        using int_if_literal = typename std::enable_if<
            is_constexpr_constructible<U>::value, T>::type;

        struct constexpr_template_worm;

        struct constexpr_template_worm {

            using type = constexpr_template_worm;

            static const constexpr_template_worm value;

            template<typename T, int_if_literal<T> = 0>
            inline constexpr operator T& () const {
                return CONSTEXPR_CHECKS_MAKE_CONSTEXPR(T&);
            }

            template<typename T, int_if_literal<T> = 0>
            inline constexpr operator T && () const {
                return CONSTEXPR_CHECKS_MAKE_CONSTEXPR(T&&);
            }

            inline constexpr constexpr_template_worm() = default;

            inline constexpr constexpr_template_worm(
                const constexpr_template_worm&) = default;

            inline constexpr constexpr_template_worm(
                constexpr_template_worm&&) = default;

            template<typename... T>
            inline constexpr constexpr_template_worm(T&&...) {}

            inline constexpr auto operator=(
                constexpr_template_worm) const { return type{}; }

            template<typename T>
            inline constexpr auto operator=(T&&) const { return type{}; }

            inline constexpr auto operator+() const { return type{}; }
            inline constexpr auto operator-() const { return type{}; }
            inline constexpr auto operator*() const { return type{}; }
            inline constexpr auto operator&() const { return type{}; }
            inline constexpr auto operator!() const { return type{}; }
            inline constexpr auto operator~() const { return type{}; }
            inline constexpr auto operator()(...) const {
                return type{};
            }
        };

        const constexpr_template_worm
            constexpr_template_worm::value = {};

        //template_worm is only used in unevaluated contexts
        struct template_worm : constexpr_template_worm {

            //msvc doesn't like this
            static constexpr const auto value =
                constexpr_template_worm{};

            template<typename T>
            operator T& () const;

            template<typename T>
            operator T && () const;

            template_worm() = default;

            //MSVC doesn't like this because it can deduce 'void'
            template<typename... T>
            template_worm(T&&...);

            template_worm operator+() const;
            template_worm operator-() const;
            template_worm operator*() const;
            template_worm operator&() const;
            template_worm operator!() const;
            template_worm operator~() const;
            template_worm operator()(...) const;
        };

#define CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(...) \
template<typename T>                                        \
constexpr inline constexpr_template_worm                    \
__VA_ARGS__ (constexpr_template_worm, T&&) { return {}; }   \
                                                            \
template<typename T>                                        \
constexpr inline constexpr_template_worm                    \
__VA_ARGS__ (T&&, constexpr_template_worm) { return {}; }   \
                                                            \
constexpr inline constexpr_template_worm                    \
__VA_ARGS__ (constexpr_template_worm,                       \
    constexpr_template_worm) { return {}; }                 \
                                                            \
template_worm __VA_ARGS__ (template_worm, template_worm);   \
                                                            \
template<typename T>                                        \
template_worm __VA_ARGS__ (T&&, template_worm);             \
                                                            \
template<typename T>                                        \
template_worm __VA_ARGS__ (template_worm, T&&);             \
/**/

        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator+)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator-)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator/)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator*)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator==)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator!=)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator&&)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator||)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator|)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator&)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator%)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator,)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator<<)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator>>)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator<)
        CONSTEXPR_CHECKS_TEMPLATE_WORM_BINARY_OPERATOR(operator>)

            template<typename T, typename... Args>
        struct invoke_info {

            static constexpr bool value =
                callable_traits::is_invokable<T, Args...>();

            static constexpr int arg_count = sizeof...(Args);
        };

        template<typename T>
        struct invoke_info<T, void> {

            static constexpr bool value =
                callable_traits::is_invokable<T>();

            static constexpr int arg_count = 0;
        };

        struct sentinel {};

        template<typename U, typename = void>
        struct min_args {

            using result_type = CONSTEXPR_CHECKS_DISJUNCTION(
                invoke_info<U, void>,
                min_args<U, std::make_index_sequence<1>>
            );

            static constexpr int arg_count = result_type::arg_count;
        };

        template<typename U>
        struct min_args<U, sentinel> {
            static constexpr bool value = true;
            static constexpr int arg_count = -1;
        };

        template<typename U, std::size_t... I>
        struct min_args<U, std::index_sequence<I...>> {

            using next = typename std::conditional<
                sizeof...(I)+1 <= CONSTEXPR_CHECKS_VARIADIC_LIMIT,
                std::make_index_sequence<sizeof...(I)+1>,
                sentinel
            >::type;

            using result_type = CONSTEXPR_CHECKS_DISJUNCTION(
                invoke_info<U, decltype((I, std::declval<
                    const template_worm&>()))...>,
                min_args<U, next>
            );

            static constexpr bool value = result_type::value;
            static constexpr int arg_count = result_type::arg_count;
        };

        template<typename T>
        struct variadic_min_arity {
            static constexpr int value = min_args<T>::arg_count;
        };

        template<typename T>
        using min_arity = std::integral_constant<int,
            CONSTEXPR_CHECKS_DISJUNCTION(
                type_value<arity<T>, arity<T>::value >= 0>,
                type_value<variadic_min_arity<T>, true>
            )::type::value
        >;

        template<typename>
        struct test_invoke_constexpr {

            // An arbitrary expression as a left-hand argument in a comma
            // operator expression passed as a template value argument will
            // SFINAE when the expression is not constexpr.

            template<typename T, typename... Rgs,
                typename U = typename std::remove_reference<T>::type>
            inline constexpr auto
            operator()(T&& t, Rgs&&...) const
                ->sfinae_if_not_integral_constant<U,
                    std::integral_constant<bool, (U::value(
                        CONSTEXPR_CHECKS_MAKE_CONSTEXPR(Rgs&&)...
                    ), true)>>;

            template<typename T, typename... Rgs,
                typename U = typename std::remove_reference<T>::type>
            inline constexpr auto
            operator()(T&&, Rgs&&...) const
                ->sfinae_if_integral_constant<U,
                    std::integral_constant<bool,
                    (CONSTEXPR_CHECKS_MAKE_CONSTEXPR(T&&)(
                        CONSTEXPR_CHECKS_MAKE_CONSTEXPR(Rgs&&)...
                        ), true)>>;

            inline constexpr auto
            operator()(...) const -> std::false_type;
        };

        template<typename Member, typename Class>
        struct test_invoke_constexpr<Member Class::*> {

            // Here, P is an integral_constant pointer to member fn.
            template<typename P, typename U, typename... Rgs,
                typename Obj = make_invokable<Class, U&&>>
            inline constexpr auto
            operator()(P&& p, U&&, Rgs&&... rgs) const
                ->sfinae_if_not_integral_constant<P,
                std::integral_constant<bool,
                ((CONSTEXPR_CHECKS_MAKE_CONSTEXPR(Obj).*
                    std::remove_reference<P>::type::value)(
                        CONSTEXPR_CHECKS_MAKE_CONSTEXPR(Rgs&&)...
                        ), true)>>;

            inline constexpr auto
            operator()(...) const->std::false_type;
        };

        template<typename... Ts>
        using are_all_constexpr_constructible =
            CONSTEXPR_CHECKS_CONJUNCTION(
                is_constexpr_constructible<Ts>...);

        template<typename T, bool = is_integral_constant<T>::value>
        struct unwrap_integral_constant {
            using type = T;
        };

        template<typename T>
        struct unwrap_integral_constant<T, true> {
            using type = shallow_decay<decltype(T::value)>;
        };

        template<typename T>
        struct unwrap_if_integral_constant {

            using no_ref = shallow_decay<T>;

            using unwrapped =
                typename unwrap_integral_constant<no_ref>::type;

            using type = typename std::conditional<
                std::is_class<unwrapped>::value, T, unwrapped>::type;
        };

        template<typename T, typename... Args, typename std::enable_if<
            CONSTEXPR_CHECKS_NEGATION(are_all_constexpr_constructible<
                T, Args...>)::value, int>::type = 0>
        inline constexpr std::false_type
        is_constexpr_invokable_impl(T&&, Args&&...) { return{}; }

        template<typename T, typename... Args, typename std::enable_if<
            are_all_constexpr_constructible<T, Args...>::value,
            int>::type = 0>
        inline constexpr auto
        is_constexpr_invokable_impl(T&& t, Args&&... args) {

            using type =
                typename unwrap_if_integral_constant<T&&>::type;

            using test = test_invoke_constexpr<type>;

            using result = decltype(test{}(::std::forward<T>(t),
                ::std::forward<Args>(args)...));

            return std::integral_constant<bool, result::value>{};
        }

        template<bool, typename T, typename... Args>
        struct is_constexpr_invokable_impl_types {
            using type = std::false_type;
        };

        template<typename T, typename... Args>
        struct is_constexpr_invokable_impl_types<true, T, Args...> {

            using test_type =
                typename unwrap_if_integral_constant<T>::type;

            using test = test_invoke_constexpr<test_type>;

            using result = decltype(test{}(
                ::std::declval<T>(),
                ::std::declval<Args>()...));

            using type = std::integral_constant<bool, result::value>;
        };

        template<typename F, typename Seq>
        struct is_constexpr_t;

        template<typename F, std::size_t... I>
        struct is_constexpr_t<F, std::index_sequence<I...>> {

            using worm =
                ::constexpr_checks::detail::constexpr_template_worm;

            template<typename T,
                typename U = typename std::remove_reference<T>::type>
            inline constexpr auto
            operator()(T&&) const
                ->sfinae_if_not_integral_constant<U,
                std::integral_constant<bool, (static_cast<void>(U::value(
                    worm{ I }...
                )), true)>>;

            template<typename T,
                typename U = typename std::remove_reference<T>::type>
            inline constexpr auto
            operator()(T&&) const
                ->sfinae_if_integral_constant<U,
                std::integral_constant<bool,
                (static_cast<void>(CONSTEXPR_CHECKS_MAKE_CONSTEXPR(T&&)(
                    worm{ I }...
                    )), true)>>;

            inline constexpr auto
            operator()(...) const->std::false_type;
        };

        template<typename Member, typename Class, std::size_t... I>
        struct is_constexpr_t<Member Class::*,
            std::index_sequence<I...>> {

            using worm =
                ::constexpr_checks::detail::constexpr_template_worm;

            using invoke_type =
                callable_traits::qualified_parent_class_of<
                Member Class::*>;

            template<typename T>
            inline constexpr auto
            operator()(T&& t) {
                return decltype(::constexpr_checks::detail::
                    test_invoke_constexpr<Member Class::*>{}(
                        ::std::forward<T>(t),
                        ::std::declval<invoke_type>(),
                        worm{ I }...)){};
            }
        };

        template<typename T>
        inline constexpr std::false_type
        is_constexpr_impl(T&&, std::false_type) { return{}; }

        template<typename T>
        inline constexpr auto
        is_constexpr_impl(T&& t, std::true_type) {

            using type =
                typename unwrap_if_integral_constant<T&&>::type;

            using this_offset = std::integral_constant<int,
                std::is_member_pointer<type>::value ? 1 : 0>;

            // need to remove the INVOKE-required object
            using min_args = std::integral_constant<int,
                min_arity<type>::value - this_offset::value>;

            using seq = std::make_index_sequence<
                min_args::value < 0 ? 0 : min_args::value>;

            return decltype(is_constexpr_t<type, seq>{}(
                ::std::forward<T>(t))){};
        }
    }

    template<typename T, typename... Args>
    inline constexpr auto
    is_constexpr_invokable(T&& t, Args&&... args) {
        return detail::is_constexpr_invokable_impl(
            ::std::forward<T>(t),
            ::std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    inline constexpr auto
    is_constexpr_invokable() {

        using are_constexpr_constructible =
            detail::are_all_constexpr_constructible<Args...>;

        return typename detail::is_constexpr_invokable_impl_types<
            are_constexpr_constructible::value, Args...>::type{};
    }

    template<typename T>
    inline constexpr auto
    is_constexpr(T&& t) {

        return ::constexpr_checks::detail::is_constexpr_impl(
            ::std::forward<T>(t),
            detail::is_constexpr_constructible<T>{}
        );
    }

    template<typename T>
    inline constexpr auto
    is_constexpr() {
        return decltype(::constexpr_checks::
            is_constexpr(std::declval<T>())){};
    }
}

#endif //#ifndef CONSTEXPR_CHECKS_HPP
