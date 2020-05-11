#pragma once

#include "KVStore.h"

#include <leveldb/db.h>

#include <memory>
#include <mutex>

namespace celonis {
namespace kvstore {

class KVStoreImpl final : public KVStore {

  public:
    static std::shared_ptr<KVStore> create(const std::string& filename, bool create_if_missing);

    KVStoreImpl(std::unique_ptr<leveldb::DB> db, std::unique_ptr<KVStore> cache);
    ~KVStoreImpl() = default;

    bool Get(const std::string& key, std::string& value);
    void Put(const std::string& key, const std::string& value);
    void Delete(const std::string& key);

  private:
    std::unique_ptr<leveldb::DB> _db;
    std::unique_ptr<KVStore> _cache;
    std::mutex _mtx;
};

}
}
