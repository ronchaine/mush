# mush
Public mirror for Multiple Useful Small Headers.

## Usage
Get core.hpp and any headers you would like.  Headers not in extra are allowed
to depend only on standard headers, core.hpp, string.hpp and buffer.hpp. #include
stuff you want and you are good to go.

Headers in extra are allowed to depend on whatever, so check out the header file's
documentation to figure it out.  Also, extra is deprecated, and everything in
there will be removed at later date.

There's 1.0 branch, which you should checkout if you just want to use the headers,
it contains none of the unfinished or deprecated stuff for now.

## Requirements
For everything to work, requires compiler that handles both C++ concepts TS and
C++17, currently, that means g++ 7.0 or newer.  For almost everything to work,
you're good with a compiler that can do C++17 and pass -DNO_CONCEPTS.

Should also work with clang++ and libc++ from 6.0.0 onward if you disable
concepts.  Once clang (or gcc) implements C++20 concepts instead of the TS,
there will be a move to those.

## Roadmap
* Document all files, generate doxygen use guide (past due)
* More testing

## Licence
Usually [zlib/libpng licence][zlib licence], see the individual files for details,
the licencing information is (usually, unless I forgot to add it) at the end of
the header file.




[zlib licence]: https://opensource.org/licenses/Zlib
