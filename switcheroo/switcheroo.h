#ifndef SWITCHEROO_H
#define SWITCHEROO_H

#include <tuple>
#include <variant>

namespace switcheroo
{
namespace detail
{
/// @brief Check if the type T is in the tuple or variant
/// @tparam T the type to search for
/// @tparam Tuple the tuple (or variant) to search in
template<typename T, typename Tuple>
struct TypeIn;

// Specialization for std::tuple
template<typename T, typename... Types>
struct TypeIn<T, std::tuple<Types...>>
    : std::disjunction<std::is_same<T, Types>...> {
};

// Specialization for std::variant
template<typename T, typename... Types>
struct TypeIn<T, std::variant<Types...>>
    : std::disjunction<std::is_same<T, Types>...> {
};

/// @brief Get the index of a type in a tuple or variant
/// @tparam T the type to search for
/// @tparam Container the tuple (or variant) to search in
template<class T, class Container>
struct IndexOf;

// Specialization for std::tuple
template<class T, class... Types>
struct IndexOf<T, std::tuple<T, Types...>> {
    static const std::size_t value = 0;
};

template<class T, class U, class... Types>
struct IndexOf<T, std::tuple<U, Types...>> {
    static const std::size_t value
        = 1 + IndexOf<T, std::tuple<Types...>>::value;
};

// Specialization for std::variant
template<class T, class... Types>
struct IndexOf<T, std::variant<T, Types...>> {
    static const std::size_t value = 0;
};

template<class T, class U, class... Types>
struct IndexOf<T, std::variant<U, Types...>> {
    static const std::size_t value
        = 1 + IndexOf<T, std::variant<Types...>>::value;
};

/// @brief Given an std::integer_sequence, return a tuple of integral constants
/// with the same values, e.g.:
// std::integer_sequence<int, 1, 2, 3> ->
// std::tuple
//    <
//    std::integral_constant<int, 1>,
//    std::integral_constant<int, 2>,
//    std::integral_constant<int, 3>
//    >
/// @tparam T the type of the integer sequence
template<typename T>
struct ToTupleOfIntegralConstants;

template<typename T, T... Is>
struct ToTupleOfIntegralConstants<std::integer_sequence<T, Is...>> {
    using type = std::tuple<std::integral_constant<T, Is>...>;
};

/// @brief Given two tuples, return a tuple with the types that are in the
/// second but not in the first tuple, e.g.:
/// std::tuple<int, char> and std::tuple<int, double, char> --become-->
/// std::tuple<double>
/// @tparam Tuple1 the first tuple
/// @tparam Tuple2 the second tuple
template<typename Tuple1, typename Tuple2>
struct MissingTypes;

template<typename... Types1, typename... Types2>
struct MissingTypes<std::tuple<Types1...>, std::tuple<Types2...>> {
    using type = decltype(std::tuple_cat(
        std::conditional_t<TypeIn<Types2, std::tuple<Types1...>>::value,
                           std::tuple<>,
                           std::tuple<Types2>>{}...));
};

/// @brief Given an element and an integer sequence, return a tuple that
/// contains the element repeated as many times as the integer sequence
///  e.g multiplyInTuple(5, std::index_sequence<0, 1, 2>) -> std::tuple<5, 5, 5>
/// @tparam T the type of the element
/// @tparam ...Is the integer sequence
/// @param element the element to repeat
/// @param Is the integer sequence
/// @return a tuple with the element repeated as many times as the integer
template<typename T, std::size_t... Is>
auto multiplyInTuple(T&& element, std::index_sequence<Is...>)
{
    return std::make_tuple(((void)Is, std::forward<T>(element))...);
}

/// @brief Wrap a lambda that takes no arguments into a lambda that takes a
/// single argument that is ignored
/// @tparam Lambda the type of the lambda
/// @param lambda the lambda to wrap
/// @return A new lambda that takes a single argument that is ignored if the
/// original lambda is invocable without arguments otherwise the original lambda
template<typename Lambda>
auto maybeWrapLambdaWithInputArg(Lambda&& lambda)
{
    constexpr bool isInvocableWithoutArgs{std::is_invocable<Lambda>::value};
    if constexpr (isInvocableWithoutArgs) {
        return [l = std::forward<Lambda>(lambda)](auto&&) { return l(); };
    } else {
        return std::forward<Lambda>(lambda);
    }
}

} // namespace detail

/// @brief  The main class of the library, used to build matchers
/// @note This class is not meant to be used directly, use the `match` function
/// @tparam Variant the type of the variant to match, i.e. the equivalent of the
/// type that would go inside a switch statement
/// @tparam Matchers a tuple of lambdas that will be called when the variant,
/// i.e. the equivalent of body of each case in a switch statement
/// @tparam MatcherArgIndexesT a tuple of integral constants that represent the
/// indexes of the types in the variant, used to determine which matcher
/// corresponds to which type. For example, if the variant is
/// std::variant<int, double, char> and the matchers are
/// std::tuple<lambdaForInt, lambdaForChar, lambdaForDouble>, then the
/// MatcherArgIndexesT would be:
/// std::tuple<std::integral_constant<std::size_t, 0>,
///            std::integral_constant<std::size_t, 2>,
///            std::integral_constant<std::size_t, 1>>
/// We store the indexes of the types in the variant instead of the types
/// themselves to avoid having to instantiate the types in the tuple
/// which would be hard if the types are not default-constructible
/// or consuming resources if the types are expensive to construct.
template<typename Variant, typename Matchers, typename MatcherArgIndexesT>
class MatcherBuilder
{
public:
    /// @brief Construct a new MatcherBuilder object (only for internal use)
    /// @tparam VariantArg the type of the variant
    /// @param variant The variant to match
    /// @param matchers The matchers (e.g. lambdas for every variant type)
    /// @param matcherArgIndexes The indexes of the matched types (used
    /// internally to determine which matcher corresponds to which type)
    template<typename VariantArg>
    MatcherBuilder(VariantArg&& variant,
                   Matchers matchers,
                   MatcherArgIndexesT matcherArgIndexes)
        : mVariant{std::forward<VariantArg>(variant)}
        , mMatchers{std::move(matchers)}
        , mMatcherArgIndexes{std::move(matcherArgIndexes)}
    {
    }

