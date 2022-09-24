cmake_minimum_required(VERSION 3.23)
project(ISJ_RE_22_14571)

set(CMAKE_CXX_STANDARD 14)
#
#add_executable(server server.cpp)
#target_link_libraries(server pbc gmp)


add_executable(data_owner data_owner.cpp)
target_link_libraries(data_owner pbc gmp ssl crypto)

#target_include_directories(server PUBLIC include/pbc)



