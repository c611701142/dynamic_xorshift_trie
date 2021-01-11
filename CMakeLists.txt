cmake_minimum_required(VERSION 3.13)
project(dynamic_xorshift_trie)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall")

add_executable(test test_Xorshift_trie.cpp)
