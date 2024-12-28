#pragma once

#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>

template<typename T>
class SafetyQueue {
private:
    std::list<std::shared_ptr<T>> m_queue;
    std::mutex m_mutex;
    std::condition_variable_any m_notEmpty;
    std::condition_variable_any m_notFull;
    unsigned int m_maxSize;

private:

    bool isEmpty() const {
        return m_queue.empty();
    }

    bool isFull() const {
        return m_queue.size() == m_maxSize;
    }

public:
    SafetyQueue(unsigned int maxSize = 25) {
        this->m_maxSize = maxSize;
    }

    int queuecount() const {
        return m_queue.size();
    }

    void setMaxSize(unsigned int maxSize) {
        this->m_maxSize = maxSize;
    }

    unsigned int getMaxSize() {
        return this->m_maxSize;
    }

    unsigned int getCurrentSize() {
        return m_queue.size();
    }

    ~SafetyQueue(){}

    void product(const std::shared_ptr<T>& v) {
        std::unique_lock<std::mutex> locker(m_mutex);
        while(isFull()) {
            m_notFull.wait(m_mutex);
        }

        m_queue.push_back(v);
        m_notEmpty.notify_one();
    }
    void consumption(std::shared_ptr<T>& v) {
        std::unique_lock<std::mutex> locker(m_mutex);
        while(isEmpty()) {
            m_notEmpty.wait(m_mutex);
        }

        v = m_queue.front();
        m_queue.pop_front();
        m_notFull.notify_one();
    }

    std::string debug_info;
};
