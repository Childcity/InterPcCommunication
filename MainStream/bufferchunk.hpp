#ifndef BUFFERCHUNK_H
#define BUFFERCHUNK_H


#include "constants.h"
#include <array>

template<std::size_t SIZE>
struct BufferChunk {
    std::array<char, SIZE> data;
    std::size_t actualSize = 0;

    BufferChunk() = default;
    BufferChunk(const BufferChunk &) = default;
    BufferChunk(BufferChunk &&) = default;
    BufferChunk& operator=(const BufferChunk &) = default;
    BufferChunk& operator=(BufferChunk &&) = default;

    char &operator[](std::size_t idx)       { return data[idx]; }
    const char &operator[](std::size_t idx) const { return data[idx]; }
};

#endif // BUFFERCHUNK_H
