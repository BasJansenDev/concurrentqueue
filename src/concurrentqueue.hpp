template<typename T>
class ConcurrentQueue {
public:
    const T& wait_and_pop() {

    }
    const T& try_pop() {

    }
    void push(const T&) {
        
    }
private:
    std::deque<T> queue;
};