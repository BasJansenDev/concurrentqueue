---
layout: post
title: "Part 2: A not so fast, not so lock-free, yet concurrent queue."
---

Now, it is time for us to look at a first implementation of our queue. First, a basic interface:
```
template<typename T>
class ConcurrentQueue {
public:
    const T wait_and_pop();
    std::optional<T> try_pop();
    void push(const T& val) ;
}
```

Where we have our templated ConcurrentQueue, allowing it to be reused for any type. Our three defined functions are `const T wait_and_pop()`, `std::optional<T> try_pop()` and `void push(const T& val)`. A function that allows a consumer to wait for new data to come in, and when it does, it pops it, in a blocking way. One function that attempts a non-blocking pop. If the queue is empty, a `std::nullopt` is returned. Lastly, our `push` function allows a producer to push data onto the queue.

## First design choices
These signatures are chosen deliberately with performance in mind. `const T` XXXX PLACEHOLDER FOR PERFORMANCE XXXX. `std::optional` is chosen since we expect that it will occur rather regularly that the queue is empty. Other options would have been returning a boolean and passing an out reference, like `bool try_pop(T& out)`. This is more barebones than using `std::optional`, but the performance gain is not much beyond the construction of the optional object XXXX ADD MORE INFO ABOUT OPTIONALS XXXX. Lastly, exceptions could also be used here, shaving off the need of an extra boolean. Our signature would then look like `void try_pop(T& out)` or `const T try_pop()`. The gain here is one cycle, which can stack up over millions of pop attempts per second, however the tradeoff is the price of an exception when it does occur. A stack unwind, building up a stack trace, and then reporting this exception (which in turn needs to be caught by the producer) is a lot of extra cost in performance, namely thousands of cycles, along with (in my opinion) ugly `try-catch` blocks. If we knew for certain that our queue would always have data in it, that is, the amount of data that is produced per timeframe will always be equal to the amount of data that is consumed per timeframe, it would make sense to simply go for this exception and take the hit whenever it arises. This brings us to a crossroads. What exactly will the intention of this queue be? Do we want it to have a very fast average case, meaning we'd rather have it run a million times without a returned optional, and when the eventual exception hits, we don't care much since it doesn't budge our average too much? Or do we want to have a reliable, low-latency queue, where our average is a little higher than the other option, but our worst case is basically equal to our average case. For this project, the decision is made to aim for a low-latency queue, hence the path for exceptions is not taken. It just shows that in software engineering there's never a silver bullet, and a solution should be built with the problem that is to be solved in mind.

## First naive implementation
Enough philosophy (for now), time to dive into an actual implementation. Again, we're not aiming for anything fast or perfect here yet, we just want to have an MVP upon which we can keep building. Our `wait_and_pop` looks as follows:

```
    const T wait_and_pop() {
        std::unique_lock<std::mutex> lk(queueMutex);
        cv.wait(lk, [this]{return !queue.empty();});
        auto val = queue.back();
        queue.pop_back();
        return val;
    }
```