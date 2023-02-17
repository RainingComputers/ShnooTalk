# 0.3.1-alpha

-   Updated docs with missing stdlib functions
-   Add cortex m4 support

# 0.3.0-alpha

-   Bootstrap stdlib
-   Object files are now generated in separate `_obj` folder
-   Added `__isNonZero__` custom operator
-   Changed custom operator and resource management hooks to have `__` as prefix and suffix
-   Added CLI option to generate shared objects
-   Ignore `__beforeCopy__` and `__deconstructor__` hooks for variable named `self`
-   Fix bugs in importing enums
-   Autofill type parameters for generic function call
-   Added `___toCharArray__` and `__coerceCharArray__` hooks
-   Added `?` operator
-   Fix bugs related to `:=` operator

# 0.2.0-alpha

-   Support for incomplete types
-   Added `addr` built in
-   Added direct from imports
-   Added `:=` initialization
-   Added destructuring
-   Added generic function calls
-   Added `make` built in
-   Error messages now have line and col numbers and module trace
-   Type error messages show generic type errors better
-   Improved and better formatted error messages
-   Fixed sizeof to accept proper types
-   Added `-llvm-release` flag
-   Changed `sizeof` builtin
-   Added resource management hooks
-   Added `externC` functions
-   Fixed bugs related to mutability and pointers
-   Added `in` operator
-   Added subscript operator overloading
-   Added support for arrays and pointers of generic type
-   Improved enums
-   Added for each loop

# 0.1.1-alpha

-   Added `-release` flag that turns on LLVM optimizations
-   Fixed stack memory leak
-   Fixed bug in LLVM translator branching logic

# 0.1.0-alpha

-   Added generics
-   Added forward function declaration

# 0.0.1-alpha

-   First alpha release
-   Language core syntax
