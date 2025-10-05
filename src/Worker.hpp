#include <vector>
#include "concurrentqueue.hpp"

template<typename T>
class Worker {
    public:
        Worker(std::shared_ptr<ConcurrentQueue<T>> _queue) {
            queue = _queue;
        }

        void init(std::vector<T> _vector) {
            vector = _vector;
        }

        void pushAll() {
            for(auto i : vector){
                queue->push(i);
            }
        }

        void readAll() {
            while(const auto& v = queue->try_pop() != std::nullopt){};
        }
    private:
        std::vector<T> vector;
        std::shared_ptr<ConcurrentQueue<T>> queue;
        std::thread thread{};
};