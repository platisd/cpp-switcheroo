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
    Color color{Color::Green};
    using namespace std::string_literals;

    std::string colorName{};
    switch (color) {
    case Color::Red:
        colorName = "Red"s;
        break;
    case Color::Green:
        colorName = "Green"s;
        break;
    case Color::Blue:
        colorName = "Blue"s;
        break;
    default:
        colorName = "Unknown"s;
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
#include <string>
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
    using namespace std::string_literals;

    const auto colorName = std::visit(Overload{[](Red) { return "Red"s; },
                                               [](Green) { return "Green"s; },
                                               [](Blue) { return "Blue"s; }},
                                      color);
    std::cout << colorName << std::endl;

    const auto anotherColorName = std::visit(Overload{[](Red) { return "Red"s; },
                                                      [](auto) { return "not red"s; }},
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
#include <string>

#include "switcheroo/switcheroo.h"

struct Red {};
struct Green {};
struct Blue {};
using Color = std::variant<Red, Green, Blue>;

int main()
{
    using namespace std::string_literals;
    using namespace switcheroo;

    const Color color{Green{}};

    const auto colorName = match(color)
                               .when<Red>([](auto) { return "red"s; })
                               .when<Green>([](auto) { return "green"s; })
                               .when<Blue>([](auto) { return "blue"s; })
                               .run();
    std::cout << colorName << std::endl;

    const auto anotherColorName = match(color)
                                      .when<Red>([](auto) { return "red"s; })
                                      .otherwise([](auto) { return "not red"s; })
                                      .run();
    std::cout << anotherColorName << std::endl;

    return 0;
}
```

**Pros:**

- Can be used with many types
    - Allows access to the value of the variant
- Can't forget a case
    - Enforced by the compiler
    - Can defer to a default case with `otherwise`
    - Does not allow a default case if all cases are covered
- Type-safe
- Easier (IMHO) to understand than `std::visit` and the "overload" pattern

**Cons:**

- Needs C++17 or later
- Requires the inclusion of an external library (header-only)
- The least efficient of the three (but unlikely to be a bottleneck)
- (IMHO) Easier to understand than `std::visit` and the "overload" pattern
    - Less "exotic" syntax
    - I would argue developers familiar with the "overload" visitor pattern,
      would have no issue understanding `cpp-switcheroo` and so would developers who are not familiar with the pattern
      at all

### Comparison

| Feature                        | `switch` | `Overload` | `cpp-switcheroo` |
|--------------------------------|----------|------------|------------------|
| Use with many types            | ❌        | ✅          | ✅                |
| Inhibit forgetting a case      | ❌        | ✅          | ✅                |
| Avoid unnecessary default case | ❌        | ❌          | ✅                |
| Easy to understand             | 🥇       | 🥉         | 🥈 (IMHO)        |
| Works with any C++ standard    | ✅        | ❌          | ❌                |
| Type-safe                      | ❌        | ✅          | ✅                |
| Require boilerplate code       | 🥇       | 🥈         | 🥉               |
| Efficiency                     | 🥇       | 🥈         | 🥉               |

## How to use

It's a single-header library, so copy the `switcheroo` directory or `switcheroo/switcheroo.h`
to your project and `include` it in your source code.
In [main.cpp](main.cpp) you can see an example of how to use `cpp-switcheroo`.
