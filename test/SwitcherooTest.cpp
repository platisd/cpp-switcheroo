#include <string>
#include <variant>

#include "switcheroo/switcheroo.h"
#include "gtest/gtest.h"

using namespace switcheroo;
using namespace std::string_literals;

struct Red {
    int red{0};
};

struct Green {
    int green{1};
};

struct Blue {
    int blue{2};
};

struct SwitcherooTest : public ::testing::Test {
};

TEST_F(SwitcherooTest, switcheroo_whenCalled_WillInvokeCorrectCallback)
{
    std::variant<std::monostate, Red, Green, Blue> color{Green{}};

    auto result
        = match(color)
              .when<Red>([](const auto&) { return "red"s; })
              .when<Green>([](const auto&) { return "green"s; })
              .when<Blue>([](const auto&) { return "blue"s; })
              .when<std::monostate>([](const auto&) { return "no color"s; })
              .run();

    EXPECT_EQ(result, "green"s);
}

TEST_F(SwitcherooTest,
       switcheroo_whenUsingArgumentInCallback_WillPassCorrectArgument)
{
    std::variant<std::monostate, Red, Green, Blue> color{Blue{42}};

    auto result = match(color)
                      .when<Red>([](const auto& c) { return c.red; })
                      .when<Green>([](const auto& c) { return c.green; })
                      .when<Blue>([](const auto& c) { return c.blue; })
                      .when<std::monostate>([](const auto&) { return -1; })
                      .run();

    EXPECT_EQ(result, 42);
}

TEST_F(SwitcherooTest, switcheroo_whenMonoState_WillInvokeCorrectCallback)
{
    std::variant<std::monostate, Red, Green, Blue> color{};

    auto result
        = match(color)
              .when<Red>([](const auto&) { return "red"s; })
              .when<Green>([](const auto&) { return "green"s; })
              .when<Blue>([](const auto&) { return "blue"s; })
              .when<std::monostate>([](const auto&) { return "no color"s; })
              .run();

    EXPECT_EQ(result, "no color"s);
}

TEST_F(SwitcherooTest, switcheroo_whenNoMatch_WillInvokeOtherwiseCallback)
{
    std::variant<std::monostate, Red, Green, Blue> color{};

    auto result = match(color)
                      .when<Red>([](const auto&) { return 0; })
                      .otherwise([](auto) { return -1; })
                      .run();

    EXPECT_EQ(result, -1);
}

TEST_F(SwitcherooTest,
       switcheroo_whenOtherwiseProvided_WillInvokeCorrectCallback)
{
    std::variant<std::monostate, Red, Green, Blue> color{Green{}};

    auto result = match(color)
                      .when<Green>([](const auto&) { return 0; })
                      .otherwise([](auto) { return -1; })
                      .run();

    EXPECT_EQ(result, 0);
}

TEST_F(SwitcherooTest,
       switcheroo_whenCasesProvidedInDifferentOrder_WillInvokeCorrectCallback)
{
    std::variant<Red, Green, Blue> color{Green{}};

    auto result = match(color)
                      .when<Blue>([](const auto&) { return 2; })
                      .when<Green>([](const auto&) { return 1; })
                      .when<Red>([](const auto&) { return 0; })
                      .run();

    EXPECT_EQ(result, 1);
}

TEST_F(SwitcherooTest, switcheroo_whenNoReturnTypeInMatchers_WillStillWork)
{
    std::variant<std::monostate, Red, Green, Blue> color{Green{}};

    match(color)
        .when<Red>([](const auto&) { std::cout << "Red\n"; })
        .when<Green>([](const auto&) { std::cout << "Green\n"; })
        .when<Blue>([](const auto&) { std::cout << "Blue\n"; })
        .when<std::monostate>([](const auto&) { std::cout << "No color\n"; })
        .run();
}

TEST_F(SwitcherooTest,
       switcheroo_whenNoReturnTypeInMatchersAndOtherwise_WillStillWork)
{
    std::variant<std::monostate, Red, Green, Blue> color{Green{}};

    match(color)
        .when<Red>([](const auto&) { std::cout << "Red\n"; })
        .otherwise([](auto) { std::cout << "Otherwise\n"; })
        .run();
}

