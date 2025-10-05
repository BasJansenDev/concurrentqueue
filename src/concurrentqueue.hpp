#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

template<typename T>
class ConcurrentQueue {
public:
    const T wait_and_pop() {
        std::unique_lock<std::mutex> lk(queueMutex);
        cv.wait(lk, [this]{return !queue.empty();});
        auto val = queue.back();
        queue.pop_back();
        return val;
    }
    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lk(queueMutex);
        if(not queue.empty()){
            auto val = queue.back();
            queue.pop_back();
            return val;
        } else {
            return std::nullopt;
        }

    }
    void push(const T& val) {
        std::unique_lock<std::mutex> lk(queueMutex);
        queue.push_front(val);
    }


private:
    std::deque<T> queue{};
    std::mutex queueMutex;
    std::condition_variable cv;
};