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
enum class Month {
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

template<Month T>
struct Wrap {
    static constexpr Month value = T;
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

    // Lambdas without arguments are also allowed
    const auto result3
        = match(color)
              .when<Red, Green>([] { return 42; }) // Skip parentheses
              .when<Blue>([](auto&&) { return 1; })
              .otherwise([]() { return 2; }) // OK with empty parentheses
              .run();
    std::cout << "Result3: " << result3 << '\n'; // Result3: 42

    // Enum types wrapped in a template class example
    using MonthT = std::variant<Wrap<Month::January>,
                                Wrap<Month::February>,
                                Wrap<Month::March>,
                                Wrap<Month::April>,
                                Wrap<Month::May>,
                                Wrap<Month::June>,
                                Wrap<Month::July>,
                                Wrap<Month::August>,
                                Wrap<Month::September>,
                                Wrap<Month::October>,
                                Wrap<Month::November>,
                                Wrap<Month::December>>;

    const MonthT month{Wrap<Month::February>{}};

    const auto goodWeather
        = match(month)
              .when<Wrap<Month::June>>([](auto) { return true; })
              .when<Wrap<Month::July>>([](auto) { return true; })
              .when<Wrap<Month::August>>([](auto) { return true; })
              .otherwise([](auto) { return false; })
              .run();

    std::cout << "Good weather: " << std::boolalpha << goodWeather
              << '\n'; // Good weather: false

    // Match multiple "values"
    const auto badWeather
        = match(month)
              .when<Wrap<Month::January>,
                    Wrap<Month::February>,
                    Wrap<Month::March>,
                    Wrap<Month::April>>([](auto) { return true; })
              .otherwise([](auto) { return false; })
              .run();

    std::cout << "Bad weather: " << std::boolalpha << badWeather
              << '\n'; // Bad weather: true

    return 0;
}