    /// @brief Add a matcher for the specified type(s)
    /// @note All your matchers must be added before calling `otherwise`
    /// @note You can't match the same type twice
    /// @note All matchers should return the same type
    /// @tparam ...TypesToMatch the type(s) to match (e.g. int, double, etc.)
    /// @tparam Matcher the type of the matcher (e.g. a lambda)
    /// @param matcher a lambda that takes a single MatcherArg argument
    /// @return a new MatcherBuilder object with the added matcher
    template<typename... TypesToMatch, typename Matcher>
    [[nodiscard]] auto when(Matcher&& matcher) const
    {
        constexpr bool isInvocableWithOneArg
            = (std::is_invocable<Matcher, TypesToMatch>::value && ...);
        constexpr bool isInvocableWithoutArgs
            = std::is_invocable<Matcher>::value;

        static_assert(isInvocableWithOneArg || isInvocableWithoutArgs,
                      "Matcher must be callable with one of the provided types "
                      "or without arguments");

        const auto newMatchers = std::tuple_cat(
            mMatchers,
            detail::multiplyInTuple(
                detail::maybeWrapLambdaWithInputArg(
                    std::forward<Matcher>(matcher)),
                std::make_index_sequence<sizeof...(TypesToMatch)>{}));
        using NewMatchersType = decltype(newMatchers);

        static_assert((detail::TypeIn<TypesToMatch, Variant>::value && ...),
                      "Matcher type not found in variant");

        const auto newMatcherArgIndexes = std::tuple_cat(
            mMatcherArgIndexes,
            std::tuple<std::integral_constant<
                std::size_t,
                detail::IndexOf<TypesToMatch, Variant>::value>...>{});
        using NewMatcherArgIndexesType
            = std::decay_t<decltype(newMatcherArgIndexes)>;

        return MatcherBuilder<Variant,
                              NewMatchersType,
                              NewMatcherArgIndexesType>{
            mVariant, newMatchers, newMatcherArgIndexes};
    }
    /// @brief The matcher to use when no other matcher matches
    /// @tparam NewFallbackMatcher the type of the fallback matcher
    /// @param fallbackMatcher the matcher to use when no other matcher
    /// @return a new MatcherBuilder object with the added fallback matcher
    template<typename NewFallbackMatcher>
    [[nodiscard]] auto otherwise(NewFallbackMatcher&& fallbackMatcher) const
    {
        // Check if the matcher is invocable with one argument or without
        // The type should be auto or no argument at all, so we test with int
        // for the case of auto
        constexpr bool isInvocableWithOneArg
            = std::is_invocable<NewFallbackMatcher, int>::value;
        constexpr bool isInvocableWithoutArgs
            = std::is_invocable<NewFallbackMatcher>::value;

        static_assert(isInvocableWithOneArg || isInvocableWithoutArgs,
                      "The fallback matcher must be callable with auto or "
                      "without arguments");

        // Get the missing indexes and store them in a tuple
        using AllIndexesAsIntegralConstants =
            typename detail::ToTupleOfIntegralConstants<
                std::make_index_sequence<std::variant_size_v<Variant>>>::type;
        using MissingIndexesType =
            typename detail::MissingTypes<MatcherArgIndexesT,
                                          AllIndexesAsIntegralConstants>::type;

        const auto fallbackMatchers = detail::multiplyInTuple(
            detail::maybeWrapLambdaWithInputArg(
                std::forward<NewFallbackMatcher>(fallbackMatcher)),
            std::make_index_sequence<std::tuple_size_v<MissingIndexesType>>{});

        auto newMatchers      = std::tuple_cat(mMatchers, fallbackMatchers);
        using NewMatchersType = decltype(newMatchers);
        const auto newMatcherArgIndexes
            = std::tuple_cat(mMatcherArgIndexes, MissingIndexesType{});
        using NewMatcherArgIndexesType
            = std::decay_t<decltype(newMatcherArgIndexes)>;

        return MatcherBuilder<Variant,
                              NewMatchersType,
                              NewMatcherArgIndexesType>{
            mVariant, newMatchers, newMatcherArgIndexes};
    }

