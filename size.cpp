#include "size.h"
#include <iomanip> // for std::setw (which sets the width of the subsequent output)
#include <iostream>
#include <climits> // for CHAR_BIT
// std::size_t is defined in a number of different headers. If you need to use std::size_t, 
// <cstddef> is the best header to include, as it contains the least number of other defined identifiers.
#include <cstddef>

//#define LINE_WIDTH 32
constexpr int LINE_WIDTH{ 32 };

void cout_sizes() {
    std::cout << "A byte is " << CHAR_BIT << " bits\n\n";

    std::cout << std::left; // left justify output

    std::cout << std::setw(LINE_WIDTH) << "bool:" << sizeof(bool) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "char:" << sizeof(char) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "short:" << sizeof(short) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "int:" << sizeof(int) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "long:" << sizeof(long) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "long long:" << sizeof(long long) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "float:" << sizeof(float) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "double:" << sizeof(double) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "long double:" << sizeof(long double) << " bytes\n";
    std::cout << '\n';

    // fixed - width integers are not guaranteed to be defined on all architectures.
    // They only exist on systems where there are fundamental integral types that match their widths 
    // and following a certain binary representation.Your program will fail to compile on 
    // any such architecture that does not support a fixed - width integer that your program is using.
    // However, given that modern architectures have standardized around 8 / 16 / 32 / 64 - bit variables, 
    // this is unlikely to be a problem unless your program needs to be portable to some 
    // exotic mainframe or embedded architectures.
    // Best Practice: Use a fixed-width integer type when you need an integral type that has a guaranteed range.
    std::cout << std::int8_t{ 65 } << ": Fixed width integer int8_t/uint8_t (value 65) treated as char on most platforms\n";
    std::cout << '\n';

    // To help address the above downsides, C++ also defines two alternative sets of integers that are guaranteed to exist.
    // std::uint_least32_t will give you the smallest unsigned integer type that’s at least 32-bits.
    std::cout << std::setw(LINE_WIDTH) << "least 8:  " << sizeof(std::int_least8_t) * 8 << " bits\n";
    std::cout << std::setw(LINE_WIDTH) << "least 16: " << sizeof(std::int_least16_t) * 8 << " bits\n";
    std::cout << std::setw(LINE_WIDTH) << "least 32: " << sizeof(std::int_least32_t) * 8 << " bits\n";
    std::cout << std::setw(LINE_WIDTH) << "least 64: " << sizeof(std::int_least64_t) * 8 << " bits\n";
    std::cout << '\n';
    // std::int_fast32_t will give you the fastest signed integer type 
    // that’s at least 32-bits. By fastest, we mean the integral type that can be processed 
    // most quickly by the CPU.
    std::cout << std::setw(LINE_WIDTH) << "fast 8:  " << sizeof(std::int_fast8_t) * 8 << " bits\n";
    std::cout << std::setw(LINE_WIDTH) << "fast 16: " << sizeof(std::int_fast16_t) * 8 << " bits\n";
    std::cout << std::setw(LINE_WIDTH) << "fast 32: " << sizeof(std::int_fast32_t) * 8 << " bits\n";
    std::cout << std::setw(LINE_WIDTH) << "fast 64: " << sizeof(std::int_fast64_t) * 8 << " bits\n";
    std::cout << '\n';

    // However, these fast and least integers have their own downsides. First, not many programmers 
    // actually use them, and a lack of familiarity can lead to errors. Then the fast types can also 
    // lead to memory wastage, as their actual size may be significantly larger than indicated by their name.
    // Most seriously, because the size of the fast/least integers is implementation-defined, your program 
    // may exhibit different behaviors on architectures where they resolve to different sizes.
    // Best Practice: Avoid the fast and least integral types because they may exhibit different 
    // behaviors on architectures where they resolve to different sizes.

    // Best practice

    // Prefer int when the size of the integer doesn’t matter(e.g.the number will always fit within the range of a 2 - byte signed integer).For example, if you’re asking the user to enter their age, or counting from 1 to 10, it doesn’t matter whether int is 16 - bits or 32 - bits(the numbers will fit either way).This will cover the vast majority of the cases you’re likely to run across.
    //  - Prefer std::int#_t when storing a quantity that needs a guaranteed range.
    //  - Prefer std::uint#_t when doing bit manipulation or well - defined wrap - 
    //    around behavior is required(e.g. for cryptography or random number generation).
    // Avoid the following when possible :
    //  - short and long integers(prefer a fixed - width integer type instead).
    //  - The fast and least integral types(prefer a fixed - width integer type instead).
    //  - Unsigned types for holding quantities(prefer a signed integer type instead).
    //  - The 8 - bit fixed - width integer types(prefer a 16 - bit fixed - width integer type instead).
    //  - Any compiler - specific fixed - width integers(for example, Visual Studio defines __int8, __int16, etc…)

    // std::size_t is an alias for an implementation - defined unsigned integral type.
    // It is used within the standard library to represent the byte - size or length of objects.
    // std::size_t is actually a typedef.
    int x{ 5 };
    std::size_t s{ sizeof(x) };
    std::cout << std::setw(LINE_WIDTH) << "size_t of sizeof(int): " << sizeof(s) * 8 << " bits\n";
    // Much like an integer can vary in size depending on the system, std::size_t also varies in size. 
    // std::size_t is guaranteed to be unsigned and at least 16 bits, but on most systems will be 
    // equivalent to the address-width of the application. That is, for 32-bit applications, 
    // std::size_t will typically be a 32-bit unsigned integer, and for a 64-bit application, 
    // std::size_t will typically be a 64-bit unsigned integer.

    // floats
    std::cout << std::setw(LINE_WIDTH) << "float:" << sizeof(float) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "double:" << sizeof(double) << " bytes\n";
    std::cout << std::setw(LINE_WIDTH) << "long double:" << sizeof(long double) << " bytes\n";
    // long double is a strange type. On different platforms, its size can vary between 8 and 16 bytes, 
    // and it may or may not use an IEEE 754 compliant format. We recommend avoiding long double.
    std::cout << std::boolalpha; // print bool as true or false rather than 1 or 0
    std::cout << std::setw(LINE_WIDTH) << "float using IEEE 754: " << std::numeric_limits<float>::is_iec559 << '\n';
    std::cout << std::setw(LINE_WIDTH) << "double using IEEE 754: " << std::numeric_limits<double>::is_iec559 << '\n';
    std::cout << std::setw(LINE_WIDTH) << "long double using IEEE 754: " << std::numeric_limits<long double>::is_iec559 << '\n';

    
    // a float can hold any number with up to 6 significant digits: 6.67832 = 6.67832e+01
    std::cout << 6.67832f << "\n";
    // not precise anymore (gets rounded up)
    std::cout << 6.678325f << "\n";
    // but this only has one significant digit so it can be stored in a larger range (e-38 to e38 for float)
    float f{ 8e-38f };
    std::cout << f << " " << sizeof(f) << " bytes\n";
    //std::cout << std::setprecision(41);
    std::cout << 0.00000000000000000000000000000000000000008f << "\n"; // 8e-41 - precision loss (40 was ok)
    std::cout << 0.00000000000000000000000000000000000000008 << "\n"; // using double has double the precision

}

void unsigned_example() {
    // produces warning (treated as error)
    return;
    //unsigned short x{ 65535 }; // largest 16-bit unsigned value possible
    //std::cout << "x was: " << x << '\n';
    //std::cout << sizeof(x) << "\n";
    //x = 65536; // 65536 is out of our range, so we get modulo wrap-around
    //std::cout << "x is now: " << x << '\n';
    //std::cout << sizeof(x) << "\n";
    //x = 65537; // 65537 is out of our range, so we get modulo wrap-around
    //std::cout << "x is now: " << x << '\n';
}