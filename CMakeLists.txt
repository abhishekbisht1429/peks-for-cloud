cmake_minimum_required(VERSION 3.23)
project(ISJ_RE_22_14571)

set(CMAKE_CXX_STANDARD 14)



add_executable(socket tb_util/socket.cpp)
add_executable(http tb_util/http.cpp data_owner.cpp data_consumer.cpp)
add_executable(server server.cpp)

#target_include_directories(server PUBLIC include/pbc)
target_link_libraries(server pbc gmp)

