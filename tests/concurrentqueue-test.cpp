#include <gtest/gtest.h>
#include "../src/concurrentqueue.hpp"

class ConcurrentQueueTestSuite : public testing::Test {
    public:
    ConcurrentQueueTestSuite() = default;

};

TEST_F(ConcurrentQueueTestSuite, InitTest) {
    int test = 10;
    ConcurrentQueue<int> intSut{};
    intSut.push(test);
    auto ret = intSut.try_pop();
    ASSERT_EQ(test,ret);
}

TEST_F(ConcurrentQueueTestSuite, InitTest) {
    int test = 10;
    ConcurrentQueue<int> intSut{};
    intSut.push(test);
    auto ret = intSut.wait_and_pop();
    ASSERT_EQ(test,ret);
}


