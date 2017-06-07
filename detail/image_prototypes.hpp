#ifndef MUSH_IMAGE_PROTOTYPES
#define MUSH_IMAGE_PROTOTYPES

#include <cstdint>
#include <cstdlib>

#include <vector>

#include "../buffer.hpp"

namespace mush
{
    namespace impl
    {
        struct P3D
        {
            size_t x, y, z;
            P3D(size_t x, size_t y, size_t z) : x(x), y(y), z(z) {}
        };

        struct P2D
        {
            size_t x, y;
            P2D(size_t x, size_t y) : x(x), y(y) {}
        };
    }

    class Palette : public std::vector<uint32_t>
    {
    };

    /** 
     * @brief Basic image class
     */
    class Image
    {
        uint32_t    width;
        uint32_t    height;
        uint32_t    channels;

        Buffer      image;
        Palette     palette;
        
        struct pixel_access_type;

        pixel_access_type operator[](impl::P2D l);
        uint8_t& operator[](impl::P3D l);
    };
    
    /** 
     * @brief Proxy function for easier access to Image data
     */
    struct Image::pixel_access_type
    {
        private:
            Image& img;
            size_t x, y;

        public:
            pixel_access_type(Image& image, size_t x, size_t y) : img(image), x(x), y(y) {}

            void operator=(uint32_t col)
            {
                assert((img.channels == 4) && "operator= only implemented for 4-channel images");
                img.image[img.width * img.channels * y + img.channels * x + 0] = (col & (0xff000000)) >> 24;
                img.image[img.width * img.channels * y + img.channels * x + 1] = (col & (0xff0000)) >> 16;
                img.image[img.width * img.channels * y + img.channels * x + 2] = (col & (0xff00)) >> 8;
                img.image[img.width * img.channels * y + img.channels * x + 3] = (col & (0xff));
            }
    };
    
    inline Image::pixel_access_type Image::operator[](impl::P2D l)
    {
        return pixel_access_type(*this, l.x, l.y);
    }

    inline uint8_t& Image::operator[](impl::P3D l)
    {
        return image[width * channels * l.y + channels * l.x + l.z];
    }
}

#endif
