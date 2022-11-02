# ShnooTalk language guide

This doc goes over syntax and features of the ShnooTalk programming language.

🦄 Emoji in the document indicates notes about missing features and things that might change in the future.

## Comments

Comments in ShnooTalk start with `#` character.

```
# This is a comment
const a := 1  # This is another comment
```

## Literals

### Integer literals

Integer literals can be written in hex, binary or decimal.

```
1234        # decimal
0xABCD      # hex
0xabcd      # also hex
0b101101    # binary
```

### Float literals

Floating point literals are made up of numbers with `.` in the middle.

```
1.234
0.234
.234  # invalid float literal
```

### Character literals

Character literals store a single character and are inclosed within single quotes (`'`).

```
'c'
'a'
'\n'
```

### String literals

String literals are enclosed within double quotes (`"`). Internally string literals are represented as null terminated char arrays.

```
"Hello world"
"This is another string\n"
```

### Multiline string literals

Long string literals can be split up into multiple parts to improved readability. The parts or concatenated into one single string without any delimiter.

Below three statements are all equivalent

```
const a := "Hello world"

const b := "Hello" " world"

const c :=
    "Hello"
    " World"
```

### The `def` keyword

If a literal is used in multiple places, it can be aliased or given a name to improve readability. Multiline string literals cannot be used with `def`.

```
def PI 3.14
def HELLO_WORLD "Hello world"
def ZERO 0

println(PI)             # equivalent to println(3.14)
println(HELLO_WORLD)    # equivalent to println("Hello world")
println(ZERO)           # equivalent to println(ZERO)
```

### Builtin literals

The following literals are already pre-defined in ShnooTalk

| Name     | Value    | Type           |
| -------- | -------- | -------------- |
| false    | 0        | integer        |
| true     | 1        | integer        |
| NULL     | 0        | integer        |
| NAN      | nan      | floating point |
| INFINITY | infinity | floating point |

## Program entry point

All executable programs must have a function called `main`. When a program starts, this is the first function that will be called, and it must return a `int` primitive type.

```
fn main() -> int
{
    println("Hello world")
    return 0
}
```

## Types

### Primitive types

ShnooTalk has the following primitive types

| Name              | Description                                                                  |
| ----------------- | ---------------------------------------------------------------------------- |
| `byte`            | Signed 8-bit integer.                                                        |
| `ubyte` or `char` | Unsigned 8-bit integer.                                                      |
| `short`           | Signed 16-bit integer.                                                       |
| `ushort`          | Unsigned 16-bit integer.                                                     |
| `int`             | Signed 32-bit integer. This is the default integer type.                     |
| `uint`            | Unsigned 32-bit integer.                                                     |
| `long`            | Signed 64-bit integer.                                                       |
| `ulong`           | Unsigned 64-bit integer. This is used to represent sizes in ShnooTalk 🦄.    |
| `float`           | 32-bit floating point number.                                                |
| `double`          | 64-but floating point number. This is the default for floating point number. |
| `bool`            | Alias for `byte`                                                             |

🦄 This will change to a new primitive type called `usize` in the future.

### Arrays

Arrays store multiple values of the same type on the stack. They can be indexed and declared using the `[n]` syntax. Arrays are indexed starting from zero.

```
fn main() -> int
{
    const a: int[3] = [10, 20, 30]

    println(a[1])   # prints 20

    return 0
}
```

🦄 There is no runtime or compile time bounds checking performed when an array is indexed as of now, so arrays must be used with caution. This will change in the future.

🦄 Users are encouraged to use `List` from the standard library instead of arrays, which has bounds checking, but this allocates values on the heap.

### Pointers

Pointer types store memory addresses instead of a value, hence pointers can _reference_ or _point to_ another variable and any modification made to the pointer will also reflect on the variable. The `<-` operator is used to make or change what variable the pointer is pointing.

Pointers are declared using the `*` syntax.

```
fn main() -> int
{
    var a: int = 2
    var ptr: int* <- a

    ptr += 2        # This will modify a

    println(a)      # will print 4

    var b: int = 10

    ptr <- b        # ptr now points to b instead of a

    println(ptr)    # prints 10 which is the value of b

    return 0
}
```

🦄 Pointers and arrays are very unsafe in ShnooTalk and must be used minimally and only if required. Users are encouraged to use types from the standard library instead. Operations on pointers maybe only permitted inside `unsafe` blocks in the future.

### Array pointers

Array pointers are the same as pointer but for arrays and are indexable using the `[n]` syntax.

Array pointers are declared using the `[]` syntax without any literal between the brackets.

```
fn main() -> int
{
    var a: int[3] = [1, 2, 3]
    var b: int[] <- a

    b[1] = 200                  # this will modify a

    println(a[0], a[1], a[2])   # prints 1 200 3

    return 0
}
```

### Structs

Struct are user defined types. It lets the user define a new type composed of multiple different or same primitive types.

```
struct Player
{
    var id: int
    var x: double, y: double
}

fn main() -> int
{
    var a: Player

    a.id = 1
    a.x = 2.3
    a.y = 4.5

    println(a.id, a.x, a.y)

    return 0
}
```

🦄 There will be a better syntax to initialize all struct fields in the future.

🦄 The compiler does not pad struct for alignment yet. This feature might be added in the future. Currently struct types are packed without any padding.

### Enums

Enums lets the user define a integer type that can only have limited range of continues values. Enums are usually used to type categorical types like colors, error types etc.

```
enum TempRange {LOW, MEDIUM, HIGH}

fn main() -> int
{
    var a: TempRange = LOW

    println(a)  # prints 0

    a = HIGH

    println(a)  # prints 2

    a = 3       # compile error, this results in type error

    return 0
}
```

## Variables

### Global variables

Global variables are declared outside all functions, they are accessible to all the functions declared in the same file.

