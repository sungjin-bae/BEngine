# NOTE only supoort Screens platform
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(BEngine)

set ( CMAKE_CXX_FLAGS "-std=c++11" )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.72.0 COMPONENTS thread REQUIRED)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

#include_directories($ENV{VULKAN_SDK}/Include)
#link_directories($ENV{VULKAN_SDK}/Lib)

include_directories (
    include )

set ( SOURCE
  src/main.cpp
  src/internal/window.cpp
)

set ( HEADERS
  include/internal/window.h
)

add_executable( ${PROJECT_NAME}
    ${SOURCE}
    ${HEADERS})


#target_link_libraries (${PROJECT_NAME}  Vulkan)
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_include_directories(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

target_link_libraries (${PROJECT_NAME}  glfw)

include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})