cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
cmake_policy(VERSION 3.2)


file(GLOB_RECURSE SOURCE_SOURCE "./Private/*.cpp")
file(GLOB_RECURSE SOURCE_HEADER "./Public/*.h")


source_group("Source Files" FILES ${SOURCE_SOURCE})
source_group("Source Files" FILES ${SOURCE_HEADER})