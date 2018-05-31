# mush
Public mirror for Multiple Useful Small Headers.

## Usage
Get core.hpp and any headers you would like.  Headers not in extra are allowed
to depend only on standard headers, core.hpp, string.hpp and buffer.hpp. #include
stuff you want and you are good to go.  Headers in extra are allowed to depend on
whatever, so check out the header file's documentation to figure it out.

## Requirements
For everything to work, requires compiler that handles both C++ concepts TS and
C++17, currently, that means g++ 7.0 or newer.  For almost everything to work,
you're good with a compiler that can do C++17 and pass -DNO_CONCEPTS.

## ToDo before calling 1.0 done
* Document all files, generate doxygen use guide
* Remove "WIP" files from release
* More testing

## Licence
Usually [zlib/libpng licence][zlib licence], see the individual files for details,
the licencing information is (usually, unless I forgot to add it) at the end of
the header file.




[zlib licence]: https://opensource.org/licenses/Zlib
