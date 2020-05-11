#include "KVStoreImpl.h"
#include "LRUCache.h"
#include "shared.h"
#include "gtest/gtest.h"

#include <leveldb/write_batch.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using namespace celonis::kvstore;

namespace {

std::unique_ptr<leveldb::DB> openDB(const std::string& name)
{
    leveldb::DB* dbPtr = nullptr;
    leveldb::Options options;
    if (leveldb::DB::Open(options, name, &dbPtr).ok()) {
        return std::unique_ptr<leveldb::DB>(dbPtr);
    }
    return nullptr;
}
}

class KVStoreImplTest : public ::testing::Test {
  public:
    KVStoreImplTest()
        : _store(std::make_shared<KVStoreImpl>(
            openDB(DBName), std::make_unique<cache::LRUCache>(CacheSize)))
        , _gen(_rd())
        , _dis(0, DBSize - 1)
    {
        // warm the cache
        auto v = std::string{};
        for (auto i = 0; i < CacheSize; ++i) {
            _store->Get(getRandomString(), v);
        }
    };
    void SetUp(){};
    void TearDown(){};

  protected:
    int getRandomNumber()
    {
        return _dis(_gen);
    };
    std::string getRandomString()
    {
        return std::to_string(getRandomNumber());
    };

  protected:
    std::shared_ptr<KVStoreImpl> _store;
    std::random_device _rd;
    std::mt19937 _gen;
    std::uniform_int_distribution<> _dis;
};

TEST_F(KVStoreImplTest, OneMillionGet)
{
    const auto hardwareConcurrency = std::thread::hardware_concurrency();
    const auto nOperations = std::floor(1000 * 1000 / hardwareConcurrency);
    auto threads = std::list<std::thread>{};
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (auto t = 0; t < hardwareConcurrency; ++t) {
        threads.emplace_back(std::thread([this, nOperations]() {
            // keys are distributed between ["0", "DBSize")
            for (auto i = 0; i < nOperations; ++i) {
                auto value = std::string{};
                EXPECT_TRUE(_store->Get(getRandomString(), value));
            }
        }));
    }
    for (auto& t : threads) {
        t.join();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << hardwareConcurrency * nOperations << " Get operations in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms"
              << std::endl;
    std::cout << hardwareConcurrency * nOperations
            / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000
              << " ops/sec" << std::endl;
}

TEST_F(KVStoreImplTest, OneMillionPut)
{
    const auto hardwareConcurrency = std::thread::hardware_concurrency();
    const auto nOperations = std::floor(1000 * 1000 / hardwareConcurrency);
    auto threads = std::list<std::thread>{};
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (auto t = 0; t < hardwareConcurrency; ++t) {
        threads.emplace_back(std::thread([this, nOperations]() {
            // keys are distributed between ["0", "DBSize")
            for (auto i = 0; i < nOperations; ++i) {
                const auto key = getRandomString();
                const auto value = getRandomString();
                _store->Put(key, value);
            }
        }));
    }
    for (auto& t : threads) {
        t.join();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << hardwareConcurrency * nOperations << " Put operations in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms"
              << std::endl;
    std::cout << hardwareConcurrency * nOperations
            / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000
              << " ops/sec" << std::endl;
}

TEST_F(KVStoreImplTest, OneMillionDelete)
{
    const auto hardwareConcurrency = std::thread::hardware_concurrency();
    const auto nOperations = std::floor(1000 * 1000 / hardwareConcurrency);
    auto threads = std::list<std::thread>{};
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (auto t = 0; t < hardwareConcurrency; ++t) {
        threads.emplace_back(std::thread([this, nOperations]() {
            // keys are distributed between ["0", "DBSize")
            for (auto i = 0; i < nOperations; ++i) {
                _store->Delete(getRandomString());
            }
        }));
    }
    for (auto& t : threads) {
        t.join();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << hardwareConcurrency * nOperations << " Delete operations in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms"
              << std::endl;
    std::cout << hardwareConcurrency * nOperations
            / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000
              << " ops/sec" << std::endl;
}

TEST_F(KVStoreImplTest, OneMillionRandomOperation)
{
    const auto hardwareConcurrency = std::thread::hardware_concurrency();
    const auto nOperations = std::floor(1000 * 1000 / hardwareConcurrency);
    auto threads = std::list<std::thread>{};
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (auto t = 0; t < hardwareConcurrency; ++t) {
        threads.emplace_back(std::thread([this, nOperations]() {
            auto value = std::string{};
            // keys are distributed between ["0", "DBSize")
            for (auto i = 0; i < nOperations; ++i) {
                switch (getRandomNumber() % 3) {
                case 0:
                    // GET
                    _store->Get(getRandomString(), value);
                    break;
                case 1:
                    // PUT
                    _store->Put(getRandomString(), getRandomString());
                    break;
                case 2:
                    // DELETE
                    _store->Delete(getRandomString());
                    break;
                default:
                    break;
                }
            }
        }));
    }
    for (auto& t : threads) {
        t.join();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << hardwareConcurrency * nOperations << " Random operations in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms"
              << std::endl;
    std::cout << hardwareConcurrency * nOperations
            / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000
              << " ops/sec" << std::endl;
}
