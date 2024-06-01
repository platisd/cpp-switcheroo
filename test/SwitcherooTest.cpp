#include <functional>
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
        = Match(color)
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

    auto result = Match(color)
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
        = Match(color)
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

    auto result = Match(color)
                      .when<Red>([](const auto&) { return 0; })
                      .otherwise([]() { return -1; })
                      .run();

    EXPECT_EQ(result, -1);
}

TEST_F(SwitcherooTest,
       switcheroo_whenOtherwiseProvided_WillInvokeCorrectCallback)
{
    std::variant<std::monostate, Red, Green, Blue> color{Green{}};

    auto result = Match(color)
                      .when<Green>([](const auto&) { return 0; })
                      .otherwise([]() { return -1; })
                      .run();

    EXPECT_EQ(result, 0);
}

TEST_F(SwitcherooTest,
       switcheroo_whenCasesProvidedInDifferentOrder_WillInvokeCorrectCallback)
{
    std::variant<Red, Green, Blue> color{Green{}};

    auto result = Match(color)
                      .when<Blue>([](const auto&) { return 2; })
                      .when<Green>([](const auto&) { return 1; })
                      .when<Red>([](const auto&) { return 0; })
                      .run();

    EXPECT_EQ(result, 1);
}

// typeIn
static_assert(typeIn<int, std::tuple<int, double, float>>::value);
static_assert(!typeIn<int, std::tuple<double, float>>::value);
static_assert(typeIn<int, std::variant<int, double, float>>::value);
static_assert(!typeIn<int, std::variant<double, float>>::value);
static_assert(
    typeIn<std::integral_constant<long unsigned int, 0>,
           std::tuple<std::integral_constant<long unsigned int, 0>,
                      std::integral_constant<long unsigned int, 1>>>::value);
// IndexOf
static_assert(IndexOf<int, std::tuple<int, double, float>>::value == 0);
static_assert(IndexOf<double, std::tuple<int, double, float>>::value == 1);
static_assert(IndexOf<float, std::variant<int, double, float>>::value == 2);

// CallableWithoutArgs
static_assert(CallableWithoutArgs<std::function<void()>>::value);
static_assert(!CallableWithoutArgs<std::function<void(int)>>::value);