TEST_F(SwitcherooTest,
       switcheroo_whenMultipleTypesToMatch_WillInvokeCorrectMatcher)
{
    using Color = std::variant<std::monostate, Red, Green, Blue>;

    auto matcher = [&](Color c) {
        return match(c)
            .when<Green, Red>([](const auto&) { return 0; })
            .when<Blue>([](const auto&) { return 1; })
            .when<std::monostate>([](const auto&) { return 2; })
            .run();
    };

    EXPECT_EQ(0, matcher({Green{}}));
    EXPECT_EQ(0, matcher({Red{}}));
    EXPECT_EQ(1, matcher({Blue{}}));
    EXPECT_EQ(2, matcher({}));
}

TEST_F(SwitcherooTest, switcheroo_whenLambdaWithNoArguments_WillStillWork)
{
    using Color = std::variant<std::monostate, Red, Green, Blue>;

    auto matcher = [&](Color c) {
        return match(c)
            .when<Green, Red>([] { return 0; })
            .when<Blue>([]() { return 1; })
            .otherwise([](auto&&) { return 2; })
            .run();
    };

    EXPECT_EQ(0, matcher({Green{}}));
    EXPECT_EQ(0, matcher({Red{}}));
    EXPECT_EQ(1, matcher({Blue{}}));
    EXPECT_EQ(2, matcher({}));
}

TEST_F(SwitcherooTest, switcheroo_whenOtherwiseWithNoArguments_WillStillWork)
{
    using Color = std::variant<std::monostate, Red, Green, Blue>;

    auto matcher = [&](Color c) {
        return match(c)
            .when<Green, Red>([] { return 0; })
            .when<Blue>([]() { return 1; })
            .otherwise([] { return 2; })
            .run();
    };

    EXPECT_EQ(0, matcher({Green{}}));
    EXPECT_EQ(0, matcher({Red{}}));
    EXPECT_EQ(1, matcher({Blue{}}));
    EXPECT_EQ(2, matcher({}));
}

namespace switcheroo::detail
{
// TypeIn
static_assert(TypeIn<int, std::tuple<int, double, float>>::value);
static_assert(!TypeIn<int, std::tuple<double, float>>::value);
static_assert(TypeIn<int, std::variant<int, double, float>>::value);
static_assert(!TypeIn<int, std::variant<double, float>>::value);
static_assert(
    TypeIn<std::integral_constant<long unsigned int, 0>,
           std::tuple<std::integral_constant<long unsigned int, 0>,
                      std::integral_constant<long unsigned int, 1>>>::value);
// IndexOf
static_assert(IndexOf<int, std::tuple<int, double, float>>::value == 0);
static_assert(IndexOf<double, std::tuple<int, double, float>>::value == 1);
static_assert(IndexOf<float, std::variant<int, double, float>>::value == 2);

// MissingTypes
static_assert(
    std::is_same_v<MissingTypes<std::tuple<int, double, float>,
                                std::tuple<int, double, float, char>>::type,
                   std::tuple<char>>);
static_assert(std::is_same_v<MissingTypes<std::tuple<int, double, float>,
                                          std::tuple<int, double, float>>::type,
                             std::tuple<>>);

// ToTupleOfIntegralConstants
static_assert(
    std::is_same_v<
        ToTupleOfIntegralConstants<std::integer_sequence<int, 0, 1, 2>>::type,
        std::tuple<std::integral_constant<int, 0>,
                   std::integral_constant<int, 1>,
                   std::integral_constant<int, 2>>>);

// MissingTypes
static_assert(
    std::is_same_v<MissingTypes<std::tuple<int, double, float>,
                                std::tuple<int, double, float, char>>::type,
                   std::tuple<char>>);
static_assert(std::is_same_v<MissingTypes<std::tuple<int, double, float>,
                                          std::tuple<int, double, float>>::type,
                             std::tuple<>>);

// multipleInTuple
TEST(MultiplyInTupleTest, multiplyInTuple)
{
    auto result = multiplyInTuple(1, std::make_index_sequence<3>{});
    EXPECT_EQ(result, std::make_tuple(1, 1, 1));
}

} // namespace switcheroo::detail
