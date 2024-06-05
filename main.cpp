#include <iostream>
#include <string>
#include <variant>

#include "switcheroo/switcheroo.h"

// Enum types as classes
struct Red {
    std::string red{"red"};
};

struct Green {
    std::string green{"green"};
};

struct Blue {
    std::string blue{"blue"};
};

// Enum types wrapped in a template class

enum class MonthType {
    January,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};

template<MonthType T>
struct Month {
    static constexpr MonthType value = T;
};

int main()
{
    using namespace std::string_literals;
    using namespace switcheroo;

    // Enum types as classes example
    using Color = std::variant<std::monostate, Red, Green, Blue>;
    const Color color{Green{}};

    const auto result
        = match(color)
              .when<Red>([](const Red& r) { return r.red; })
              .when<Green>([](const auto& g) { return g.green; })
              .when<Blue>([](auto) { return "blue"s; })
              .when<std::monostate>([](auto) { return "no color"s; })
              .run();

    std::cout << "Result: " << result << '\n'; // Result: green

    const auto result2 = match(color)
                             .when<Red>([](const auto&) { return 0; })
                             .otherwise([](auto) { return -1; })
                             .run();
    std::cout << "Result2: " << result2 << '\n'; // Result2: -1

    // Enum types wrapped in a template class example
    using MonthT = std::variant<Month<MonthType::January>,
                                Month<MonthType::February>,
                                Month<MonthType::March>,
                                Month<MonthType::April>,
                                Month<MonthType::May>,
                                Month<MonthType::June>,
                                Month<MonthType::July>,
                                Month<MonthType::August>,
                                Month<MonthType::September>,
                                Month<MonthType::October>,
                                Month<MonthType::November>,
                                Month<MonthType::December>>;

    const MonthT month{Month<MonthType::February>{}};

    const auto goodWeather
        = match(month)
              .when<Month<MonthType::June>>([](auto) { return true; })
              .when<Month<MonthType::July>>([](auto) { return true; })
              .when<Month<MonthType::August>>([](auto) { return true; })
              .otherwise([](auto) { return false; })
              .run();

    std::cout << "Good weather: " << std::boolalpha << goodWeather
              << '\n'; // Good weather: false

    return 0;
}
