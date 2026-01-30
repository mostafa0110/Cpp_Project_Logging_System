#pragma once

#include <vector>
#include <optional>
#include <type_traits>
#include <utility>
#include <concepts>
#include <mutex>
#include <condition_variable>

template <typename T>
class RingBuffer
{
private:
    std::vector<std::optional<T>> buffer;
    std::size_t head = 0;
    std::size_t tail = 0;
    std::size_t elemCount = 0;
    std::size_t maxCapacity;
    mutable std::mutex bufferMutex;
    std::condition_variable notEmpty;
    std::condition_variable notFull;

public:
    explicit RingBuffer(std::size_t capacity)
        : buffer(capacity), maxCapacity(capacity)
    {
    }

    ~RingBuffer() = default;

    // Non-copyable
    RingBuffer(const RingBuffer &) = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;

    // Move constructor
    RingBuffer(RingBuffer &&other) noexcept
        : maxCapacity(0)
    {
        std::lock_guard<std::mutex> lock(other.bufferMutex);
        buffer = std::move(other.buffer);
        head = other.head;
        tail = other.tail;
        elemCount = other.elemCount;
        maxCapacity = other.maxCapacity;
        
        other.head = 0;
        other.tail = 0;
        other.elemCount = 0;
        other.maxCapacity = 0;
    }

    RingBuffer &operator=(RingBuffer &&) = delete;

    // Blocking push
    template <typename U>
        requires std::convertible_to<U, T>
    void push(U &&value)
    {
        std::unique_lock<std::mutex> lock(bufferMutex);
        notFull.wait(lock, [this]() { return !isFull_unlocked(); });
        
        buffer[head] = std::forward<U>(value);
        head = (head + 1) % maxCapacity;
        ++elemCount;
        
        lock.unlock();
        notEmpty.notify_one();
    }

    // Non-blocking push
    template <typename U>
        requires std::convertible_to<U, T>
    [[nodiscard]] bool tryPush(U &&value)
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        if (isFull_unlocked())
        {
            return false;
        }
        buffer[head] = std::forward<U>(value);
        head = (head + 1) % maxCapacity;
        ++elemCount;
        notEmpty.notify_one();
        return true;
    }

    // Blocking pop
    [[nodiscard]] T pop()
    {
        std::unique_lock<std::mutex> lock(bufferMutex);
        notEmpty.wait(lock, [this]() { return !isEmpty_unlocked(); });
        
        T value = std::move(buffer[tail].value());
        buffer[tail].reset();
        tail = (tail + 1) % maxCapacity;
        --elemCount;
        
        lock.unlock();
        notFull.notify_one();
        return value;
    }

    // Non-blocking pop
    [[nodiscard]] std::optional<T> tryPop()
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        if (isEmpty_unlocked())
        {
            return std::nullopt;
        }
        T value = std::move(buffer[tail].value());
        buffer[tail].reset();
        tail = (tail + 1) % maxCapacity;
        --elemCount;
        notFull.notify_one();
        return value;
    }

    [[nodiscard]] bool isEmpty() const noexcept
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        return isEmpty_unlocked();
    }

    [[nodiscard]] bool isFull() const noexcept
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        return isFull_unlocked();
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        return elemCount;
    }

    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return maxCapacity;
    }

private:
    [[nodiscard]] bool isEmpty_unlocked() const noexcept
    {
        return elemCount == 0;
    }

    [[nodiscard]] bool isFull_unlocked() const noexcept
    {
        return elemCount == maxCapacity;
    }
};