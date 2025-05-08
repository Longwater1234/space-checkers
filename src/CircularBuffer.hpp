//
// Created by Davis on 2023/11/04
//

#pragma once
#include <cstdint>
#include <deque>

namespace chk
{
/**
 * (NOT THREAD SAFE!) Rotating container with strict size limit. If `maxCapacity` is reached, remove the first added
 * item before inserting the new element in. Works in FIFO policy
 */
template <typename T> class CircularBuffer
{

  public:
    // Constructor, sets maxCapacity limit
    explicit CircularBuffer(const uint32_t maxCapacity) : max_capacity(maxCapacity)
    {
        m_deque.resize(max_capacity);
    }
    CircularBuffer() = delete;
    CircularBuffer &operator=(const CircularBuffer &) = delete;
    CircularBuffer(CircularBuffer &other) = delete;
    void addItem(const T &item); // copy overload
    void addItem(T &&item);      // Move overload
    T &getFront() noexcept;
    void removeFirst();
    [[nodiscard]] bool isEmpty() const noexcept;
    const std::deque<T> &getAll() const;
    void clean();

  private:
    uint32_t max_capacity; // max Capacity
    std::deque<T> m_deque; // actual container of elements
};

/**
 * remove all items from buffer
 * @tparam T any type
 */
template <typename T> void CircularBuffer<T>::clean()
{
    m_deque.clear();
}

/**
 * Whether the buffer is empty
 * @tparam T any type
 * @return TRUE or FALSE
 */
template <typename T> bool CircularBuffer<T>::isEmpty() const noexcept
{
    return m_deque.empty();
}

/**
 * returns all elements from the buffer
 */
template <typename T> inline const std::deque<T> &CircularBuffer<T>::getAll() const
{
    return this->m_deque;
}

/**
 * Get the first in queue (front), WITHOUT removing it
 * @tparam T any type
 * @return The first element in queue
 */
template <typename T> T &CircularBuffer<T>::getFront() noexcept
{
    return m_deque.front();
}

/**
 * Remove the first inserted (oldest) element from buffer
 */
template <typename T> void CircularBuffer<T>::removeFirst()
{
    if (!m_deque.empty())
    {
        m_deque.pop_front();
    }
}

/**
 * Add new element to buffer (using copy)
 * @tparam T any type
 * @param item the element to be inserted
 */
template <typename T> void CircularBuffer<T>::addItem(const T &item)
{
    // if full, remove 1st item first
    if (m_deque.size() >= max_capacity)
    {
        m_deque.pop_front();
    }
    m_deque.emplace_back(item);
}

/**
 * Move element into the buffer
 * @tparam T any type
 * @param item the element to be inserted
 */
template <typename T> void CircularBuffer<T>::addItem(T &&item)
{
    if (m_deque.size() >= max_capacity)
    {
        m_deque.pop_front();
    }
    m_deque.emplace_back(std::move(item));
}
} // namespace chk