Global variables can only be initialized inside a function and cannot be `const`.

```
var a: int

fn foo()
{
    a += 1
}

fn bar()
{
    println(a)
}

fn main() -> int
{
    a = 10      # initialize global variable
    foo()
    bar()       # prints 11
    return 0
}
```

🦄 There might be syntax for declaring an init function for a file to initialize global variables that will be automatically called before the program starts in the future.

### Local variables

Local variables are declared inside a block enclosed with `{` and `}` and are only accessible inside the same block they are declared or withing a nested block. They cannot be accessed in the parent block.

```
fn main() -> int
{
    var a: int = 1

    if a == 1 {
        println(a)      # a is accessible inside nested blocks

        var b: int = 2
        println(b)      # b is accessible inside the same block it is declared in
    }

    println(b)          # compile error, b is not accessible in the parent block

    return 0
}
```

Local variables can be initialized either using `var` or the `const` keyword. Variables initialized using the `const` keyword cannot be modified later in the program and providing an initial value is compulsory.

```
fn main() -> int
{
    var a: int
    a = 1               # can be initialized later
    const b: int = 1

    a += 1
    println(a)          # prints 2

    b += 1              # compile error, b cannot be modified

    return 0
}
```

The type can be ignored by using `:=` operator instead of `=`. Using `const` and `:=` is preferred over `var` and `=` in ShnooTalk.

```
fn main() -> int
{
    const a := 1      # default type int is used

    println(a)

    return 0
}
```

For pointers, `:=` or `=` syntax cannot be used and `<-` operator must be used instead. Type must always be explicitly specified.

```
fn main() -> int
{
    var a := 1
    var ptr: int* <- a

    println(ptr)    # prints 1

    return 0
}
```

## Assignments

The `=` operator can be used to change the value of a variable.

```
fn main() -> int
{
    var a := 1
    println(a)
    a = 2           # change to 2
    println(a)      # prints 2
    return 0
}
```

For pointers, `=` operator modifies the variable the pointer is pointing or referencing. And `<-` operator can be use to change what variable the pointer is pointing [as seen here](#pointers).

There are also other assignment operators other than `=`. These are `*=`, `/=`, `%=`, `+=`, `-=`, `<<=`, `>>=`, `&=`, `^=`, `|=`.

```
fn main() -> int
{
    var a := 1
    println(a)
    a += 1          # equivalent to a = a + 1
    println(a)      # prints 2
    return 0
}
```

## Destructuring

The destructuring syntax allows unpacking arrays and struct into variables in assignment or declaration/initialization.

### Ordered destructuring

Unpack values from struct or array in order and assign it to a variables.

```
fn main() -> int
{
    var [a, b] := [1, 2]    # declare a and b and unpack this array into a and b

    println(a)              # prints 1
    println(b)              # prints 2

    .[a, b] = [3, 4]
    # The preceding dot is purely syntactic
    #   and is there to make grammar parsable
    #   the dot can be ignored in some cases but
    #   it is recommended to have it anyways

    println(a)              # prints 3
    println(b)              # prints 4

    return 0
}
```

It is also possible to unpack a struct into variables.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    p.x = 1
    p.y = 2

    const [a, b] := p

    println(a)  # prints 1
    println(b)  # prints 2

    return 0
}
```

It is also possible to initialize struct fields using destructuring.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    .[p.x, p.y] = [1, 2]   # unpack array elements into struct fields of p

    println(p.x, p.y)      # prints 1 2

    return 0
}
```

The unpacking and assignment always happens from left to right, so this CANNOT be used to swap value of variables, so below will not work.

```
fn main() -> int
{
    var [a, b] := [1, 2]

    .[a, b] = [b, a]

    print(a, b)     # print 2 2 and NOT 2 1

    return 0
}
```

### Named destructuring

When right hand side of destructuring assignment or initialization is a struct and left hand side contains only identifiers whose names are present as a field in the struct, ShnooTalk will perform a named destructuring. It will unpack the field into the variable with same name as the field.

ShnooTalk will try to perform named destructuring first and fallbacks on ordered destructuring if it is not possible.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    p.x = 1
    p.y = 2

    const [y, x] := p   # performs named structuring

    println(x)          # prints 1  (would have been 2 if ordered destructuring)
    println(y)          # prints 2  (would have been 1 if ordered destructuring)

    return 0
}
```

Unlike ordered destructuring, the number of elements on left and right don't have to match.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    p.x = 1
    p.y = 2

    const [y] := p   # performs named structuring

    println(y)       # prints 2

    return 0
}
```

## Expressions

You can use expressions in ShnooTalk to do computations and calculations.

```
fn main() -> int
{
    const a := 1 + 2
    println(a)      # prints 3

    return 0
}
```

### Binary operators

ShnooTalk has the following binary operators

| Symbol | Name                |
| ------ | ------------------- |
| \*     | Multiply            |
| /      | Divide              |
| %      | Remainder or modulo |
| +      | Add                 |
| -      | Subtract            |
| <<     | Left shift          |
| >>     | Right shift         |
| &      | Bitwise and         |
| ^      | Bitwise xor         |
| \|     | Bitwise or          |

### Unary operators

ShnooTalk has the following unary operators

| Symbol | Name        |
| ------ | ----------- |
| -      | Minus       |
| ~      | Bitwise not |

### Conditional expressions

You can use conditional expression in ShnooTalk to evaluate and check conditions. Conditional expression result to an integer that is either 0 or 1. The result of a conditional expression can be assigned to any integer type but it is recommended to use the `bool` type.

```
fn main() -> int
{
    const a := 4

    println(a < 3)              # prints 0
    println(a >= 4)             # prints 1
    println(a > 1 and a < 5)    # prints 1

    return 0
}
```

### Relational operators

ShnooTalk has the following relational operators

