#include "KVStoreImpl.h"
#include "cache/LRUCache.h"

#include <iostream>

namespace celonis {
namespace kvstore {

namespace {
constexpr auto DefaultMaxCacheEntries = 10 * 1024;
}

std::shared_ptr<KVStore> KVStoreImpl::create(const std::string& filename, bool create_if_missing)
{
    leveldb::DB* dbPtr;
    leveldb::Options options;
    options.create_if_missing = create_if_missing;
    const auto status = leveldb::DB::Open(options, filename, &dbPtr);
    if (status.ok()) {
        const auto cacheSize = DefaultMaxCacheEntries;
        std::unique_ptr<KVStore> cache = std::make_unique<cache::LRUCache>(cacheSize);
        // preload cache
        auto it = dbPtr->NewIterator(leveldb::ReadOptions());
        auto count = 0;
        for (it->SeekToFirst(); it->Valid() && count < cacheSize; it->Next(), ++count) {
            const auto key = it->key().ToString();
            const auto value = it->value().ToString();
            cache->Put(key, value);
        }
        delete it;
        return std::shared_ptr<KVStoreImpl>(
            new KVStoreImpl(std::unique_ptr<leveldb::DB>(dbPtr), std::move(cache)));
    } else {
        std::cerr << "Unable to open/create database " << filename << std::endl;
        std::cerr << status.ToString() << std::endl;
        return nullptr;
    }
}

KVStoreImpl::KVStoreImpl(std::unique_ptr<leveldb::DB> db, std::unique_ptr<KVStore> cache)
    : _db(std::move(db))
    , _cache(std::move(cache))
{
    assert(_db);
    assert(_cache);
}

bool KVStoreImpl::Get(const std::string& key, std::string& value)
{
    std::lock_guard<std::mutex> lock(_mtx);
    // check cache first
    if (!_cache->Get(key, value)) {
        // cache miss
        auto readOption = leveldb::ReadOptions{};
        auto v = std::string{};
        if (_db->Get(readOption, key, &v).ok()) {
            value = v;
            // fill cache
            _cache->Put(key, v);
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void KVStoreImpl::Put(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto cachedValue = std::string{};
    if (_cache->Get(key, cachedValue) && cachedValue == value) {
        // do nothing
        return;
    }
    auto writeOption = leveldb::WriteOptions{};
    // writeOption.sync = true;
    _db->Put(writeOption, key, value);
    _cache->Put(key, value);
}
void KVStoreImpl::Delete(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mtx);
    auto writeOption = leveldb::WriteOptions{};
    // writeOption.sync = true;
    _db->Delete(writeOption, key);
    _cache->Delete(key);
}

}
}
