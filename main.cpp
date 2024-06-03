#include <iostream>
#include <string>
#include <variant>

#include "switcheroo/switcheroo.h"

struct Red {
    std::string red{"red"};
};

struct Green {
    std::string green{"green"};
};

struct Blue {
    std::string blue{"blue"};
};

int main()
{
    using namespace std::string_literals;
    using namespace switcheroo;

    const std::variant<std::monostate, Red, Green, Blue> color{Green{}};

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

    return 0;
}