| Symbol | Name                     |
| ------ | ------------------------ |
| <      | Less than                |
| <=     | Less than or equal to    |
| >      | Greater than             |
| >=     | Greater than or equal to |
| ==     | Equal                    |
| !=     | Not equal                |

### Logical operators

ShnooTalk has the following logical operators

| Symbol       | Name |
| ------------ | ---- |
| `not` or `!` | Not  |
| `and`        | And  |
| `or`         | Or   |

### Type casting

ShnooTalk does not allow binary operator or relational operators between two different primitive types.

```
fn main() -> int
{
    const a := 2
    const b := 2.3

    println(a + b)    # compile error

    return 0
}
```

So you will have to explicitly tell ShnooTalk to cast types using the <code>`</code> type cast operator,

```
fn main() -> int
{
    const a := 2
    const b := 2.3

    println(double`a + b)     # cast a from int to double, prints 4.3

    return 0
}
```

### Pointer type casting

You can use the <code>\*\`</code> and <code>[]`</code> operator to bitcast pointers from one type to another. This operator is very unsafe as it overrides ShnooTalk's type checking. This operator is useful to interface with C functions.

```
fn main() -> int
{
    var a := 65

    const ptrA: int* <- a
    const ptrB: char* <- char*`ptrA     # bitcast int* to char*

    println(ptrB)                       # prints A which is 65 in ascii

    return 0
}
```

You can use <code>[]`</code> to bitcast array pointers.

```
extfn malloc(bytes: ulong) -> ubyte[]

fn main() -> int
{
    const a: int[] <- int[]`malloc(100)

    return 0
}
```

You can also cast integers into pointer

```
fn main() -> int
{
    const a: int* <- int*`NULL

    return 0
}
```

## Control statements

Control flow statements can change the flow of the program. Control flow statements in ShnooTalk are very similar to C but no parenthesis is required for the conditional statement.

### `if` statement

If statements have a compulsory starting `if` branch, then subsequent optional multiple `else if` branch and then final optional `else` branch.

```
fn main() -> int
{
    const a := 10

    if a > 5
    {
        println("A is greater than 5")
    }
    else if a > 3
    {
        println("A is greater than 3")
    }
    else
    {
        println("A is less than or equal to 2")
    }

    return 0
}
```

If there is only one statement inside the block, `{` and `}` can be ignored. This applies for all control statements.

```
fn main() -> int
{
    const a := 4

    if a > 2
        println("A is greater than 2")
    else
        println("A is less than or equal to 2")

    return 0
}
```

The entire statement could also be written in one line

```
fn main() -> int
{
    const a := 4

    if a > 2 println("A is greater than 2") else println("A is less than or equal to 2")

    return 0
}
```

If statements and other control statements can also accept an expression that results in any integer value instead of just 0 or 1, and anything not equal to zero will be treated as `true`

```
fn main() -> int
{
    if 2 println("Hello")   # prints hello

    return 0
}
```

### Infinite loop

Executes a set of statements continuously forever until a `break` is encountered.

```
fn main() -> int
{
    loop
    {
        println("Hello world")  # prints Hello world forever
    }

    return 0
}
```

### `while` loop

While loop is used to execute a set of statements repeatedly while a condition remains true.

```
fn main() -> int
{
    var a := 1

    while a <= 4
    {
        println(a)      # prints 1 2 3 4
        a += 1
    }

    return 0
}
```

### `do while` loop

A do while loop is very similar to the while loop, except the condition is checked AFTER executing the statements unlike the while loop where the condition is checked BEFORE executing the statements. Hence the statements are executed at least one in a do while loop.

```
fn main() -> int
{
    var a := 4

    do
    {
        a += 1
        println(a)  # prints 5, while loop would not print anything
    } while a < 4

    return 0
}
```

### `for` loop

A for loop is very similar to the while loop except it also lets you declare or assign a variable and provide an update statement.

```
fn main() -> int
{
    for var i := 1; i <= 10; i += 1
    {
        println(i)
    }

    return 0
}
```

This is equivalent to

```
fn main() -> int
{
    var i := 1

    while i <= 10
    {
        println(i)
        i += 1
    }

    return 0
}
```

### `break` and `continue`

The `break` keyword is used to exit or abort a loop prematurely.

```
fn main() -> int
{
    for var i := 1; i <= 10; i += 1
    {
        if i == 5 break
        println(i)      # prints upto 4 and not 10
    }

    return 0
}
```

The `continue` keyword can be used to skip a particular loop

```
fn main() -> int
{
    for var i := 1; i <= 10; i += 1
    {
        if i == 5 or i == 7 continue
        println(i)      # prints upto 10 but skips 5 and 7
    }

    return 0
}
```

## Functions

### Basic functions

Functions in ShnooTalk can be declared with the `fn` keyword

```
fn sayHello()
{
    println("Hello")
}

fn main() -> int
{
    sayHello()
    return 0
}
```

The `return` statement can be used to return values from the function. The `->` token is used to mark the return type. Functions can also accept parameters.

```
fn add(a: int, b: int) -> int
{
    return a + b
}

fn main() -> int
{
    const a := add(1, 2)

    println(a)

    return 0
}
```

By default functions cannot modify the parameters passed to them, if you would like to do so, the parameter has to marked mutable with the `mut` token. If a parameter is marked `mut`, `const` variables cannot be passed as an argument.

```
fn modifyInPlace(mut a: int)
{
    a += 1
}

fn main() -> int
{
    var a := 1      # has to be var, const won't work

    modifyInPlace(a)

    println(a)      # prints 2

    return 0
}
```

### Functions as constructors

Function can be used as constructors for `struct` type. The convention is the type's name is in _PascalCase_ and the constructor function name is the same name as the type but in _camelCase_

```
struct Point
{
    var x: int, y: int
}

fn point(x: int, y: int) -> Point
{
    var p: Point
    p.x = x
    p.y = y
    return p
}

fn main() -> int
{
    const p := point(1, 2)

    println(p.x, p.y)

    return 0
}

```

### Methods

Functions whose first parameter is a `struct` type, will also become a method associated with the struct. Methods are almost just syntax sugar for calling a normal function in ShnooTalk. The first parameter is always named `self`.

```
struct Point
{
    var x: int, y: int
}

fn point(x: int, y: int) -> Point
{
    var p: Point
    p.x = x
    p.y = y
    return p
}

fn printPoint(self: Point)
{
    println(self.x, self.y)
}

fn add(self: Point, other: Point) -> Point
{
    return point(self.x + other.x, self.y + other.y)
}

fn main() -> int
{
    const p := point(1, 2)
    const q := point(3, 4)

    # Both statements are equivalent
    const a := p.add(q)
    const b := add(p, q)

    a.printPoint()  # prints 4 6
    b.printPoint()  # prints 4 6 also

    return 0
}
```

### `extfn` functions

If you want to use function from C libraries, you will have to declare them using the `extfn` keyword

```
extfn exit(status: int)

fn main() -> int
{
    exit(1)
    return 0
}
```

### `externC` functions

If you want to use ShnooTalk functions in C or export it from WASM module compiled from ShnooTalk or use it in other languages with C interoperability, the function has to be marked with the `externC` keyword. This will tell ShnooTalk not to name mangle the function. Using this keyword can also lead to linker errors if they are declared more than once in different modules with the same name unlike normal functions.

```
externC fn add(a: int, b: int) -> int
{
    return a + b
}
```

## Print statements

You can use the `print` and `println` keywords to print to console or terminal. `println` will add newline (`\n`) character at the end.

```
fn main() -> int
{
    print("Hello world\n")
    println("Hello world")  # no need for \n

    return 0
}
```

You can pass multiple values to the print statement, each value will be printed separated by a space character.

```
fn main() -> int
{
    # prints 1 2 Hello world
    println(1, 2, "Hello", "world")

    return 0
}
```

If you don't want the space between each value, you can use `;` instead of `,`

```
fn main() -> int
{
    # prints 12 Helloworld
    println(1; 2, "Hello"; "world")

    return 0
}
```

## Builtin functions

### `sizeof` builtin

This is a special function that accepts a type as a parameter and prints the total size that type occupies in memory in bytes. This function is run at compile time.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    println(sizeof(int))        # prints 4
    println(sizeof(Point))      # prints 8

    return 0
}
```

### `addr` builtin

The `addr` function return the address that a pointer is holding. The function is very useful in checking if a pointer is null.

```
fn main() -> int
{
    var a := 1
    var ptr: int* <- a

    println(addr(ptr))  # print address of a

    if addr(ptr)
        println("ptr is not null")

    return 0
}
```

## Standard library

The guide has covered most of ShnooTalk's low level core syntax, but the language by itself does not have lot of features that a modern programing language would provide such as lists, dictionaries or maps, reading and writing files, generating random numbers etc.

So ShnooTalk comes with a standard library written in low level ShnooTalk syntax using C functions from [libc](https://en.wikipedia.org/wiki/C_standard_library).

### Returning multiple values

ShnooTalk by itself does not technically support returning multiple values, but it has destructuring. The standard library takes advantage of this and provides some types to make returning multiple values possible.

These types are `Pair`, `Triple` and `Quad`. You can think of them as `std::pair` from C++ or somewhat like tuples from rust (maybe not exactly tuples but you get the idea).

```
from "stdlib/Pair.shtk" use Pair, pair

