#include <gtest/gtest.h>
#include <chrono>
#include "../src/concurrentqueue.hpp"
#include "../src/Worker.hpp"

class ConcurrentQueueTestSuite : public testing::Test {
    public:
    ConcurrentQueueTestSuite() = default;

};

TEST_F(ConcurrentQueueTestSuite, IntTest) {
    int test = 10;
    ConcurrentQueue<int> intSut{};
    intSut.push(test);
    auto ret = intSut.try_pop();
    ASSERT_EQ(test,ret);
}

TEST_F(ConcurrentQueueTestSuite, IntAndWaitTest) {
    int test = 10;
    ConcurrentQueue<int> intSut{};
    intSut.push(test);
    auto ret = intSut.wait_and_pop();
    ASSERT_EQ(test,ret);
}

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

