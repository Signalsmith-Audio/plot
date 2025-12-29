cmake_minimum_required(VERSION 3.24)

add_library(signalsmith-plot INTERFACE)
target_include_directories(signalsmith-plot INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
