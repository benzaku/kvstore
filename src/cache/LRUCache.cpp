#include "LRUCache.h"

#include <iostream>

namespace celonis {
namespace kvstore {
namespace cache {

LRUCache::LRUCache(int size)
    : _size(size)
{
    _keyValueMap.reserve(_size);
}

bool LRUCache::Get(const std::string& key, std::string& value)
{
    auto found = _keyValueMap.find(key);
    if (found == _keyValueMap.end()) {
        // Not found
        return false;
    }

    value = found->second.first;
    if (found->second.second == _freqList.begin()) {
        // no need to update the queue
        return true;
    }

    _freqList.erase(_keyValueMap[key].second);
    auto iter = _freqList.insert(_freqList.begin(), key);
    _keyValueMap[key]
        = std::pair<std::string, std::list<std::string>::iterator>(_keyValueMap[key].first, iter);
    return true;
}

void LRUCache::Put(const std::string& key, const std::string& value)
{
    if (_keyValueMap.find(key) != _keyValueMap.end()) {
        update(key, value);
    } else {
        insert(key, value);
    }
}

void LRUCache::Delete(const std::string& key)
{
    auto found = _keyValueMap.find(key);
    if (found != _keyValueMap.end()) {
        _freqList.erase(found->second.second);
        _keyValueMap.erase(key);
    }
}

void LRUCache::update(const std::string& key, const std::string& value)
{
    auto valueIterPair = _keyValueMap[key];
    if (valueIterPair.second == _freqList.begin()) {
        // no need to update queue
        _keyValueMap[key]
            = std::pair<std::string, std::list<std::string>::iterator>(value, valueIterPair.second);
        return;
    }
    // update queue
    _freqList.erase(valueIterPair.second);
    auto iter = _freqList.insert(_freqList.begin(), key);
    _keyValueMap[key] = std::pair<std::string, std::list<std::string>::iterator>(value, iter);
}

void LRUCache::insert(const std::string& key, const std::string& value)
{
    if (_keyValueMap.size() < _size) {
        // No remove
        auto iter = _freqList.insert(_freqList.begin(), key);
        _keyValueMap[key] = std::pair<std::string, std::list<std::string>::iterator>{ value, iter };
    } else {
        // with remove
        removeLeastRecentlyUsed();
        insert(key, value);
    }
}
void LRUCache::removeLeastRecentlyUsed()
{
    auto keyToRemove = _freqList.back();
    _keyValueMap.erase(keyToRemove);
    _freqList.pop_back();
}

}
}
}
