add_library(MsgHandler STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/JsonEncoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/JsonStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MsgDecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/BinaryStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MsgBuilder.cpp
)

find_package(ZLIB REQUIRED)

target_include_directories(MsgHandler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)
target_link_libraries(MsgHandler
    MySqlConnPool
    TcpServer
    ZLIB::ZLIB

)