fn foobar() -> Pair[int, double]
{
    return pair(1, 2.35)
}

fn main() -> int
{
    const [x, y] := foobar()

    println(x)              # prints 1
    println(y)              # prints 2.35

    const pair := foobar()

    println(pair.first)     # prints 1
    println(pair.second)    # prints 2.35

    return 0
}
```

The other two types Triple and Quad are used to return three and four values.

### Random

The random module contains helper functions to generate random numbers. Generated number be in range from `start` and upto but not equal to `end`.

-   `fn randomInt(start: int, end: int) -> int`
-   `fn randomLong(start: long, end: long) -> long`
-   `fn random(start: double, end: double) -> double`

### Error handling

#### Panic

When the error is unrecoverable and you want to abort the program, panic can be used.

```
from "stdlib/Panic.shtk" use panic

fn main() -> int
{
    panic("Abort the program!", 1)          # aborts the program with exit code 1

    println("This will not be printed")     # will not be printed

    return 0
}
```

#### Result

When the error is recoverable you don't want a function to abort the entire program, instead you want to indicate that an error has occurred so it can be handled by the caller, otherwise you would like to return the result.
For this ShnooTalk has the `Result[T, E]` type. `T` and `E` are generic parameters, where `T` is the type that will be returned on success and `E` is the type that will be returned when an error has occurred. This type is similar to the `Result<T, E>` enum from rust.

The error type `E` is usually an enum indicating the type of error.

Here is a simple example

```
from "stdlib/Result.shtk" use Result, ok, fail
from "stdlib/Random.shtk" use randomInt

enum LicenseError {
    TooYoung,
    TooOld
}

fn getDrivingLicense(age: int) -> Result[int, LicenseError]
{
    if age < 18
        return fail(TooYoung)

    if age > 80
        return fail(TooOld)

    const id := randomInt(0, 1000)
    return ok(id)
}

