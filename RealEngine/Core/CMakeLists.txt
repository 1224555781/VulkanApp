cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
cmake_policy(VERSION 3.2)


file(GLOB_RECURSE CORE_SOURCE "./Private/*.cpp")
file(GLOB_RECURSE CORE_HEADER "./Public/*.h")


source_group("CORE Files" FILES ${CORE_SOURCE})
source_group("CORE Files" FILES ${CORE_HEADER})