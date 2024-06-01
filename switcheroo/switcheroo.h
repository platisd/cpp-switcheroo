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

template<typename Callable>
struct CallableWithoutArgs {
    static const bool value = std::is_invocable<Callable>::value;
};
} // namespace detail

template<typename Variant,
         typename Matchers,
         typename MatcherArgIndexesT,
         typename FallbackMatcher,
         bool fallbackProvided = false>
class MatcherBuilder
{
public:
    MatcherBuilder(Variant variant, // TODO: Avoid copy?
                   Matchers matchers,
                   MatcherArgIndexesT matcherArgIndexes,
                   FallbackMatcher fallbackMatcher)
        : mVariant{std::move(variant)}
        , mMatchers{std::move(matchers)}
        , mMatcherArgIndexes{std::move(matcherArgIndexes)}
        , mFallbackMatcher{fallbackMatcher}
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
            "Type already matched, cannot match again");

        const auto newMatcherArgIndexes = std::tuple_cat(
            mMatcherArgIndexes,
            std::tuple<MatcherArgIndexType>{MatcherArgIndexType{}});
        using NewMatcherArgIndexesType
            = std::decay_t<decltype(newMatcherArgIndexes)>;

        return MatcherBuilder<Variant,
                              NewMatchersType,
                              NewMatcherArgIndexesType,
                              FallbackMatcher>{
            mVariant, newMatchers, newMatcherArgIndexes, mFallbackMatcher};
    }

    template<typename NewFallbackMatcher>
    [[nodiscard]] auto otherwise(NewFallbackMatcher&& fallbackMatcher) const
    {
        static_assert(detail::CallableWithoutArgs<NewFallbackMatcher>::value);
        return MatcherBuilder<Variant,
                              Matchers,
                              MatcherArgIndexesT,
                              NewFallbackMatcher,
                              true /* fallbackProvided */>{
            mVariant,
            mMatchers,
            mMatcherArgIndexes,
            std::forward<NewFallbackMatcher>(fallbackMatcher)};
    }

    [[nodiscard]] auto run() const
    {
        // Assert that the number of variant types is the same as the number of
        // matcher arguments
        static_assert(
            fallbackProvided
                != (std::tuple_size_v<MatcherArgIndexesT>
                    == std::variant_size_v<Variant>),
            "You need to match all types of the variant or provide a fallback "
            "matcher with `otherwise`. Also, you may not match all types of "
            "variant and provide a fallback matcher at the same time");

        return std::visit(
            [this](auto&& arg) {
                using ArgT          = std::decay_t<decltype(arg)>;
                const auto argIndex = detail::IndexOf<ArgT, Variant>::value;
                using ArgIndexType
                    = std::integral_constant<std::size_t, argIndex>;
                if constexpr (detail::TypeIn<ArgIndexType,
                                             MatcherArgIndexesT>::value) {
                    const auto index
                        = detail::IndexOf<ArgIndexType,
                                          MatcherArgIndexesT>::value;
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
    MatcherArgIndexesT mMatcherArgIndexes;
    FallbackMatcher mFallbackMatcher;
};

template<typename Variant>
auto Match(Variant variant)
{
    return MatcherBuilder{variant, std::tuple<>(), std::tuple<>(), []() {}};
}
} // namespace switcheroo

#endif // SWITCHEROO_H
