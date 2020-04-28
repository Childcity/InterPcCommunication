#ifndef COMMON_HPP
#define COMMON_HPP

#include "constants.h"
#include "bufferchunk.hpp"
#include "../Utils/ThreadSafeQueue.hpp"

#include <QtConcurrent>


namespace MainStream {

using InBuffChunk = BufferChunk<READ_BUFSIZE>;
using OutBuffChunk = BufferChunk<WRITE_BUFSIZE>;

using InThreadSafeQueue = ThreadSafeQueue<InBuffChunk>;
using OutThreadSafeQueue = ThreadSafeQueue<OutBuffChunk>;

template<class CHUNK_T>
static QFuture<void> FillQueueInSepThread(ThreadSafeQueue<CHUNK_T> &queue, const QByteArray &data)
{
    return QtConcurrent::run([&queue](const QByteArray &data){
        for (int i = 0; i < data.size(); i += WRITE_BUFSIZE) {
            OutBuffChunk chunk;
            int actualSize;
            for (actualSize = 0; actualSize < static_cast<int>(WRITE_BUFSIZE) && (i+actualSize) < data.size(); ++actualSize) {
                chunk[actualSize] = data[i+actualSize];
            }
            chunk.actualSize = actualSize;
            queue.push(std::move(chunk));
        }
    }, data);
}

}

#endif // COMMON_HPP