fn main() -> int
{
    const [id, err] := getDrivingLicense(32)   # Try different values

    if err
    {
        if err.get() == TooYoung
            println("Too young to get license")
        else if err.get() == TooOld
            println("Too old to get license")

        return 0
    }

    println("Got license with id", id)

    return 0
}
```

The standard library defines all the possible errors it can return in one common enum `StdlibError`, in `stdlib/ErrorTypes.shtk`.

The convention is that every library will have a one common enum defined in `ErrorTypes.shtk` with all the types of error that can happen and is used across the library.

#### Optional

The `Optional` type can be used used to represent a value that may not exist. It is similar to the `Option<T>` from rust or `std::optional` from C++ or `Optional[T]` from Python.

Here is a simple example

```
from "stdlib/Optional.shtk" use Optional, none, some
from "stdlib/Random.shtk" use randomInt

fn divide(numerator: float, denominator: float) -> Optional[float]
{
    if denominator == 0.0
        return none()

    return some(numerator/denominator)
}

fn main() -> int
{
    const [answer, err] := divide(5.0, 0.0)

    if err
        println("Cannot divide")
    else
        println(answer)

    return 0
}
```

#### Error

The `Error[E]` is a generic type used to indicate weather an error has occurred or not, but there is no output result. Example may include connecting to a database, deleting a file etc. The `Result[T, E]` used this type to enclose the error.

```
from "stdlib/Error.shtk" use Error, error, nil
from "stdlib/String.shtk" use String, string

enum DoorError {
    WrongUsername,
    WrongPassword
}

fn openDoor(username: String, password: String) -> Error[DoorError]
{
    if username != "Alice".string()
        return error(WrongUsername)

    if password != "1234".string()
        return error(WrongPassword)

    return nil()
}

fn main() -> int
{
    const err := openDoor("Alice", "1234")    # Try different values

    if err
        println(err.get())
    else
        println("Door open")

    return 0
}
```

#### Expect method

When the `expect` method is called on a _failed_ `Result` or an _none_ `Optional`, the program will panic and abort otherwise it will return the value.
This is very similar to the expect method from rust.
This can be useful for prototyping and testing code.

The below example will fail

```
from "stdlib/Optional.shtk" use Optional, none

fn main() -> int
{
    const a := none[int]().expect()

    println(a)

    return 0
}
```

But the below example will print '4'

```
from "stdlib/Optional.shtk" use Optional, some

fn main() -> int
{
    const a := some[int](4).expect()

    println(a)    # prints 4

    return 0
}
```

#### OR operator

The OR operator can be use on `Result` or an `Optional` type to provide default value if the no value is present.

```
from "stdlib/Optional.shtk" use Optional, some, none

fn main() -> int
{
    const a := none[int]() | 2
    println(a)                    # prints 2

    const b := some[int](4) | 2
    println(b)                    # prints 4

    return 0
}
```

The operator can also be used between two `Optional` or `Result` types

```
from "stdlib/Optional.shtk" use Optional, some, none

fn main() -> int
{
    const cOptional := none[int]() | some[int](2)

    const c := cOptional.expect()

    println(c)                    # prints 2

    return 0
}
```

#### Question mark operator

While propagating errors to the caller, your code might get cluttered with `if` statements

```
from "stdlib/Optional.shtk" use Optional, some, none

fn foo() -> Optional[int]
{
    const [a, err] := some[int](2)

    if err
        return none()

    return some(a + 2)
}

fn main() -> int
{
    const b := foo().expect()

    println(b)    # prints 4

    return 0
}
```

Instead you can use the `?` operator to make the code concise and readable. You can avoid a lot of `if err return x` statements.

```
from "stdlib/Optional.shtk" use Optional, some, none

fn foo() -> Optional[int]
{
    const a := some[int](2)?

    return some(a + 2)
}

fn main() -> int
{
    const b := foo().expect()

    println(b)    # prints 4

    return 0
}
```

#### Result, Optional and Error reference

Here are the list of operations you can perform on the `Result` and the `Optional` types

**Result**

-   `fn ok(item: T) -> Result`
-   `fn fail(error: E) -> Result`
-   `fn hasValue(self: Result) -> bool`
-   `fn isFail(self: Result) -> bool`
-   `fn expect(self: Result) -> T`
-   `?` operator

**Optional**

-   `fn some(item: T) -> Optional`
-   `fn none() -> Optional`
-   `fn hasValue(self: Optional) -> bool`
-   `fn isNone(self: Optional) -> bool`
-   `fn expect(self: Optional) -> T`
-   `?` operator

### String

Without the standard library strings are represented as null terminated char arrays.

```
fn main() -> int
{
    const message := "Hello world"
    println(message)

    return 0
}
```

Char arrays are not resizable and you cannot perform any string operations on them. Char arrays also don't have any bounds checking because they are [arrays](#arrays). So the standard library provides a String type.

```
from "stdlib/String.shtk" use string

fn main() -> int
{
    const message := "Hello world".string()
    println(message)

    return 0
}
```

If you have used C++ you can think of the two types as `std::string` and C strings or if you have used rust, `String::from` and string slice. String type is always allocated on the heap and are resizable.

Here are some operations you can perform on the String type

```
from "stdlib/String.shtk" use string, parseInt

fn main() -> int
{
    var message := "Hello".string()

    message += " World"         # append string
    println(message)            # prints Hello World

    println(message.length())   # prints 11

    println(message[6])         # indexing, prints W

    println(message[[4, 8]])    # substring, prints "o Wo"

    const number := "1234".parseInt().expect()  # parse to int
    println(number)                             # prints 1234

    println("ello" in message)  # check if substring exits, prints 1

    message.setChar(4, '$')     # set 4th character
    println(message)            # prints "Hell$ World"

    return 0
}
```

String literals (null terminated char arrays) are coerced into `String` type when passed as a parameter to a function,

```
from "stdlib/String.shtk" use String, string

fn printMessage(msg: String)
{
    println(msg)
}

fn main() -> int
{
    printMessage("Hello world".string())
    printMessage("Hello world")             # Will also work without .string

    return 0
}
```

However the coercion does not work on Arrays

```
from "stdlib/String.shtk" use String, string

