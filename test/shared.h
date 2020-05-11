#pragma once

#include <list>
#include <string>

constexpr auto DBName = "./test_database.dat";
constexpr auto DBSize = 1 * 1000 * 1000; // 1 million entry
constexpr auto CacheSize = DBSize; // 20% of the size of the database.
