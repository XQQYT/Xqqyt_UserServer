add_library(OpensslHandler STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/OpensslHandler.cpp
)

target_include_directories(OpensslHandler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(OpensslHandler
    ssl 
    crypto
    sodium
)