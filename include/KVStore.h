#pragma once

#include <string>

namespace celonis {
namespace kvstore {

class KVStore {
  public:
    virtual ~KVStore() = default;

    virtual bool Get(const std::string& key, std::string& value) = 0;
    virtual void Put(const std::string& key, const std::string& value) = 0;
    virtual void Delete(const std::string& key) = 0;
};

}
}
