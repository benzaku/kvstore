#include "shared.h"
#include "gtest/gtest.h"

#include <leveldb/db.h>
#include <leveldb/options.h>
#include <leveldb/write_batch.h>

#include <memory>

namespace {

std::unique_ptr<leveldb::DB> createRandomDBIfNotExists()
{
    leveldb::DB* dbPtr = nullptr;
    leveldb::Options options;
    leveldb::DestroyDB(DBName, options);
    auto status = leveldb::DB::Open(options, DBName, &dbPtr);
    if (!status.ok()) {
        options.create_if_missing = true;
        if (leveldb::DB::Open(options, DBName, &dbPtr).ok()) {
            auto batch = leveldb::WriteBatch{};
            for (auto i = 0; i < DBSize; ++i) {
                batch.Put(std::to_string(i), std::to_string(i * i));
            }
            auto options = leveldb::WriteOptions{};
            dbPtr->Write(options, &batch);
        }
    }
    return std::unique_ptr<leveldb::DB>(dbPtr);
}
}

int main(int argc, char** argv)
{
    createRandomDBIfNotExists();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