fn printMessage(msg: String[2])
{
    println(msg[0], msg[1])
}

fn main() -> int
{
    printMessage(["Hello".string(),  "world".string()])
    printMessage(["Hello", "world"])             # compile error

    return 0
}
```

Here are list of operations you can do on the String type

-   `fn length(self: String) -> ulong`
-   `fn clone(self: String) -> String`
-   `fn isAlphaNumeric(self: String) -> bool`
-   `fn isSpace(self: String) -> bool`
-   `fn isUpper(self: String) -> bool`
-   `fn isLower(self: String) -> bool`
-   `fn isInteger(self: String) -> bool`
-   `fn isNumber(self: String) -> bool`
-   `fn lower(self: String) -> String`
-   `fn upper(self: String) -> String`
-   `fn subString(self: String, range: ulong[2]) -> Optional[String]`
-   `fn split(self: String, delimeter: char) -> List[String]`
-   `fn startsWith(self: String, other: String) -> bool`
-   `fn endsWith(self: String, other: String) -> bool`
-   `fn find(self: String, other: String) -> Optional[ulong]`
-   `fn strip(self: String) -> String`
-   `fn parseInt(self: String) -> Optional[int]`
-   `fn parseLong(self: String) -> Optional[long]`
-   `fn parseFloat(self: String) -> Optional[float]`
-   `fn parseDouble(self: String) -> Optional[double]`
-   Operators `+`, `==`, `!=`, `>`, `<`, `>=`, `<=`, `in`, `[]`,
-   `[[start, end]]` for substring

All String and other collection types in the standard library are copied by reference in ShnooTalk

```
from "stdlib/String.shtk" use string, parseInt

fn main() -> int
{
    var a := "abcd".string()
    var b := a

    b.setChar(2, '#')   # also modifies a

    println(a)          # prints "ab#d"

    return 0
}
```

If you don't want that behavior, you will have to use the `clone` method and explicitly copy

```
from "stdlib/String.shtk" use string, parseInt

fn main() -> int
{
    var a := "abcd".string()
    var b := a.clone()

    b.setChar(2, '#')   # does NOT modifies a

    println(a)          # prints "abcd"

    return 0
}
```

### List

List are similar to [arrays](#arrays) but they are allocated on the heap, have bounds checking and are resizable.

```
from "stdlib/List.shtk" use List, list

fn printList(list: List[int])
{
    for x in list       # iterate over list
        print(x; ", ")

    println("")
}

fn main() -> int
{
    var a := make(List[int], [1, 2, 3, 4, 5])
    println(a.length())             # prints 5
    printList(a)                    # prints 1, 2, 3, 4, 5

    println(a[2])                   # prints 3
    println(a.get(2).expect())      # same as a[2] prints 3
    printList(a[[1, 3]])            # prints 2, 3

    a.append(6)
    printList(a)                    # prints 1, 2, 3, 4, 5, 6

    a += [7, 8, 9]
    printList(a)                    # prints 1, 2, 3, 4, 5, 6, 7, 8, 9

    a.remove(2)                     # remove 3 from list
    printList(a)                    # prints 1, 2, 4, 5, 6, 7, 8, 9,

    println(a.pop().expect())       # remove last element and returns it, prints 9

    a.setItem(1, 100)               # set first element to 100
    printList(a)                    # 1, 100, 4, 5, 6, 7, 8,

    return 0
}
```

Here are the list of operations you can do on a List

-   `fn clone(self: List) -> List`
-   `fn append(mut self: List, item: T)`
-   `fn appendArray(mut self: List, items: T[], length: ulong)`
-   `fn remove(mut self: List, index: ulong) -> T`
-   `fn swapRemove(mut self: List, index: ulong) -> T`
-   `fn last(self: List) -> Optional[T]`
-   `fn clear(mut self: List)`
-   `fn pop(mut self: List) -> Optional[T]`
-   `fn length(self: List) -> ulong`
-   `fn isEmpty(self: List) -> bool`
-   `fn capacity(self: List) -> ulong`
-   `fn get(self: List, index: ulong) -> Optional[T]`
-   `fn setItem(mut self: List, index: ulong, item: T)`
-   `fn insert(mut self: List, index: ulong, item: T)`
-   `fn reverseInPlace(mut l: List)`
-   `fn reverse(l: List) -> List`
-   Operators `+` and `[index]`
-   `[[start, end]]` to copy range of elements into a new list

🦄 ShnooTalk will be able to print arrays and lists by default in the future

### List utilities

For types that have the `<` and `==` operator defined, ShnooTalk provides extra utility operation functions for List. Since these are generic functions, you need to use `foo[type](x, y, z)` function call syntax.

```
from "stdlib/List.shtk" use List, list
from "stdlib/ListUtils.shtk" use sort

fn printList(list: List[int])
{
    for x in list       # iterate over list
        print(x; ", ")

    println("")
}

fn main() -> int
{
    var a := make(List[int], [5, 3, 7, 4, 1])

    printList(sort[int](a))

    return 0
}
```

Here is the list of all utility functions you can import

-   `fn sortInPlace(mut l: List[T])`
-   `fn sort(l: List[T]) -> List[T]`
-   `fn search(l: List[T], item: T) -> Optional[ulong]`
-   `fn binarySearch(l: List[T], item: T) -> Optional[ulong]`
-   `fn count(l: List[T], item: T) -> ulong`
-   `fn isEqual(a: List[T], b: List[T]) -> bool`

### Dict

The Dict type is a map that stores mapping from one type to another. You can think of it as a List but the index can be any type and does not have to be sorted. The type you pass for the key should implement `==` and `<` operator.

```
from "stdlib/Dict.shtk" use Dict, dict
from "stdlib/Pair.shtk" use Pair, pair
from "stdlib/String.shtk" use String, string

