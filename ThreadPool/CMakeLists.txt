add_library(ThreadPool STATIC
    ThreadPool.cpp  # 空的
    ThreadPool.hpp
    PriorityQueue.hpp
    Queue.h
    ThreadQueue.hpp
)

target_include_directories(ThreadPool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ThreadPool
    pthread
)
