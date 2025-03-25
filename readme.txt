PROJECT 2 README FILE

Compilation:
compile AutoGrad.cpp.
***C++17 or higher*** version is required for successful compilation.
***ATTENTION*** Dev-C++ may fail to compile this program if GCC version 
is too low to support C++17. Check your C++ version when a compile
error occurred.

Usage:
1. Generate EXE file.
2. run it.

Note:
1. this program is an infinite loop and every time the program receives 
a line as an expression. Then, the program will analyze and calculate its partial
derivatives and output them.
2. all invalid characters will be considered as ***SPACE***
3. IN THE WORST CASES, calculating partial derivative and simplifying it for 1 variable is O(n^4), so it may take a long period
of time waiting for the final result.
4. if you'd like to break the infinite loop and change the program into
  one-time process, comment "while(1)" in line 3742.