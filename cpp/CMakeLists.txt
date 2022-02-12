cmake_minimum_required(VERSION 3.1...3.16.8 FATAL_ERROR)

find_package(spdlog REQUIRED)

add_executable( fix_client "./src/fix_client.cc" )

set_target_properties( fix_client
                       PROPERTIES
                       CXX_STANDARD 20
                       CXX_EXTENSIONS OFF
                       CXX_STANDARD_REQUIRED ON
                       CXX_POSITION_INDEPENDENT_CODE ON )

target_include_directories( fix_client
                            PUBLIC
                            "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries( fix_client
                       PUBLIC
                       spdlog::spdlog
                       quickfix
                       tcmalloc )


add_executable( fix_server "./src/fix_server.cc" )

set_target_properties( fix_server
                       PROPERTIES
                       CXX_STANDARD 20
                       CXX_EXTENSIONS OFF
                       CXX_STANDARD_REQUIRED ON
                       CXX_POSITION_INDEPENDENT_CODE ON )

target_include_directories( fix_server
                            PUBLIC
                            "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries( fix_server
                       PUBLIC
                       spdlog::spdlog
                       quickfix
                       tcmalloc )


add_executable( fix_util "./src/fix_util.cc" )

set_target_properties( fix_util
                       PROPERTIES
                       CXX_STANDARD 20
                       CXX_EXTENSIONS OFF
                       CXX_STANDARD_REQUIRED ON
                       CXX_POSITION_INDEPENDENT_CODE ON )

target_include_directories( fix_util
                            PUBLIC
                            "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries( fix_util
                       PUBLIC
                       spdlog::spdlog
                       quickfix
                       tcmalloc )
