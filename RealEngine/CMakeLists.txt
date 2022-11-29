# CMakeList.txt : CMake project for RealEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE CORE_SOURCE "./Core/Private/*.cpp")
file(GLOB_RECURSE CORE_HEADER "./Core/Public/*.h")
# Add source to this project's executable.
add_executable (RealEngine "Windows/WindowsMain.cpp"  ${CORE_SOURCE} ${CORE_HEADER})

project(RealEngine)

#target_link_libraries(RealEngine ${CMAKE_CURRENT_SOURCE_DIR} vulkan-1.lib)
add_subdirectory(glm)
add_subdirectory(Core)
add_subdirectory(GLFW)



add_library(VULKAN INTERFACE IMPORTED)
add_subdirectory(Vulkan)

# Vulkan -- Library 
target_include_directories(RealEngine PUBLIC 
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/Vulkan/Include"
                          )

target_link_directories(RealEngine PUBLIC "${PROJECT_SOURCE_DIR}/Vulkan/Lib")
target_link_libraries(RealEngine ${CMAKE_CURRENT_SOURCE_DIR} vulkan-1.lib)

# GLFW -- Library
target_include_directories(RealEngine PUBLIC 
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/GLFW/include"
                          )
target_link_directories(RealEngine PUBLIC "${PROJECT_SOURCE_DIR}/GLFW/lib-vc2019")
target_link_libraries(RealEngine ${CMAKE_CURRENT_SOURCE_DIR} glfw3.lib)



target_include_directories(RealEngine
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )


  add_compile_options(-std=c++17)
# TODO: Add tests and install targets if needed.
