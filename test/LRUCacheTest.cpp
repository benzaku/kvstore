#include "gtest/gtest.h"
#include <LRUCache.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>

using namespace celonis::kvstore::cache;

class LRUCacheTest : public ::testing::Test {
  public:
    LRUCacheTest()
        : _cache(std::make_shared<LRUCache>(1024))
    {
        std::srand(std::time(nullptr));
        for (int i = 0; i < 1024; ++i) {
            _cache->Put(std::to_string(i), std::to_string(i * i));
        }
    };
    void SetUp(){};
    void TearDown(){};

  protected:
    std::shared_ptr<LRUCache> _cache;
};

TEST_F(LRUCacheTest, Get)
{
    for (int i = 0; i < 1024; ++i) {
        auto value = std::string{};
        if (_cache->Get(std::to_string(i), value)) {
            EXPECT_EQ(std::to_string(i * i), value);
        }
    }

    auto v = std::string{};
    EXPECT_FALSE(_cache->Get(std::to_string(1024), v));
}

TEST_F(LRUCacheTest, Put)
{
    _cache->Put(std::to_string(1024), std::to_string(1024 * 1024));
    auto v = std::string{};
    EXPECT_FALSE(_cache->Get("0", v));
}

TEST_F(LRUCacheTest, Delete)
{
    _cache->Delete("0");
    auto v = std::string{};
    EXPECT_FALSE(_cache->Get("0", v));
}

TEST_F(LRUCacheTest, Throughput)
{
    const auto nOperations = 20 * 1000;
    auto cache = _cache;
    auto randomOp = [&cache]() {
        auto v = std::string{};
        for (auto i = 0; i < nOperations; ++i) {
            const auto k = std::rand() % 1024;
            switch (std::rand() % 3) {
            case 0:
                // Get
                cache->Get(std::to_string(k), v);
                break;
            case 1:
                // Put
                cache->Put(std::to_string(k), std::to_string(k * k));
                break;
            case 2:
                // Delete
                cache->Delete(std::to_string(k));
                break;
            default:
                break;
            }
        }
    };

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    randomOp();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << nOperations << " operations in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms"
              << std::endl;
    std::cout << nOperations
            / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000
              << " Random ops/sec" << std::endl;
}
