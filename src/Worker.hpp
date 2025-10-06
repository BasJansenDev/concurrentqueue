#include <vector>
#include "concurrentqueue.hpp"

template<typename T>
class Worker {
    public:
        Worker(std::shared_ptr<ConcurrentQueue<T>> _queue) {
            queue = _queue;
        }

        void init(std::vector<T> _vector) {
            writeVector = _vector;
        }

        void pushAll() {
            for(auto i : writeVector){
                queue->push(i);
            }
        }

        void readAll() {
            while(const auto& v = queue->try_pop() != std::nullopt){
                readVector.emplace_back(v);
            };
        }
        
        std::vector<T> writeVector;
        std::vector<T> readVector;
    private:
        std::shared_ptr<ConcurrentQueue<T>> queue;
        std::thread thread{};
};