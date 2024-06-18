# cpp-switcheroo

A compile-time "switch-like" statement for C++17 and later. 🪄

## What is this?

`cpp-switcheroo` is a header-only library that provides a compile-time "switch-like" statement for C++17 and later.
It is a safer alternative to the traditional `switch` statement, and a (IMHO) more readable alternative to
visiting `std::variant` types with the "Overload" pattern.
While the library introduces additional syntax compared to
the ["Overload" pattern](#stdvisit-a-stdvariant-with-the-overload-pattern),
I personally find the added bits enhance readability and make the code less "exotic".

Let's see the differences between the three approaches:

### Classic `switch` statement

```cpp
#include <iostream>
#include <string>

enum class Color { Red, Green, Blue };

int main()
{
    const Color color{Color::Green};

    std::string colorName{};
    switch (color) {
    case Color::Red:
        colorName = "Red";
        break;
    case Color::Green:
        colorName = "Green";
        break;
    case Color::Blue:
        colorName = "Blue";
        break;
    default:
        colorName = "Unknown";
    }
    std::cout << colorName << std::endl;

    return 0;
}
```

**Pros:**

- Easy to understand and familiar
- Very efficient
- No extra code required
- Works with any C++ standard

**Cons:**

- Can only be used with integral types
- Can forget a `case` or `break` statement
- No type safety, (e.g. may static cast an integer to the enum type)

### `std::visit` a `std::variant` with the "overload" pattern

Read more about the pattern and how it
works [here](https://www.modernescpp.com/index.php/visiting-a-std-variant-with-the-overload-pattern/).

```cpp
#include <iostream>
#include <variant>

struct Red {};
struct Green {};
struct Blue {};
using Color = std::variant<Red, Green, Blue>;

template<typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

// Not needed in C++20
template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

int main()
{
    const Color color{Green{}};

    const auto colorName = std::visit(Overload{[](Red) { return "Red"; },
                                               [](Green) { return "Green"; },
                                               [](Blue) { return "Blue"; }},
                                      color);
    std::cout << colorName << std::endl;

    const auto anotherColorName = std::visit(Overload{[](Red) { return "Red"; },
                                                      [](auto) { return "not red"; }},
                                             color);
    std::cout << anotherColorName << std::endl;

    return 0;
}
```

**Pros:**

- Can be used with many types
    - Allows access to the value of the variant
- Can't forget a case
    - Enforced by the compiler
- Type-safe
- Relatively efficient

**Cons:**

- Needs C++17 or later
- Requires some boilerplate code
    - Very little boilerplate in C++20
- Not as easy to understand as a `switch` statement
- Some developers may not be familiar with the "overload" pattern
    - Exotic syntax
    - Not obvious what's happening
- Allows a default case even if all cases are covered

### `cpp-switcheroo`

```cpp

#include <iostream>

#include "switcheroo/switcheroo.h"

struct Red {};
struct Green {};
struct Blue {};
using Color = std::variant<Red, Green, Blue>;

int main()
{
    using namespace switcheroo;

    const Color color{Green{}};

    const auto colorName = match(color)
                               .when<Red>([](auto) { return "red"; })
                               .when<Green>([](auto) { return "green"; })
                               .when<Blue>([](auto) { return "blue"; })
                               .run();
    std::cout << colorName << std::endl;

    const auto anotherColorName = match(color)
                                      .when<Red>([](auto) { return "red"; })
                                      .otherwise([](auto) { return "not red"; })
                                      .run();
    std::cout << anotherColorName << std::endl;

    const auto yetAnotherColorName = match(color)
                                         .when<Red, Green>([](auto) { return "red or green"; })
                                         .otherwise([](auto) { return "blue"; })
                                         .run();

    std::cout << yetAnotherColorName << std::endl;

    return 0;
}
```

**Pros:**

- Can be used with many types
    - Allows access to the value of the variant
    - Specify multiple types in a single `when` statement
- Can't forget a case
    - Enforced by the compiler
    - Can defer to a default case with `otherwise`
    - Does not allow a default case if all cases are covered
- Type-safe
- Easier (IMHO) to understand than `std::visit` and the "overload" pattern
    - Less "exotic" syntax
    - I would argue developers familiar with the "overload" visitor pattern,
      would have no issue understanding `cpp-switcheroo` and so would developers who are not familiar with the pattern
      at all

**Cons:**

- Needs C++17 or later
- Requires the inclusion of the `cpp-switcheroo` library (header-only)
- The least efficient of the three (but unlikely to be a bottleneck)

### Comparison

| Feature                        | `switch` | `Overload` | `cpp-switcheroo` |
| ------------------------------ | -------- | ---------- | ---------------- |
| Use with many types            | ❌        | ✅          | ✅                |
| Combine multiple cases         | ✅        | ❌          | ✅                |
| Inhibit forgetting a case      | ❌        | ✅          | ✅                |
| Avoid unnecessary default case | ❌        | ❌          | ✅                |
| Easy to understand             | 🥇        | 🥉          | 🥈 (IMHO)         |
| Works with any C++ standard    | ✅        | ❌          | ❌                |
| Type-safe                      | ❌        | ✅          | ✅                |
| Require boilerplate code       | 🥇        | 🥈          | 🥉                |
| Efficiency                     | 🥇        | 🥈          | 🥉                |

## How to use

It's a single-header library, so copy the `switcheroo` directory or [switcheroo/switcheroo.h](switcheroo/switcheroo.h)
to your project and `include` it in your source code.
In [main.cpp](main.cpp) you can see an example of how to use `cpp-switcheroo`.