fn main() -> int
{
    var qty := make(Dict[String, int], [
        pair[String, int]("apple", 10),
        pair[String, int]("orange", 4),
        pair[String, int]("banana", 12)
    ])

    println(qty["banana".string()])             # get value corresponding to banana, prints 12
    println(qty.get("banana").expect())         # same as qty["banana".string()], prints 12

    println("banana".string() in qty)           # check if a key exists, prints 1

    for val in qty[["apple".string(), "banana".string()]]   # get multiple values
        println(val)                                        # prints 10 and 12

    println(qty.remove("orange").expect())      # remove or delete a key, return corresponding value , prints 4
    println("orange".string() in qty)           # prints 0 because key was removed


    return 0
}
```

Here are list of all operation you can do on Dict

-   `fn items(self: Dict) -> List[Pair[K, V]]`
-   `fn clear(mut self: Dict)`
-   `fn clone(self: Dict) -> Dict`
-   `fn length(self: Dict) -> ulong`
-   `fn isEmpty(self: Dict) -> bool`
-   `fn capacity(self: Dict) -> ulong`
-   `fn get(self: Dict, key: K) -> Optional[V]`
-   `fn getList(self: Dict, keys: List[K]) -> List[Optional[V]]`
-   `fn insert(mut self: Dict, key: K, item: V) -> Optional[V]`
-   `fn remove(mut self: Dict, key: K) -> Optional[V]`
-   `fn keyExists(self: Dict, key: K) -> bool`
-   `fn keys(self: Dict) -> List[K]`
-   `fn values(self: Dict) -> List[V]`
-   `[key]` operator for retrieving one value
-   `[[key1, key2, key3]]` for retrieving multiple values as a list from arrays
-   `[List[K]]` also for retrieving multiple values as a list from List

### Time

Provides time related utilities.

```
from "stdlib/Time.shtk" use sleep, time

fn main() -> int
{
    sleep(1000)     # sleep for 1000 microseconds
    println(time()) # get current timestamp

    return 0
}
```

Here is the reference

-   `fn sleep(microseconds: int) -> Error[StdlibError]`
-   `fn time() -> long`

### Math

Provides basic math functions, defined in `stdlib/Math.shtk`

-   `PI` literal
-   `E` literal
-   `fn abs(x: double) -> double`
-   `fn absInt(x: int) -> int`
-   `fn absLong(x: long) -> long`
-   `fn ceil(x: double) -> double`
-   `fn ceilInt(x: double) -> int`
-   `fn ceilLong(x: double) -> long`
-   `fn floor(x: double) -> double`
-   `fn floorInt(x: double) -> int`
-   `fn floorLong(x: double) -> long`
-   `fn exp(x: double) -> double`
-   `fn log(x: double) -> double`
-   `fn sqrt(x: double) -> double`
-   `fn pow(base: double, to: double) -> double`
-   `fn degrees(angleRadians: double) -> double`
-   `fn radians(angleDegrees: double) -> double`
-   `fn cos(x: double) -> double`
-   `fn sin(x: double) -> double`
-   `fn tan(x: double) -> double`
-   `fn acos(x: double) -> double`
-   `fn asin(x: double) -> double`
-   `fn atan(x: double) -> double`

### Math generic

Defined in `stdlib/MathGeneric.shtk`. Provides utility function for types that have `>` and `<` operators defined.

```
from "stdlib/MathGeneric.shtk" use min, max

fn main() -> int
{
    const [a, b] := [1, 2]

    println(max[int](a, b))     # prints 2

    const [x, y] := [1.23, 4.5]

    println(min[double](x, y))  # prints 1.23

    return 0
}
```

Here is the reference

-   `fn max(a: T, b: T) -> T`
-   `fn min(a: T, b: T) -> T`

### Files

Provides functions to read and write files, here is a little example to read a file line by line

```
from "stdlib/File.shtk" use open

fn main() -> int
{

    var file := open("todo.md", "r").expect()

    loop {
        const [line, err] := file.readLine()
        if err break

        print(line)
    }

    return 0
}
```

You can also read an entire file into a string using a single function call,

```
from "stdlib/File.shtk" use readStringFromFile

fn main() -> int
{

    const contents := readStringFromFile("todo.md").expect()

    println(contents)

    return 0
}
```

Here is the reference, defined in `stdlib/File.shtk`

**Functions**

-   `fn open(path: String, mode: String) -> Result[File, StdlibError]`
    -   See `fopen` method from [C reference](https://en.cppreference.com/w/c/io/fopen) for modes
-   `fn readLine(self: File) -> Result[String, StdlibError]`
-   `fn read(self: File) -> Result[String, StdlibError]`
-   `fn seek(self: File, offset: long, whence: SeekType) -> Error[StdlibError]`
-   `fn tell(self: File) -> Result[long, StdlibError]`
-   `fn write(self: File, content: String) -> Error[StdlibError]`
-   `fn close(mut self: File) -> Error[StdlibError]`
-   `fn readStringFromFile(path: String) -> Result[String, StdlibError]`

**Enums**

See [C reference](https://en.cppreference.com/w/c/io/fseek) for more info on seek types

```
enum SeekType {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
}
```

### OS

Provides utility function to interact with the operating system, defined in `stdlib/OS.shtk`

-   `fn getArg(argc: int, argv: ulong[], idx: int) -> Optional[String]`
-   `fn panic(message: String, status: int)`
-   `fn system(command: String) -> Result[Pair[String, int], StdlibError]`
-   `fn getEnv(key: String) -> Optional[String]`
-   `fn getcwd() -> Optional[String]`
-   `fn chdir(path: String) -> Error[StdlibError]`
-   `fn mkdir(path: String) -> Error[StdlibError]`
-   `fn rmdir(path: String) -> Error[StdlibError]`
-   `fn rm(path: String) -> Error[StdlibError]`

## `make` builtin

This is a special function that accepts a type as its first argument, and then accepts any number of arguments of any type, then finds the first matching suitable constructor method for that type.

Here is an example,

```
struct Point
{
    var x: int, y: int, z: int
}

