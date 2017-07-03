#define MUSH_MAKE_IMPLEMENTATIONS

#include "image.hpp"
#include "shapes.hpp"

#include "checksum.hpp"

#include "string.hpp"

int main(int argc, char* argv[])
{
    /*
    const mush::Rectangle r1 = {0,0,20,20};
    mush::Point p1;
    mush::Rectangle r2;
    mush::Rectangle& r3 = r2;
    if (mush::overlap(r1, r2))
        std::cout << "overlap!\n";
    if (mush::overlap(r1, r3))
        std::cout << "overlap!\n";

    if (mush::overlap(p1, r2));
    */

    uint32_t crc_table[256];

    uint32_t c, n, k;

    for (n = 0; n < 256; ++n)
    {
        c = n;
        for (k = 0; k < 8; ++k)
        {
            if (c & 1)
                c = 0xedb88320 ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }

    for (int i = 0; i < 256; ++i)
        std::cout << mush::crc::table[i] << "\tvs\t" << crc_table[i] << "\tvs\t" << "\n";

    int i = 5;

    mush::string intstr = i;

    std::cout << intstr << "\n";

    return 0;
}
