#ifndef BUFFERCHUNK_H
#define BUFFERCHUNK_H


#include "constants.h"
#include <array>

template<std::size_t SIZE>
struct BufferChunk {
    std::array<char, SIZE> data;
    std::size_t actualSize;
};

#endif // BUFFERCHUNK_H
