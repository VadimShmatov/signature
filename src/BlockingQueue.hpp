#pragma once
#include <boost/log/trivial.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename Data>
class BlockingQueue {
private:

    std::queue<Data> queue;
    mutable std::mutex queue_mutex;
    const size_t queue_limit;
    const float watermark;

    size_t number_of_writers = 0;
    bool is_closed = false;
    bool is_overflown = false;
    bool is_empty = false;

    std::condition_variable new_item_or_closed_event;
    std::condition_variable item_removed_event;

public:
    BlockingQueue(size_t size_limit, float watermark = 0.25) : queue_limit(size_limit), watermark(watermark)
    {}

    void start_writing()
    {
        std::unique_lock lock(queue_mutex);
        number_of_writers++;
    }

    void stop_writing()
    {
        std::unique_lock lock(queue_mutex);
        if (--number_of_writers == 0) {
            is_closed = true;
            new_item_or_closed_event.notify_all();
        }
    }

    void push(Data&& data)
    {
        std::unique_lock lock(queue_mutex);
        if (queue_limit > 0) {
            while (queue.size() >= queue_limit) {
                is_overflown = true;
                //BOOST_LOG_TRIVIAL(warning) << "Queue " << typeid(Data).name() << ": is_overflown = true [" << queue.size() << "/" << queue_limit << "]";
                item_removed_event.wait(lock);
            }
        }
        queue.push(std::forward<Data>(data));
        if (is_empty) {
            if (queue.size() >= (1.0 - watermark) * queue_limit) {
                is_empty = false;
                //BOOST_LOG_TRIVIAL(warning) << "Queue " << typeid(Data).name() << ": is_empty = false [" << queue.size() << "/" << queue_limit << "]";
                new_item_or_closed_event.notify_all();
            }
        }
        else {
            new_item_or_closed_event.notify_one();
        }
    }

    bool pop(Data& popped_value)
    {
        std::unique_lock lock(queue_mutex);
        while (queue.empty()) {
            if (queue.empty() && is_closed) {
                return false;
            }
            is_empty = true;
            //BOOST_LOG_TRIVIAL(warning) << "Queue " << typeid(Data).name() << ": is_empty = true [" << queue.size() << "/" << queue_limit << "]";
            new_item_or_closed_event.wait(lock);
        }
        popped_value = std::move(queue.front());
        queue.pop();
        if (is_overflown) {
            if (queue.size() <= watermark * queue_limit) {
                is_overflown = false;
                //BOOST_LOG_TRIVIAL(warning) << "Queue " << typeid(Data).name() << ": is_overflown = false [" << queue.size() << "/" << queue_limit << "]";
                item_removed_event.notify_all();
            }
        }
        else {
            item_removed_event.notify_one();
        }
        return true;
    }

    const size_t get_size() const
    {
        return queue.size();
    }

    const bool get_closed() const
    {
        return is_closed;
    }
};