#ifndef __TYPES_H__
#define __TYPES_H__

// C/C++
// typedef int Type;

// C++11, C++14, C++17, C++20, C++23 ...
#include <string>
using Type = int;

// T1 must be int for 32-bit architecture and long long for 64-bit architecture
// It must work for windows, linux, iOS, macOS, android, etc.

using T1 = int;

using Ref = long;

using Height = int;
using Balance = int;

using Char      = char; 
using Direction = int;

using Param1 = long;
using Param2 = std::string;

#endif // __TYPES_H__