    /// @brief Run the matchers and return the result
    /// @return the result of the matched lambda
    [[nodiscard]] auto run() const
    {
        static_assert(
            std::tuple_size_v<MatcherArgIndexesT>
                == std::variant_size_v<Variant>,
            "You need to match all types of the variant or provide a fallback "
            "matcher with `otherwise`. Also, you may not match all types of "
            "variant and provide a fallback matcher at the same time");

        return std::visit(
            [this](auto&& arg) {
                using ArgT          = std::decay_t<decltype(arg)>;
                const auto argIndex = detail::IndexOf<ArgT, Variant>::value;
                using ArgIndexType
                    = std::integral_constant<std::size_t, argIndex>;
                const auto index
                    = detail::IndexOf<ArgIndexType, MatcherArgIndexesT>::value;
                return std::get<index>(mMatchers)(arg);
            },
            mVariant);
    }

private:
    Variant mVariant;
    Matchers mMatchers;
    MatcherArgIndexesT mMatcherArgIndexes;
};

/// @brief Create a new Matcher, this is the entry point of the library
/// @tparam Variant the type of the variant to match
/// @param variant The variant that contains all the different "cases" to match
/// as types, e.g. std::variant<Jan, Feb, Mar> assumes there are three different
/// cases to match (Jan, Feb, Mar) where Jan, Feb, Mar are types (e.g. structs)
/// @return a new MatcherBuilder object to add matchers using the
/// MatcherBuilder::when and MatcherBuilder::otherwise methods
template<typename Variant>
auto match(Variant&& variant)
{
    return MatcherBuilder<std::decay_t<Variant>, std::tuple<>, std::tuple<>>{
        std::forward<Variant>(variant), std::tuple<>{}, std::tuple<>{}};
}
} // namespace switcheroo

#endif // SWITCHEROO_H
