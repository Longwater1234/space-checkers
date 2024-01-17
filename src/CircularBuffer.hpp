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
    explicit CircularBuffer(uint32_t maxCapacity) : maxCapacity(maxCapacity)
    {
        m_deque_.resize(maxCapacity);
    }
    void addItem(T item);
    T &getTop();
    [[nodiscard]] bool isEmpty() const;
    void printAll();
    void clean();

  private:
    uint32_t maxCapacity = 0; // max Capacity
    std::deque<T> m_deque_;   // actual container of elements
};

/**
 * remove all items from buffer
 * @tparam T any type
 */
template <typename T> void CircularBuffer<T>::clean()
{
    m_deque_.clear();
}

/**
 * Whether the buffer is empty
 * @tparam T any type
 * @return TRUE or FALSE
 */
template <typename T> bool CircularBuffer<T>::isEmpty() const
{
    return m_deque_.empty();
}

/**
 * Pop out the first in queue (front)
 * @tparam T any type
 * @return The first element in queue
 */
template <typename T> T &CircularBuffer<T>::getTop()
{
    return m_deque_.front();
}

/**
 * printAll all elements in the buffer
 * @tparam T any type
 */
template <typename T> void CircularBuffer<T>::printAll()
{
    std::cout << "[";
    for (const auto &item : m_deque_)
    {
        std::cout << item << ", ";
    }
    std::cout << "]" << std::endl;
}

/**
 * Add new element to buffer
 * @tparam T any type
 * @param item the element to be inserted
 */
template <typename T> void CircularBuffer<T>::addItem(T item)
{
    // if full, remove 1st item first
    if (m_deque_.size() >= maxCapacity)
    {
        m_deque_.pop_front();
    }
    m_deque_.push_back(item);
}
} // namespace chk
