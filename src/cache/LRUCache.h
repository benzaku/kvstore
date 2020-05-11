#pragma once

#include "KVStore.h"

#include <atomic>
#include <list>
#include <mutex>
#include <unordered_map>

namespace celonis {
namespace kvstore {
namespace cache {

/// Non thread safety LRU Cache.
class LRUCache : public KVStore {
  public:
    LRUCache(int size);
    virtual ~LRUCache() = default;

    bool Get(const std::string& key, std::string& value) override;
    void Put(const std::string& key, const std::string& value) override;
    void Delete(const std::string& key) override;

  private:
    void update(const std::string& key, const std::string& value);
    void insert(const std::string& key, const std::string& value);
    void removeLeastRecentlyUsed();

  private:
    const int _size;
    std::list<std::string> _freqList;
    std::unordered_map<
        std::string,
        std::pair<std::string /*value*/, std::list<std::string>::iterator>>
        _keyValueMap;
};
}
}
}
