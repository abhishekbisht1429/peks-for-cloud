cmake_minimum_required(VERSION 3.23)
include(FetchContent)
project(ISJ_RE_22_14571)

set(CMAKE_CXX_STANDARD 14)

FetchContent_Declare(
        http
        GIT_REPOSITORY https://github.com/abhishekbisht1429/http_protocol_cpp.git
)
FetchContent_MakeAvailable(http)

add_library(proj_util util.cpp)
target_include_directories(proj_util PUBLIC ${http_SOURCE_DIR}/include)
target_link_libraries(proj_util pbc gmp ssl crypto)

add_executable(data_owner data_owner.cpp)
target_include_directories(data_owner PUBLIC ${http_SOURCE_DIR}/include)
target_link_libraries(data_owner pbc gmp proj_util http)

add_executable(server server.cpp)
target_link_libraries(server pbc gmp proj_util http botan-2)
target_include_directories(server PUBLIC ${http_SOURCE_DIR}/include)

add_executable(data_consumer data_consumer.cpp)
target_include_directories(data_consumer PUBLIC ${http_SOURCE_DIR}/include)
target_link_libraries(data_consumer pbc gmp proj_util http)




