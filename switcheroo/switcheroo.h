#ifndef SWITCHEROO_H
#define SWITCHEROO_H

#include <tuple>
#include <variant>

namespace switcheroo
{
namespace detail
{
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

// ToTupleOfIntegralConstants
// Given an std::integer_sequence, return a tuple of integral constants with
// the same values
template<typename T>
struct ToTupleOfIntegralConstants;

template<typename T, T... Is>
struct ToTupleOfIntegralConstants<std::integer_sequence<T, Is...>> {
    using type = std::tuple<std::integral_constant<T, Is>...>;
};

// MissingTypes
// Given two tuples, return a tuple with the types that are in the second tuple
// but not in the first tuple
template<typename Tuple1, typename Tuple2>
struct MissingTypes;

template<typename... Types1, typename... Types2>
struct MissingTypes<std::tuple<Types1...>, std::tuple<Types2...>> {
    using type = decltype(std::tuple_cat(
        std::conditional_t<TypeIn<Types2, std::tuple<Types1...>>::value,
                           std::tuple<>,
                           std::tuple<Types2>>{}...));
};

// Given an element (first argument) and an integer sequence, return a tuple
// that contains the element repeated as many times as the integer sequence
template<typename T, std::size_t... Is>
auto multiplyInTuple(T&& element, std::index_sequence<Is...>)
{
    return std::make_tuple(((void)Is, std::forward<T>(element))...);
}

} // namespace detail

template<typename Variant, typename Matchers, typename MatcherArgIndexesT>
class MatcherBuilder
{
public:
    MatcherBuilder(Variant variant,
                   Matchers matchers,
                   MatcherArgIndexesT matcherArgIndexes)
        : mVariant{std::move(variant)}
        , mMatchers{std::move(matchers)}
        , mMatcherArgIndexes{std::move(matcherArgIndexes)}
    {
    }

    template<typename MatcherArg, typename Matcher>
    [[nodiscard]] auto when(Matcher&& matcher) const
    {
        auto newMatchers = std::tuple_cat(
            mMatchers, std::tuple<Matcher>{std::forward<Matcher>(matcher)});
        using NewMatchersType = decltype(newMatchers);

        static_assert(detail::TypeIn<MatcherArg, Variant>::value,
                      "Matcher type not found in variant");

        const auto matcherArgIndex
            = detail::IndexOf<MatcherArg, Variant>::value;
        using MatcherArgIndexType
            = std::integral_constant<std::size_t, matcherArgIndex>;
        static_assert(
            !detail::TypeIn<MatcherArgIndexType, MatcherArgIndexesT>::value,
            "Type already matched, cannot match again. If you haven't matched "
            "this argument before, please ensure that `otherwise` is the last "
            "matcher");

        const auto newMatcherArgIndexes = std::tuple_cat(
            mMatcherArgIndexes,
            std::tuple<MatcherArgIndexType>{MatcherArgIndexType{}});
        using NewMatcherArgIndexesType
            = std::decay_t<decltype(newMatcherArgIndexes)>;

        return MatcherBuilder<Variant,
                              NewMatchersType,
                              NewMatcherArgIndexesType>{
            mVariant, newMatchers, newMatcherArgIndexes};
    }

    template<typename NewFallbackMatcher>
    [[nodiscard]] auto otherwise(NewFallbackMatcher&& fallbackMatcher) const
    {
        // Get the missing indexes and store them in a tuple
        using AllIndexesAsIntegralConstants =
            typename detail::ToTupleOfIntegralConstants<
                std::make_index_sequence<std::variant_size_v<Variant>>>::type;
        using MissingIndexesType =
            typename detail::MissingTypes<MatcherArgIndexesT,
                                          AllIndexesAsIntegralConstants>::type;

        const auto fallbackMatchers = detail::multiplyInTuple(
            std::forward<NewFallbackMatcher>(fallbackMatcher),
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

template<typename Variant>
auto match(Variant variant)
{
    return MatcherBuilder{variant, std::tuple<>{}, std::tuple<>{}};
}
} // namespace switcheroo

#endif // SWITCHEROO_H
