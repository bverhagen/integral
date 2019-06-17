# integral

`integral` is a C++ library for binding C++ code with Lua (Lua 5.1, Lua 5.2, Lua 5.3 or LuaJIT).

![Lua logo](http://www.lua.org/images/powered-by-lua.gif)

# Index

* [Features](#features)
* [Error safety](#error-safety)
* [Requirements](#requirements)
* [Tested environments](#tested-environments)
* [Build](#build)
* [Usage](#usage)
  * [Create Lua state](#create-lua-state)
  * [Use existing Lua state](#use-existing-lua-state)
  * [Get and set value](#get-and-set-value)
  * [Register function](#register-function)
  * [Register function with default arguments](#register-function-with-default-arguments)
  * [Register class](#register-class)
  * [Get object](#get-object)
  * [Register inheritance](#register-inheritance)
  * [Register table](#register-table)
  * [Use polymorphism](#use-polymorphism)
  * [Call function in Lua state](#call-function-in-lua-state)
  * [Register lua function argument](#register-lua-function-argument)
  * [Table conversion](#table-conversion)
* [Automatic conversion](#automatic-conversion)
* [integral reserved names in Lua](#integral-reserved-names-in-lua)
* [Source](#source)
* [Author](#author)
* [License](#license)


# Features

* no macros;
* no dependencies; and
* thread safety (as per Lua state): `integral` binds to different Lua states independently.


# Error safety

* `integral` will not crash the Lua state;
* stack unwinding is done before the Lua error handling gets in action;
* thrown exceptions in exported functions are converted to Lua errors;
* wrong parameter types in exported functions turn into Lua errors;
* wrong number of parameters in exported functions turn into Lua errors;
* functions returning pointers (except const char * which is considered a string) and references are regarded as unsafe, therefore cannot be exported. Trying to register these functions will cause compilation error; and
* invalid default arguments definition causes compilation error.


# Requirements

* C++17 compiler; and
* Lua 5.1 or later.


# Tested environments

* gcc version 8.3.0 (Ubuntu 8.3.0-6ubuntu1) on Ubuntu Linux; and
* Apple LLVM version 10.0.0 (clang-1000.11.45.5) on MacOS.


# Build

Grab all the source files (*.hpp and *.cpp) in `src` directory and build (there is no preprocessor configuration for the library).

Alternatively, build and install the library with:

    $ make
    $ make install


# Usage

Include the library header `integral/integral.hpp` (`namespace integral`) and link to `libintegral.a` or `libintegral.so` if the library was built separately.

Check [samples/abstraction](samples/abstraction) directory for examples.

## Create Lua state

```cpp
#include <integral/integral.hpp>

int main(int argc, char * argv[]) {
    integral::State luaState;
    luaState.openLibs();
    luaState.doFile("path/to/script.lua"); // executes lua script
    luaState.doString("print('hello!')"); // prints "hello!"
    return 0;
}
```

See [example](samples/abstraction/state/state.cpp).

## Use existing Lua state

```cpp
    lua_State *luaState = luaL_newstate();
    // luaState ownership is NOT transfered to luaStateView
    integral::StateView luaStateView(luaState); 
    luaStateView.openLibs();
    luaStateView.doString("print('hello!')"); // prints "hello!"
    lua_close(luaState);
```

See [example](samples/abstraction/state/state.cpp).

## Get and set value

```cpp
    luaState.doString("a = 42");
    int a = luaState["a"]; // "a" is set to 42
    luaState["b"] = "forty two";
    luaState.doString("print(b)"); // prints "forty two"

    luaState.doString("t = {'x', {pi = 3.14}}");
    std::cout << luaState["t"][2]["pi"].get<double>() << '\n'; // prints "3.14"
    luaState["t"]["key"] = "value";
    luaState.doString("print(t.key)"); // prints "value"
```

See [example](samples/abstraction/reference/reference.cpp).

## Register function

```cpp
double getSum(double x, double y) {
    return x + y;
}

double luaGetSum(lua_State *luaState) {
    integral::pushCopy(luaState, integral::get<double>(luaState, 1) + integral::get<double>(luaState, 2));
    return 1;
}

// ...

    luaState["getSum"].setFunction(getSum);
    luaState.doString("print(getSum(1, -.2))"); // prints "0.8"

    luaState["luaGetSum"].setLuaFunction(luaGetSum);
    luaState.doString("print(luaGetSum(1, -.2))"); // prints "0.8"

    // lambdas can be bound likewise
    luaState["printHello"].setFunction([]{
        std::puts("hello!");
    });
    luaState.doString("printHello()"); // prints "hello!"
```

See [example](samples/abstraction/function/function.cpp).

## Register function with default arguments

```cpp
    luaState["printArguments"].setFunction([](const std::string &string, int integer) {
        std::cout << string << ", " << integer << '\n';
    }, integral::DefaultArgument<std::string, 1>("default string"), integral::DefaultArgument<int, 2>(-1));
    luaState.doString("printArguments(\"defined string\")\n" // prints "defined string, -1"
                      "printArguments(nil, 42)\n" // prints "default string, 42"
                      "printArguments()"); // prints "default string, -1"
```
See [example](samples/abstraction/default_argument/default_argument.cpp).

## Register class

```cpp
class Object {
public:
    std::string name_;

    Object(const std::string &name) : name_(name) {}

    std::string getHello() const {
        return std::string("Hello ") + name_ + '!';
    }
};

// ...

    luaState["Object"] = integral::ClassMetatable<Object>()
                             .setConstructor<Object(const std::string &)>("new")
                             .setCopyGetter("getName", &Object::name_)
                             .setSetter("setName", &Object::name_)
                             .setFunction("getHello", &Object::getHello)
                             .setFunction("getBye", [](const Object &object) {
                                return std::string("Bye ") + object.name_ + '!';
                             })
                             .setLuaFunction("appendName", [](lua_State *lambdaLuaState) {
                                // objects (except std::vector, std::array, std::unordered_map, std::tuple and std::string) are gotten by reference
                                integral::get<Object>(lambdaLuaState, 1).name_ += integral::get<const char *>(lambdaLuaState, 2);
                                return 1;
                             });
```

See [example](samples/abstraction/class/class.cpp).

## Get object

Objects (except std::vector, std::array, std::unordered_map, std::tuple and std::string) are gotten by reference.

```cpp
    luaState.doString("object = Object.new('foo')\n"
                      "print(object:getName())"); // prints "foo"
    // objects (except std::vector, std::array, std::unordered_map, std::tuple and std::string) are gotten by reference
    luaState["object"].get<Object>().name_ = "foobar";
    luaState.doString("print(object:getName())"); // prints "foobar"
```

See [example](samples/abstraction/class/class.cpp).

## Register inheritance

```cpp
class BaseOfBase1 {
public:
    void baseOfBase1Method() const {
        std::puts("baseOfBase1Method");
    }
};

class Base1 : public BaseOfBase1 {
public:
    void base1Method() const {
        std::puts("base1Method");
    }
};

class Base2 {
public:
    void base2Method() const {
        std::puts("base2Method");
    }
};

class Derived : public Base1, public Base2 {
public:
    void derivedMethod() const {
        std::puts("derivedMethod");
    }
};

// ...

    integral::State luaState;
    luaState["BaseOfBase1"] = integral::ClassMetatable<BaseOfBase1>()
                              .setConstructor<BaseOfBase1()>("new")
                              .setFunction("baseOfBase1Method", &BaseOfBase1::baseOfBase1Method);
    luaState["Base1"] = integral::ClassMetatable<Base1>()
                        .setConstructor<Base1()>("new")
                        .setFunction("base1Method", &Base1::base1Method)
                        .setBaseClass<BaseOfBase1>();
    luaState["Base2"] = integral::ClassMetatable<Base2>()
                        .setConstructor<Base2()>("new")
                        .setFunction("base2Method", &Base2::base2Method);
    luaState["Derived"] = integral::ClassMetatable<Derived>()
                        .setConstructor<Derived()>("new")
                        .setFunction("derivedMethod", &Derived::derivedMethod)
                        .setBaseClass<Base1>()
                        .setBaseClass<Base2>();
    luaState.doString("derived = Derived.new()\n"
                      "derived:base1Method()\n" // prints "base1Method"
                      "derived:base2Method()\n" // prints "base2Method"
                      "derived:baseOfBase1Method()\n" // prints "baseOfBase1Method"
                      "derived:derivedMethod()"); // prints "derivedMethod"
```

See [example](samples/abstraction/inheritance/inheritance.cpp).

## Register table

```cpp
    luaState["group"] = integral::Table()
                            .set("constant", integral::Table()
                                .set("pi", 3.14))
                            .setFunction("printHello", []{
                                std::puts("Hello!");
                            })
                            .set("Object", integral::ClassMetatable<Object>()
                                .setConstructor<Object(const std::string &)>("new")
                                .setFunction("getHello", &Object::getHello));
    luaState.doString("print(group.constant.pi)\n" // prints "3.14"
                      "group.printHello()\n" // prints "Hello!"
                      "print(group.Object.new('object'):getHello())"); // prints "Hello object!"
```

See [example](samples/abstraction/table/table.cpp).

## Use polymorphism

Objects are automatically converted to base classes types regardless of inheritance definition with `integral`.

```cpp
class Base {};

class Derived : public Base {};

void callBase(const Base &) {
    std::puts("Base");
}

// ...
    luaState["Derived"] = integral::ClassMetatable<Derived>().setConstructor<Derived()>("new");
    luaState["callBase"].setFunction(callBase);
    luaState.doString("derived = Derived.new()\n"
                      "callBase(derived)"); // prints "Base"
```

See [example](samples/abstraction/polymorphism/polymorphism.cpp).

## Call function in Lua State

```cpp
    luaState.doString("function getSum(x, y) return x + y end");
    int x = luaState["getSum"].call<int>(2, 3); // 'x' is set to 5
```

See [example](samples/abstraction/function_call/function_call.cpp).

## Register lua function argument

```cpp
    luaState["getResult"].setFunction([](int x, int y, const integral::LuaFunctionArgument &function) {
        return function.call<int>(x, y);
    });
    luaState.doString("print(getResult(-1, 1, math.min))"); // prints "-1"
```

See [example](samples/abstraction/lua_function_argument/lua_function_argument.cpp).

## Table conversion

Lua tables are automatically converted to/from std::vector, std::array, std::unordered_map and std::tuple.

```cpp
    // std::vector
    luaState["intVector"] = std::vector<int>{1, 2, 3};
    luaState.doString("print(intVector[1] .. ' ' .. intVector[2] .. ' ' ..  intVector[3])"); // prints "1 2 3"

    // std::array
    luaState.doString("arrayOfVectors = {{'one', 'two'}, {'three'}}");
    std::array<std::vector<std::string>, 2> arrayOfVectors = luaState["arrayOfVectors"];
    std::cout << arrayOfVectors.at(0).at(0) << ' ' << arrayOfVectors.at(0).at(1) << ' ' << arrayOfVectors.at(1).at(0) << '\n'; // prints "one two three"

    // std::unordered_map and std::tuple
    luaState["mapOfTuples"] = std::unordered_map<std::string, std::tuple<int, double>>{{"one", {-1, -1.1}}, {"two", {1, 4.2}}};
    luaState.doString("print(mapOfTuples.one[1] .. ' ' .. mapOfTuples.one[2] .. ' ' .. mapOfTuples.two[1] .. ' ' .. mapOfTuples.two[2])"); // prints "-1 -1.1 1 4.2"
```

See [example](samples/abstraction/table_conversion/table_conversion.cpp)


# Automatic conversion

`integral` does the following conversions:

| C++                                                              | Lua                                            |
| ---------------------------------------------------------------- | ---------------------------------------------- |
| integral types (`std::is_integral`)                              | number [integer subtype in Lua version >= 5.3] |
| floating point types (`std::is_floating_point`)                  | number [float subtype in Lua version >= 5.3]   |
| `bool`                                                           | boolean                                        |
| `std::string`, `const char *`                                    | string                                         |
| `std::vector`, `std::array`, `std::unordered_map`, `std::tuple`  | table                                          |
| from: `integral::LuaFunctionWrapper`, `integral::FunctionWrapper`| to: function                                   |
| to: `integral::LuaFunctionArgument`                              | from: function                                 |
| other class types                                                | userdata                                       |


# integral reserved names in Lua

`integral` uses the following names in Lua registry:

* `integral_LuaFunctionWrapperMetatableName`;
* `integral_TypeIndexMetatableName`;
* `integral_TypeManagerRegistryKey`; and
* `integral_InheritanceIndexMetamethodKey`.

The library also uses the following field names in its generated class metatables:

* `__index`;
* `__gc`;
* `integral_TypeFunctionsKey`;
* `integral_TypeIndexKey`;
* `integral_InheritanceKey`;
* `integral_UserDataWrapperBaseTableKey`;
* `integral_UnderlyingTypeFunctionKey`; and
* `integral_InheritanceSearchTagKey`.


# Source

`integral`'s Git repository is available on GitHub, which can be browsed at:

    http://github.com/aphenriques/integral

and cloned with:

    git clone git://github.com/aphenriques/integral.git


# Author

`integral` was made by André Pereira Henriques [aphenriques (at) outlook (dot) com].


# License

Copyright (C) 2013, 2014, 2015, 2016, 2017, 2019  André Pereira Henriques.

integral is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

integral is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

See file `COPYING` included with this distribution or check <http://www.gnu.org/licenses/> for license information.

![gplv3 logo](http://www.gnu.org/graphics/gplv3-127x51.png)
