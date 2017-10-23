#ifndef MUSH_RECTPACK
#define MUSH_RECTPACK

#include <cstdint>
#include <deque>
#include <algorithm>

#include "shapes.hpp"

namespace mush
{
    class RectanglePack
    {
        public:
            uint32_t    flags;
            uint32_t    minsize;

            std::deque<Rectangle> unused;
            std::deque<Rectangle> mapped;

            uint32_t    width, height;

            RectanglePack() : minsize(4), width(0), height(0) {}
           ~RectanglePack() {}

            size_t size()
            {
                return width * height;
            }

            //! Split the rectpack to smaller parts
            void split(const Rectangle& r)
            {
                Rectangle temp;
                for (auto r2 : unused)
                {
                    if (overlap(r, r2))
                    {
                        if (r.x < (int32_t)(r2.x + r2.w) && (int32_t)(r.x + r.w) > r2.x)
                        {
                            if (r.y > r2.y && r.y < (int32_t)(r2.y + r2.h))
                            {
                                temp = r2;
                                temp.h = r.y - r2.y;
                                if (temp.h > minsize)
                                    unused.push_front(temp);
                            }
                            if (r.y + r.h < r2.y + r2.h)
                            {
                                temp = r2;
                                temp.y = r.y + r.h;
                                temp.h = r2.y + r2.h - (r.y + r.h);
                                if (temp.h > minsize)
                                    unused.push_front(temp);
                            }
                        }
                        if (r.y < (int32_t)(r2.y + r2.h) && (int32_t)(r.y + r.h) > r2.y)
                        {
                            if (r.x > r2.x && r.x < (int32_t)(r2.x + r2.w))
                            {
                                temp = r2;
                                temp.w = r.x - r2.x;
                                if (temp.w > minsize)
                                    unused.push_front(temp);
                            }
                            if (r.x + r.w < r2.x + r2.w)
                            {
                                temp = r2;
                                temp.x = r.x + r.w;
                                temp.w = r2.x + r2.w - (r.x + r.w);
                                if (temp.w > minsize)
                                    unused.push_front(temp);
                            }
                        }
                    }
                }
            }

            //! Fit an rectangle into the pack
            Rectangle fit(uint32_t w, uint32_t h)
            {
                Rectangle rval = {0,0,0,0};
                std::sort(unused.begin(), unused.end());

                for (auto it : unused)
                {
                    if ((w < it.w) && (h < it.h))
                    {
                        rval.x = it.x;
                        rval.y = it.y;
                        rval.w = w;
                        rval.h = h;

                        split(rval);

                        return rval;
                    }
                }
                
                return rval;
            }

            //! Clear the packing data
            void reset()
            {
                unused.clear();
                mapped.clear();

                unused.emplace_back(Rectangle{0,0,width,height});
            }

            //! Removes overlapping and useless rectangles
            void prune(const Rectangle& r)
            {
                for (auto it = unused.begin(); it != unused.end(); ++it)
                {
                    if (overlap(r, *it))
                    {
                        it = unused.erase(it);
                        it--;
                    }
                }
                for (auto i = unused.begin(); i != unused.end(); ++i)
                    for (auto j = i; j != unused.end(); ++j)
                    {
                        if (i == j)
                            continue;
                        if (contains(*j, *i))
                        {
                            i = unused.erase(i);
                            i--;
                            break;
                        } else if (contains(*i, *j)) {
                            j = unused.erase(j);
                            j--;
                        }
                    }
            }
    };
}

#endif
/*
 Copyright (c) 2017 Jari Ronkainen

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose, including
    commercial applications, and to alter it and redistribute it freely, subject to
    the following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim
       that you wrote the original software. If you use this software in a product,
       an acknowledgment in the product documentation would be appreciated but is
       not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/
