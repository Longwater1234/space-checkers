//
// Created by Davis on 11/4/2023.
//

#pragma once
#include <deque>
#include <iostream>
namespace chk
{
/**
 * Custom list with size limit. If max maxCapacity is reached, the first
 * item (index 0) is removed (FIFO), before inserting the new element in.
 */
template <typename T> class CircularBuffer
{

  public:
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
    int maxCapacity = 0; // max Capacity
    size_t size = 0;     // size of the buffer
    std::deque<T> buffer;
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
 * Get the first in queue
 * @tparam T any type
 * @return The first element in queue
 */
template <typename T> T &CircularBuffer<T>::getTop()
{
    return static_cast<T &>(buffer.at(0));
}

/**
 * printAll all elements in the buffer
 * @tparam T any type
 */
template <typename T> void CircularBuffer<T>::printAll()
{
    for (const auto &item : buffer)
    {
        std::cout << item << std::endl;
    }
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
    size++;
}
} // namespace chk
