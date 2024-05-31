#ifndef SWITCHEROO_H
#define SWITCHEROO_H

#include <tuple>
#include <variant>

namespace switcheroo
{
template<typename T, typename Tuple>
struct typeInTuple;

template<typename T, typename... Types>
struct typeInTuple<T, std::tuple<Types...>>
    : std::disjunction<std::is_same<T, Types>...> {
};

template<class T, class Tuple>
struct IndexOf;

template<class T, class... Types>
struct IndexOf<T, std::tuple<T, Types...>> {
    static const std::size_t value = 0;
};

template<class T, class U, class... Types>
struct IndexOf<T, std::tuple<U, Types...>> {
    static const std::size_t value
        = 1 + IndexOf<T, std::tuple<Types...>>::value;
};

// Check if the types of a tuple can be found in a variant
template<typename Tuple, typename Variant>
struct TupleTypesMatchVariant;

template<typename... TupleTypes, typename... VariantTypes>
struct TupleTypesMatchVariant<std::tuple<TupleTypes...>,
                              std::variant<VariantTypes...>> {
    static const bool value
        = (typeInTuple<TupleTypes, std::tuple<VariantTypes...>>::value && ...);
};

// Check that tuple types are as many as variant types
template<typename Tuple, typename Variant>
struct TupleSizeMatchVariantSize;

template<typename... TupleTypes, typename... VariantTypes>
struct TupleSizeMatchVariantSize<std::tuple<TupleTypes...>,
                                 std::variant<VariantTypes...>> {
    static const bool value = sizeof...(TupleTypes) == sizeof...(VariantTypes);
};

template<typename Callable>
struct CallableWithoutArgs {
    static const bool value = std::is_invocable<Callable>::value;
};

template<typename Variant,
         typename Matchers,
         typename MatcherReturnT,
         typename MatcherArgsT,
         typename FallbackMatcher,
         bool fallbackProvided = false>
class MatcherBuilder
{
public:
    MatcherBuilder(Variant variant, // TODO: Avoid copy?
                   Matchers matchers,
                   MatcherReturnT matcherReturnTypes,
                   MatcherArgsT matcherArgTypes,
                   FallbackMatcher fallbackMatcher)
        : mVariant{std::move(variant)}
        , mMatchers{std::move(matchers)}
        , mMatcherReturnTypes{std::move(matcherReturnTypes)}
        , mMatcherArgTypes{std::move(matcherArgTypes)}
        , mFallbackMatcher{fallbackMatcher}
    {
    }

    template<typename MatcherArg, typename Matcher>
    [[nodiscard]] auto when(Matcher&& matcher) const
    {
        auto newMatchers = std::tuple_cat(
            mMatchers, std::tuple<Matcher>{std::forward<Matcher>(matcher)});
        using NewMatchersType      = decltype(newMatchers);
        using MatcherReturnType    = std::invoke_result_t<Matcher, MatcherArg>;
        auto newMatcherReturnTypes = std::tuple_cat(
            mMatcherReturnTypes, std::tuple<MatcherReturnType>{});
        using NewMatcherReturnTypes = decltype(newMatcherReturnTypes);
        static_assert(
            std::is_same_v<MatcherReturnType,
                           std::tuple_element_t<0, NewMatcherReturnTypes>>,
            "All matchers must have the same return type");

        auto newMatcherArgTypes
            = std::tuple_cat(mMatcherArgTypes, std::tuple<MatcherArg>{});
        using NewMatcherArgTypes = decltype(newMatcherArgTypes);
        static_assert(!typeInTuple<MatcherArg, MatcherArgsT>::value,
                      "Type already matched, cannot match again");

        static_assert(TupleTypesMatchVariant<MatcherArgsT, Variant>::value,
                      "You may only match on the types of the variant");

        return MatcherBuilder<Variant,
                              NewMatchersType,
                              NewMatcherReturnTypes,
                              NewMatcherArgTypes,
                              FallbackMatcher>{mVariant,
                                               newMatchers,
                                               newMatcherReturnTypes,
                                               newMatcherArgTypes,
                                               mFallbackMatcher};
    }

    template<typename NewFallbackMatcher>
    [[nodiscard]] auto otherwise(NewFallbackMatcher&& fallbackMatcher) const
    {
        static_assert(CallableWithoutArgs<NewFallbackMatcher>::value);
        return MatcherBuilder<Variant,
                              Matchers,
                              MatcherReturnT,
                              MatcherArgsT,
                              NewFallbackMatcher,
                              true /* fallbackProvided */>{
            mVariant,
            mMatchers,
            mMatcherReturnTypes,
            mMatcherArgTypes,
            std::forward<NewFallbackMatcher>(fallbackMatcher)};
    }

    [[nodiscard]] auto run() const
    {
        // Assert that the number of variant types is the same as the number of
        // matcher arguments
        static_assert(
            fallbackProvided
                != TupleSizeMatchVariantSize<MatcherArgsT, Variant>::value,
            "You need to match all types of the variant or provide a fallback "
            "matcher with `otherwise`. Also, you may not match all types of "
            "variant and provide a fallback matcher at the same time");

        return std::visit(
            [this](auto&& arg) {
                using ArgT = std::decay_t<decltype(arg)>;
                if constexpr (typeInTuple<ArgT, MatcherArgsT>::value) {
                    const auto index = IndexOf<ArgT, MatcherArgsT>::value;
                    return std::get<index>(mMatchers)(arg);
                } else {
                    return mFallbackMatcher();
                }
            },
            mVariant);
    }

private:
    Variant mVariant;
    Matchers mMatchers;
    MatcherReturnT mMatcherReturnTypes;
    MatcherArgsT mMatcherArgTypes;
    FallbackMatcher mFallbackMatcher;
};

template<typename Variant>
auto Match(Variant variant)
{
    return MatcherBuilder{
        variant, std::tuple<>(), std::tuple<>(), std::tuple<>(), []() {}};
}
} // namespace switcheroo

#endif // SWITCHEROO_H
