add_library(TcpServer STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TcpServer.cpp
)

target_include_directories(TcpServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(TcpServer
    pthread
    ThreadPool
    MySqlConnPool
    OpensslHandler
)