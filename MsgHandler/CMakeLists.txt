add_library(MsgHandler STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/JsonEncoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/JsonHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MsgDecoder.cpp
)

target_include_directories(MsgHandler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)