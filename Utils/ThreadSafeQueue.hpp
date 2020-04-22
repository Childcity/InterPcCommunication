                                                      //
// Created by childcity on 05.02.20.
//

#ifndef LEARNMULTITHREADC_MYTHREADSAFEQUEUE_HPP
#define LEARNMULTITHREADC_MYTHREADSAFEQUEUE_HPP


#include <condition_variable>
#include <memory>

template <class T>
struct ThreadSafeQueue {

private:

    // Node of queue list
    struct Node {
        std::shared_ptr<T> Data{};
        std::unique_ptr<Node> Next{};
    };

private:
    std::unique_ptr<Node> head_{};
    mutable std::mutex headmutex_{};

    Node *tail_{};
    mutable std::mutex tailmutex_{};

    std::condition_variable dataCond_{};

public:
    ThreadSafeQueue()
            : head_(std::make_unique<Node>())
            , tail_(head_.get())
    {}

    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue &other) = delete;

    ~ThreadSafeQueue() = default;

    void push(T newVal)
    {
        // Next two operations require memory allocation.
        // So we do this without blocking any std::mutex
        std::shared_ptr<T> newData = std::make_shared<T>(std::move(newVal));
        std::unique_ptr<Node> p = std::make_unique<Node>();

        // Next block is running under std::mutex 'tailstd::mutex_'
        {
            std::lock_guard<std::mutex> lk(tailmutex_);

            tail_->Data = newData;

            Node *const newTail = p.get();
            tail_->Next = std::move(p);
            tail_ = newTail;
        }

        dataCond_.notify_one();
    }

    void waitAndPop(T &value)
    {
        const std::unique_ptr<Node> oldData = waitAndPopHead(value);
    }

    std::shared_ptr<T> waitAndPop()
    {
        const std::unique_ptr<Node> oldData = waitAndPopHead();
        return oldData->Data;
    }

    bool tryPop(T &value)
    {
        const std::unique_ptr<Node> oldHead = tryPopHead(value);
        return oldHead ? true : false;
    }

    std::shared_ptr<T> tryPop()
    {
        std::unique_ptr<Node> oldHead = tryPopHead();
        return oldHead ? oldHead->Data : std::shared_ptr<T>();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(headmutex_);
        return head_.get() == getTail();
    }

    // Very long and blocking operation!
    // Blocks Head and tail!
    size_t size() const
    {
        size_t size = 0;
        Node *nodePtr = nullptr;

        std::lock_guard<std::mutex> headLock(headmutex_);
        std::lock_guard<std::mutex> tailLock(tailmutex_);
        {
            nodePtr = head_.get();
            while (nodePtr != tail_)
            {
                nodePtr = nodePtr->Next.get();
                size++;
            }
        }

        return size;
    }

private:
    Node* getTail() const
    {
        std::lock_guard<std::mutex> tailLock(tailmutex_);
        return tail_;
    }

    std::unique_ptr<Node> popHead()
    {
        std::unique_ptr<Node> oldHead = std::move(head_);
        head_ = std::move(oldHead->Next);
        return oldHead;
    }

    std::unique_lock<std::mutex> waitForData()
    {
        std::unique_lock<std::mutex> headLock(headmutex_);
        dataCond_.wait(headLock, [&] { return head_.get() != getTail(); });
        return headLock;
    }

    std::unique_ptr<Node> waitAndPopHead()
    {
        std::unique_lock<std::mutex> headLock = waitForData();
        return popHead();
    }

    std::unique_ptr<Node> waitAndPopHead(T &value)
    {
        std::unique_lock<std::mutex> headLock = waitForData();
        value = std::move(*head_->Data);
        return popHead();
    }

    std::unique_ptr<Node> tryPopHead()
    {
        std::lock_guard<std::mutex> headLock(headmutex_);

        if(head_.get() == getTail()){
            return {};
        }

        return popHead();
    }

    std::unique_ptr<Node> tryPopHead(T &value)
    {
        std::lock_guard<std::mutex> headLock(headmutex_);

        if(head_.get() == getTail()){
            return {};
        }

        value = std::move(*head_->Data);
        return popHead();
    }
};


#endif //LEARNMULTITHREADC_MYTHREADSAFEQUEUE_HPP
