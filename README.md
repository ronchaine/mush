# mush
Public mirror for Multiple Useful Small Headers.

## Usage
Get core.hpp and any headers you would like.  Headers not in extra are allowed
to depend only on standard headers and core.hpp.  #include stuff you want and
you are good to go.  Headers in extra are allowed to depend on whatever, so
check out the header file's documentation to figure it out.

## Requirements
Requires compiler that handles both C++ concepts TS and C++17, currently, that
means g++ 7.0 or newer.

## ToDo before calling 1.0 done
* Move all defined concepts to a single hpp file
* Document all files, generate doxygen use guide
* Remove "WIP" files from release
* More testing

## Licence
Usually [zlib/libpng licence][zlib licence], see the individual files for details,
the licencing information is (usually, unless I forgot to add it) at the end of
the header file.




[zlib licence]: https://opensource.org/licenses/Zlib
