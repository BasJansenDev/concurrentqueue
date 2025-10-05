---
layout: post
title: "Part 1: Setting up requirements and a testing framework"
---

Starting off with the basics, there are a few key components required to create a concurrent queue that will help us build a foundation for later improvements. The name itself already gives us a great hint at what we’re trying to achieve:

- Speed
- Lock-free
- Concurrent
- Queue

Even though we’re not tackling all of these aspects just yet, we can still set up a framework that allows us to prove that what we’re building meets each requirement as we go. We’ll do this using GTest, where we’ll create multiple tests to assert different use cases. For example, we’ll want to verify that:
- Our queue behaves like a proper queue, meaning items come out in the same order they were inserted, regardless of the number of producers or consumers.
- We allow concurrency: multiple producers and consumers can interact with the queue without race conditions or data duplication.

These are just the correctness basics of a concurrent queue. But correctness alone doesn’t make it fast.

Speed and being lock-free go hand in hand. When a concurrent queue relies on locks, it loses a lot of potential performance. Every thread has to wait for others to finish before proceeding. A natural first optimization idea would be to separate reader and writer locks, since producers and consumers operate on opposite ends of the queue and shouldn’t block each other unnecessarily.

Later on, we’ll explore other mechanisms to improve speed, such as ring buffers, atomic variables, and other lock-free techniques. In all these cases, we’ll want a way to measure our progress. Something that shows a provable performance gain in exchange for our efforts. That’s where our benchmarking framework comes in. It will measure how long our queue takes to handle specific workloads under varying levels of concurrency with different numbers of producers and consumers.

So, let’s dive in.

## Setting up the framework

First, we're going to need a component that can work as a Producer and a Consumer. Let's for now call it a Worker. We'll define a simple interface as follows:

```
template<typename T>
class Worker {
    public:
        Worker(std::shared_ptr<ConcurrentQueue<T>> _queue)
        void init(std::vector<T> _vector)
        void pushAll()
        void readAll()
};
```

Where the Worker class can be created with a `shared_ptr` to our Concurrent Queue. The Worker will be a templated class, so we have dynamic possibilities for data types to produce and consume. It comes with three simple functions. An `init`, a `pushAll` and a `readAll`. The init is there so we can load the worker with to-be-produced values in a vector. Allowing us to on the fly keep adding new values. The `pushAll` function then allows us to pull the trigger on these stored values, that they're all written into the queue, in order. Lastly, `readAll` will keep reading from the queue until the queue is empty. We may need more granular control over the pushing and reading later, but for now this will serve our needs.

We can now expand this into a simple set of tests. One for single producer, single consumer (SPSC), and one for multiple producers, multiple consumers (MPMC). 
First, we want to create a vector with values. Here, we create a vector of integers from 1 to 1e8. Then, we create our ConcurrentQueue templated to integers, initialize our worker with the loaded vector, and we're good to go.
```
    std::vector<int> loadedVector{};
    for(int i = 0; i < 1e8; i++){
        loadedVector.emplace_back(i);
    }
    auto sut = std::make_shared<ConcurrentQueue<int>>();
    Worker<int> worker{sut};
    worker.init(loadedVector);
```
Then, we're letting the worker push all these values into the queue, and let it read all of these back. Making it both a single Producer and a single Consumer.

```
    worker.pushAll();
    worker.readAll();
```

That's the Production and Consumption dealt with. However, we're now going to need some ways where we prove both the correctness of the queue, along with a printout of our speed. Since we're working with a simple int vector here, with a single producer and consumer, so we can add in the following assertion:

```
    ASSERT_EQ(worker.readVector, loadedVector);
```
Where readVector is a member variable of our Worker, into which it writes the read values. For our speed we're taking a simple approach for now, by just using `std::chrono::high_resolution_clock`. This will serve for now since our implementation is not that speedy yet, but later we'll look into using tooling such as `perf`, since the high_resolution_clock is easily affected by other processes going on in the background of our operating systems. The final test that we then have is:

```
TEST_F(ConcurrentQueueTestSuite, BenchmarkSingleThreadTest) {
    std::vector<int> loadedVector{};
    for(int i = 0; i < 1e8; i++){
        loadedVector.emplace_back(i);
    }
    auto sut = std::make_shared<ConcurrentQueue<int>>();
    Worker<int> worker{sut};
    worker.init(loadedVector);
    auto pre = std::chrono::high_resolution_clock::now();
    worker.pushAll();
    worker.readAll();
    auto aft = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration(aft - pre) << "\n";
    ASSERT_EQ(worker.readVector, loadedVector);
}
```

This crosses off our checks for measurable speed and correctness, but not yet for concurrency, since reading and writing doesn't happen in parallel. To this end we create the following test:

```
TEST_F(ConcurrentQueueTestSuite, BenchmarkParallelReadWriteTest) {
    std::vector<int> loadedVector{};
    for(int i = 0; i < 1e8; i++){
        loadedVector.emplace_back(i);
    }
    auto sut = std::make_shared<ConcurrentQueue<int>>();
    Worker<int> worker{sut};
    worker.init(loadedVector);
    auto pre = std::chrono::high_resolution_clock::now();
    auto writerThread = std::thread([&](){worker.pushAll();});
    auto readerThread = std::thread([&](){worker.readAll();});
    writerThread.join();
    readerThread.join();
    auto aft = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration(aft - pre) << "\n";
    ASSERT_EQ(worker.readVector, loadedVector);
}
```
Where we wrap production and consumption in two separate threads for it to occur simultaneously. Now we have concurrency checked off too, but only for SPSC. We can expand on the test above by just adding in more threads:

```
TEST_F(ConcurrentQueueTestSuite, BenchmarkMultiThreadTest) {
    std::vector<Worker<int>> workerVector{};
    auto sut = std::make_shared<ConcurrentQueue<int>>();
    std::vector<int> loadedVector{};
    for(int i = 0; i < 1e7; i++){
        loadedVector.emplace_back(i);
    }

    for(int i = 0; i < 10; i++){
        auto worker = Worker<int>(sut);
        worker.init(loadedVector);
        worker.pushAll();
        workerVector.emplace_back(std::move(worker));
    }
    auto pre = std::chrono::system_clock::now();
    std::vector<std::thread> threadVector;
    for(auto& worker : workerVector) {
        threadVector.emplace_back(std::thread([&worker](){worker.readAll();}));
    }
    for(auto& thread : threadVector) {
        thread.join();
    }
    auto aft = std::chrono::system_clock::now();
    std::cout << std::chrono::duration(aft - pre) << "\n";
}
```

And there we have it. A basic testing framework that allows us to test for speed, correctness and concurrency. Now, we can start looking into actually building a queue!

