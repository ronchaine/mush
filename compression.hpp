/*

LZF compress and decompress taken from FastLZ:

FastLZ - lightning-fast lossless compression library

Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)
Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
Copyright (C) 2005 Ariya Hidayat (ariya@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef MUSH_COMPRESSION
#define MUSH_COMPRESSION

#include "buffer.hpp"

// Defines for FastLZ
#define LZF_HASH_LOG  12
#define LZF_HASH_SIZE (1<< LZF_HASH_LOG)
#define LZF_HASH_MASK  (LZF_HASH_SIZE-1)

namespace mush
{
    namespace lzf
    {
        constexpr uint32_t MAX_COPY = 32;
        constexpr uint32_t MAX_LEN = 264;
        constexpr uint32_t MAX_DISTANCE = 8192;

        Buffer compress(const Buffer& input);
        Buffer uncompress(const Buffer& input);
    }
}

#ifdef MUSH_MAKE_IMPLEMENTATIONS
#define MUSH_IMPLEMENT_COMPRESSION
#endif

#ifdef MUSH_IMPLEMENT_COMPRESSION

#define UPDATE_HASH(v,p) { v = *((uint16_t*)p); v ^= *((uint16_t*)(p+1))^(v>>(16-LZF_HASH_LOG)); }

namespace mush
{
    namespace detail
    {
        // Lossless compression using LZF algorithm, this is faster on modern CPU than
        // the original implementation in http://liblzf.plan9.de/
        int compress(const void* input, int length, void* output, int maxout)
        {
            if (input == 0 || length < 1 || output == 0 || maxout < 2) {
                return 0;
            }

            const uint8_t* ip = (const uint8_t*) input;
            const uint8_t* ip_limit = ip + length - mush::lzf::MAX_COPY - 4;
            uint8_t* op = (uint8_t*) output;
            const uint8_t* last_op = (uint8_t*) output + maxout - 1;

            const uint8_t* htab[LZF_HASH_SIZE];
            const uint8_t** hslot;
            uint32_t hval;

            uint8_t* ref;
            int32_t copy;
            int32_t len;
            int32_t distance;
            uint8_t* anchor;

            /* initializes hash table */
            for (hslot = htab; hslot < htab + LZF_HASH_SIZE; ++hslot) {
                *hslot = ip;
            }

            /* we start with literal copy */
            copy = 0;
            *op++ = mush::lzf::MAX_COPY - 1;

            /* main loop */
            while (ip < ip_limit) {
                /* find potential match */
                UPDATE_HASH(hval, ip);
                hslot = htab + (hval & LZF_HASH_MASK);
                ref = (uint8_t*) * hslot;

                /* update hash table */
                *hslot = ip;

                /* find itself? then it's no match */
                if (ip == ref)
                    goto literal;

                /* is this a match? check the first 2 bytes */
                if (*((uint16_t*)ref) != *((uint16_t*)ip))
                    goto literal;

                /* now check the 3rd byte */
                if (ref[2] != ip[2])
                    goto literal;

                /* calculate distance to the match */
                distance = ip - ref;

                /* skip if too far away */
                if (distance >= mush::lzf::MAX_DISTANCE)
                    goto literal;

                /* here we have 3-byte matches */
                anchor = (uint8_t*)ip;
                len = 3;
                ref += 3;
                ip += 3;

                /* now we have to check how long the match is */
                if (ip < ip_limit - mush::lzf::MAX_LEN) {
                    while (len < mush::lzf::MAX_LEN - 8) {
                        /* unroll 8 times */
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        if (*ref++ != *ip++) break;
                        len += 8;
                    }
                    --ip;
                }
                len = ip - anchor;

                /* just before the last non-matching byte */
                ip = anchor + len;

                /* if we have copied something, adjust the copy count */
                if (copy) {
                    /* copy is biased, '0' means 1 byte copy */
                    anchor = anchor - copy - 1;
                    *(op - copy - 1) = copy - 1;
                    copy = 0;
                } else {
                    /* back, to overwrite the copy count */
                    --op;
                }

                /* length is biased, '1' means a match of 3 bytes */
                len -= 2;

                /* distance is also biased */
                --distance;

                /* encode the match */
                if (len < 7) {
                    if (op + 2 > last_op) {
                        return 0;
                    }
                    *op++ = (len << 5) + (distance >> 8);
                } else {
                    if (op + 3 > last_op) {
                        return 0;
                    }
                    *op++ = (7 << 5) + (distance >> 8);
                    *op++ = len - 7;
                }
                *op++ = (distance & 255);

                /* assuming next will be literal copy */
                *op++ = mush::lzf::MAX_COPY - 1;

                /* update the hash at match boundary */
                --ip;
                UPDATE_HASH(hval, ip);
                htab[hval & LZF_HASH_MASK] = ip;
                ++ip;

                continue;

            literal:
                if (op + 1 > last_op) {
                    return 0;
                }
                *op++ = *ip++;
                ++copy;
                if (copy >= mush::lzf::MAX_COPY) {
                    // start next literal copy item
                    copy = 0;
                    *op++ = mush::lzf::MAX_COPY - 1;
                }
            }

            /* left-over as literal copy */
            ip_limit = (const uint8_t*)input + length;

            // TODO: smart calculation to see here if enough output is left

            while (ip < ip_limit) {
                if (op == last_op) {
                    return 0;
                }
                *op++ = *ip++;
                ++copy;
                if (copy == mush::lzf::MAX_COPY) {
                    // start next literal copy item
                    copy = 0;
                    if (ip < ip_limit) {
                        if (op == last_op) {
                            return 0;
                        }
                        *op++ = mush::lzf::MAX_COPY - 1;
                    } else {
                        // do not write possibly out of bounds
                        // just pretend we moved one more, for the final treatment
                        ++op;
                    }
                }
            }

            /* if we have copied something, adjust the copy length */
            if (copy) {
                *(op - copy - 1) = copy - 1;
            } else {
                --op;
            }

            return op - (uint8_t*)output;
        }

        int decompress(const void* input, int length, void* output, int maxout)
        {
            if (input == 0 || length < 1) {
                return 0;
            }
            if (output == 0 || maxout < 1) {
                return 0;
            }

            const uint8_t* ip = (const uint8_t*) input;
            const uint8_t* ip_limit  = ip + length - 1;
            uint8_t* op = (uint8_t*) output;
            uint8_t* op_limit = op + maxout;
            uint8_t* ref;

            while (ip < ip_limit) {
                uint32_t ctrl = (*ip) + 1;
                uint32_t ofs = ((*ip) & 31) << 8;
                uint32_t len = (*ip++) >> 5;

                if (ctrl < 33) {
                    /* literal copy */
                    if (op + ctrl > op_limit)
                        return 0;

                    /* crazy unrolling */
                    if (ctrl) {
                        *op++ = *ip++;
                        --ctrl;

                        if (ctrl) {
                            *op++ = *ip++;
                            --ctrl;

                            if (ctrl) {
                                *op++ = *ip++;
                                --ctrl;

                                for (;ctrl; --ctrl)
                                    *op++ = *ip++;
                            }
                        }
                    }
                } else {
                    /* back reference */
                    --len;
                    ref = op - ofs;
                    --ref;

                    if (len == 7 - 1)
                        len += *ip++;

                    ref -= *ip++;

                    if (op + len + 3 > op_limit)
                        return 0;

                    if (ref < (uint8_t *)output)
                        return 0;

                    *op++ = *ref++;
                    *op++ = *ref++;
                    *op++ = *ref++;
                    if (len)
                        for (; len; --len)
                            *op++ = *ref++;
                }
            }

            return op - (uint8_t*)output;
        }
    }

    Buffer lzf::compress(const Buffer& input)
    {
        Buffer output;

        if (input.size() == 0)
            return output;

        input.seek(0);
        output.seek(0);

        const void* const in_data = (const void*)input.data();
        uint32_t in_len = (uint32_t)input.size();

        output.resize(in_len + 4 + 1);

        output[0] = in_len & 255;
        output[1] = (in_len >> 8) & 255;
        output[2] = (in_len >> 16) & 255;
        output[3] = (in_len >> 24) & 255;
        output[4] = 1;

        uint32_t out_len = in_len - 1;
        uint8_t* out_data = (uint8_t*)(output.data() + 5);

        uint32_t len = detail::compress(in_data, in_len, out_data, out_len);

        if ((len > out_len) || (len == 0))
        {
            output.replace(5, output.size() - 5, input);
            output[4] = 0;
        } else {
            output.resize(len + 5);
        }

        output.shrink_to_fit();

        return output;
    }

    Buffer lzf::uncompress(const Buffer& input)
    {
        Buffer output;

        size_t unpacked_size = 0;
        unpacked_size |= ((uint8_t)input[0]);
        unpacked_size |= ((uint8_t)input[1]) << 8;
        unpacked_size |= ((uint8_t)input[2]) << 16;
        unpacked_size |= ((uint8_t)input[3]) << 24;

        input.seek(0);
        size_t unpacked = input.read_le<uint32_t>();

        std::cout << unpacked_size << "==" << unpacked << "\n";

        output.resize(unpacked_size);

        uint8_t flag = input[4];

        const void* const in_data = (const void*)(input.data() + 5);
        int in_len = (int)input.size() - 5;
        uint8_t* out_data = output.data();
        uint32_t out_len = unpacked_size;

        if (flag == 0) {
            memcpy(output.data(), in_data, in_len);
        } else {
            size_t len = detail::decompress(in_data, in_len, out_data, out_len);
            assert(len == out_len);
        }

        return output;
    }
}
#endif
#endif
