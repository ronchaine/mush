#ifndef MUSH_RINGBUFFER
#define MUSH_RINGBUFFER

#include <vector>
#include <algorithm>

namespace mush
{
    template <typename T, class Container = std::vector<T>>
    class Ring
    {
        private:
            std::vector<T>  buffer;
            size_t          current;

        public:
            void resize(size_t size)
            {
                size_t pos = current % buffer.size();
                if (size < buffer.size())
                {
                    return;
                }

                std::vector<T> newbuffer;
                newbuffer.resize(size);

                std::rotate_copy(buffer.first(), buffer.last(), buffer + pos, newbuffer.begin());

                buffer = newbuffer;

                return;
            }

            void push(T element)
            {
                size_t pos = current % buffer.size();
                buffer[pos] = element;
                current++;
            }

            Ring(size_t size)
            {
                buffer.resize(size);
            }
            
            Ring() : Ring(20)
            {
            }


           ~Ring()
            {
            }
    };
}
#endif
