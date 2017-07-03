#include "image.hpp"
#include "shapes.hpp"

int main(int argc, char* argv[])
{
    const mush::Rectangle r1 = {0,0,20,20};
    mush::Point p1;
    mush::Rectangle r2;
    mush::Rectangle& r3 = r2;
    if (mush::overlap(r1, r2))
        std::cout << "overlap!\n";
    if (mush::overlap(r1, r3))
        std::cout << "overlap!\n";

    if (mush::overlap(p1, r2));

    return 0;
}
