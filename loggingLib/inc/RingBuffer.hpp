#pragma once

#include <vector>
#include <optional>
#include <type_traits>
#include <utility>
#include <concepts>

template <typename T>
class RingBuffer
{
private:
    std::vector<std::optional<T>> buffer;
    std::size_t head = 0;      // Next write position
    std::size_t tail = 0;      // Next read position
    std::size_t elemCount = 0; // Current number of elements
    std::size_t maxCapacity;   // Maximum capacity

public:
    explicit RingBuffer(std::size_t capacity)
        : buffer(capacity), maxCapacity(capacity)
    {
    }

    ~RingBuffer() = default;

    // Non-copyable
    RingBuffer(const RingBuffer &) = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;

    // Movable
    RingBuffer(RingBuffer &&other) noexcept
        : buffer(std::move(other.buffer)), head(other.head), tail(other.tail), elemCount(other.elemCount), maxCapacity(other.maxCapacity)
    {
        other.head = 0;
        other.tail = 0;
        other.elemCount = 0;
        other.maxCapacity = 0;
    }

    RingBuffer &operator=(RingBuffer &&other) noexcept
    {
        if (this != &other)
        {
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
        return *this;
    }

    // perfect forwarding
    template <typename U>
        requires std::convertible_to<U, T>
    [[nodiscard]] bool tryPush(U &&value)
    {
        if (isFull())
        {
            return false;
        }
        buffer[head] = std::forward<U>(value);
        head = (head + 1) % maxCapacity;
        ++elemCount;
        return true;
    }

    [[nodiscard]] std::optional<T> tryPop()
    {
        if (isEmpty())
        {
            return std::nullopt;
        }
        T value = std::move(buffer[tail].value());
        buffer[tail].reset(); // Clear the slot  -> destructor is called so has-value return false 
        tail = (tail + 1) % maxCapacity;
        --elemCount;
        return value;
    }

    // [[nodiscard]] -> if the function is called and the return value is ignored , compiler issues a warning
    // const -> read-only, It will not modify any member variables
    // noexcept -> the function never throw an exception (compiler skip logic needed to handle try-catch blocks)
    [[nodiscard]] bool isEmpty() const noexcept
    {
        return elemCount == 0;
    }

    [[nodiscard]] bool isFull() const noexcept
    {
        return elemCount == maxCapacity;
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return elemCount;
    }

    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return maxCapacity;
    }
};