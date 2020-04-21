#ifndef COMMON_HPP
#define COMMON_HPP

#include "constants.h"
#include "bufferchunk.hpp"
#include "ThreadSafeQueue.hpp"


namespace MainStream {

using InBuffChunk = char; //BufferChunk<READ_BUFSIZE>;
using OutBuffChunk = char; //BufferChunk<READ_BUFSIZE>;

using InThreadSafeQueue = ThreadSafeQueue<InBuffChunk>;
using OutThreadSafeQueue = ThreadSafeQueue<OutBuffChunk>;

}

#endif // COMMON_HPP
