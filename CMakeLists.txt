cmake_minimum_required(VERSION 3.20)
project(Xqqyt_UserServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ThreadPool)
add_subdirectory(MsgHandler)
add_subdirectory(MySqlConnPool)
add_subdirectory(TcpServer)

# 设置源码路径（确保包含所有.cpp文件）
set(SRC_PATH
    main.cpp
    Server.cpp
)

# 设置头文件路径
set(INCLUDE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 包含目录
include_directories(
    ${INCLUDE_PATH}
    /usr/include/x86_64-linux-gnu
)


# 可执行文件
add_executable(${PROJECT_NAME} ${SRC_PATH})

# 链接库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MsgHandler
    TcpServer
    pthread
)