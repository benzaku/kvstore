set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIR})
set(HEADERS
     ${INCLUDE_DIR}/KVStore.h
)   
set(SOURCES
    KVStoreImpl.h
    KVStoreImpl.cpp
    cache/LRUCache.h
    cache/LRUCache.cpp
)

add_library(kvstore
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(kvstore PUBLIC leveldb)

set_target_properties(kvstore PROPERTIES LINKER_LANGUAGE CXX)