fn point(x: int, y: int, z: int) -> Point
{
    var self: Point
    .[self.x, self.y, self.z] = [x, y, z]

    return self
}

fn zeroPoint() -> Point
{
    return point(0, 0, 0)
}

fn pointFrom2dArray(a: int[2]) -> Point
{
    return point(a[0], a[1], 0)
}

fn pointFromAnyDimArray(a: int[], numElements: ulong) -> Point
{
    if numElements == 0
        return zeroPoint()
    if numElements == 1
        return point(a[0], 0, 0)
    if numElements == 2
        return point(a[0], a[1], 0)

    return point(a[0], a[1], a[2])
}

fn printPoint(self: Point)
{
    println("x="; self.x, "y="; self.y, "z="; self.z)
}

fn main() -> int
{
    const p1 := make(Point)                 # calls zeroPoint
    const p2 := make(Point, 1, 2, 3)        # calls point
    const p3 := make(Point, [1, 2])         # calls pointFrom2dArray
    const p4 := make(Point, [1, 2, 3])      # calls pointFromAnyDimArray
    const p5 := make(Point, [1, 2, 3, 4])   # calls pointFromAnyDimArray

    printPoint(p1)  # prints x=0 y=0 z=0
    printPoint(p2)  # prints x=1 y=2 z=3
    printPoint(p3)  # prints x=1 y=2 z=0
    printPoint(p4)  # prints x=1 y=2 z=3
    printPoint(p5)  # prints x=1 y=2 z=3

    return 0
}
```

The `pointFromAnyDimArray` constructor has to appear after `pointFrom2dArray` constructor, otherwise `pointFromAnyDimArray` constructor will always be used because it can also accept 2d arrays and make looks for the first matching constructor.

## Module system

You can use the `from` statement to import types, functions and literals from other files

_hello.shtk_

```
fn sayHello()
{
    println("Hello!")
}
```

_main.shtk_

```
from "hello.shtk" use sayHello

fn main() -> int
{
    sayHello()
    return 0
}
```

If there are two different files and both files have a function with the same name, there would be a name conflict, in that case you would use aliasing and the `::` syntax

_hello1.shtk_

```
fn sayHello()
{
    println("Hello ONE!")
}
```

_hello2.shtk_

```
fn sayHello()
{
    println("Hello TWO!")
}
```

_main.shtk_

```
use "hello1.shtk" as hello1
use "hello2.shtk" as hello2

fn main() -> int
{
    hello1::sayHello()  # prints Hello ONE
    hello2::sayHello()  # prints Hello TWO

    return 0
}
```

One you put a path say `foo/bar.shtk` in a use statement, ShnooTalk will check in the following places,

-   The current directory compiler is running
    -   `./foo/bar.shtk`
-   In $SHNOOTALK_PATH environment variable
    -   `$SHNOOTALK_PATH/foo/bar.shtk`
-   In `/usr/local/lib`
    -   `/usr/local/lib/shnootalk-<version>/foo/bar.shtk`

## Naming conventions

-   Function names are `camelCase`
-   Variable names are `camelCase`
-   Struct or class names are `PascalCase`
-   Enums are `PascalCase`
-   Module aliases should be all `lowercase` but `camelCase` can also be used
-   File and directory names are `PascalCase`

## Mutability and pointer rules

All mutability errors in ShnooTalk are based on the following rules

-   Mutable pointer
    -   Can mutate pointer itself to point to another variable
    -   Can mutate variable it is pointing to
-   Immutable pointer
    -   Cannot mutate the pointer to point to another variable
    -   Cannot mutate variable it is pointing to directly
    -   Can mutate variable it is pointing indirectly by using `<-` assignment to another mutable pointer
-   Mutable variable
    -   Can mutate value
    -   Can be `<-` assigned to pointers
-   Immutable variable
    -   Cannot be mutated
    -   Cannot be `<-` assigned to a pointer
-   Immutable variable of `struct` type containing a pointer (🦄 not enforced yet)
    -   Cannot mutate value
    -   Cannot be `<-` assigned to a pointer
    -   Cannot be assigned to mutable value

## TODO

-   Operator hooks
    -   `__multiply` hook
    -   `__divide` hook
    -   `__mod` hook
    -   `__add` hook
    -   `__subtract` hook
    -   `__rightShift` hook
    -   `__leftShift` hook
    -   `__bitwiseAnd` hook
    -   `__bitwiseXor` hook
    -   `__bitwiseOr` hook
    -   `__lessThan` hook
    -   `__lessThanOrEqual` hook
    -   `__greaterThan` hook
    -   `__isEqual` hook
    -   `__isNotEqual` hook
    -   `__in` hook
    -   `__isNonZero` hook
    -   `__subscript` hook
-   Print hooks
    -   `__toCharArray__` and `__toString__` hook
-   Loop hooks
    -   `for x in y` loop
    -   Destructured `for x in y` loop
-   Question mark operator hooks
    -   `__questionMarkContinue__` hook
    -   `__questionMarkUnwrap__` hook
    -   `__questionMarkError__` hook
    -   `__questionMarkConstruct__` hook
-   Resource management hooks
    -   `__beforeCopy__` hook
    -   `__deconstructor__` hook
-   Hooks for string and char types
    -   `__coerceCharArray__` hook
-   Generics
    -   Type hint
    -   Generic function call
    -   Same name existence
-   Using the compiler
    -   CLI args
    -   \_obj folder
    -   Linking
-   WASM support