//
// Created by Davis on 11/4/2023.
//

#pragma once
#include <deque>
#include <iostream>
namespace chk
{
/**
 * Rotating container with strict size limit. If `maxCapacity` is reached, remove the first added item
 * before inserting the new element in. Works in FIFO policy
 */
template <typename T> class CircularBuffer
{

  public:
    // Constructor, sets maxCapacity limit
    explicit CircularBuffer(int maxCapacity) : maxCapacity(maxCapacity)
    {
        buffer.resize(maxCapacity);
    }
    void addItem(T item);
    T &getTop();
    [[nodiscard]] bool isEmpty() const;
    void printAll();
    void clean();

  private:
    int maxCapacity = 0;  // max Capacity
    size_t buf_size = 0;  // size of the buffer
    std::deque<T> buffer; // actual store of elements

  public:
    bool operator==(const CircularBuffer &other) const
    {
        return this->buffer == other.buffer;
    }
};

/**
 * remove all items from buffer
 * @tparam T any type
 */
template <typename T> void CircularBuffer<T>::clean()
{
    buffer.clear();
}

/**
 * Whether the buffer is empty
 * @tparam T any type
 * @return TRUE or FALSE
 */
template <typename T> bool CircularBuffer<T>::isEmpty() const
{
    return buffer.empty();
}

/**
 * Pop out the first in queue (front)
 * @tparam T any type
 * @return The first element in queue
 */
template <typename T> T &CircularBuffer<T>::getTop()
{
    T &val = buffer.front();
    buffer.pop_front(); // remove it
    return val;
}

/**
 * printAll all elements in the buffer
 * @tparam T any type
 */
template <typename T> void CircularBuffer<T>::printAll()
{
    for (const auto &item : buffer)
    {
        std::cout << item << '\n';
    }
    std::cout << std::endl;
}

/**
 * Add new element to buffer
 * @tparam T any type
 * @param item the element to be inserted
 */
template <typename T> void CircularBuffer<T>::addItem(T item)
{
    // if full, remove 1st item first
    if (buffer.size() >= maxCapacity)
    {
        buffer.pop_front();
    }
    buffer.push_back(item);
    buf_size++;
}
} // namespace chk
