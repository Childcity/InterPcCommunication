#ifndef COMMON_HPP
#define COMMON_HPP

#include "constants.h"
#include "bufferchunk.hpp"
#include "../Utils/ThreadSafeQueue.hpp"


namespace MainStream {

using InBuffChunk = BufferChunk<READ_BUFSIZE>;
using OutBuffChunk = BufferChunk<WRITE_BUFSIZE>;

using InThreadSafeQueue = ThreadSafeQueue<InBuffChunk>;
using OutThreadSafeQueue = ThreadSafeQueue<OutBuffChunk>;

}

#endif // COMMON_HPP
