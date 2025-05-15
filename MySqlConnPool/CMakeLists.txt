add_library(MySqlConnPool STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MySqlConnPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MySqlDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SqlConnSet.cpp
)

target_include_directories(MySqlConnPool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(MySqlConnPool
    mysqlcppconn
    pthread
)