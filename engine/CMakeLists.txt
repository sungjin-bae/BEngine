# NOTE only supoort Screens platform
CMAKE_MINIMUM_REQUIRED ( VERSION 3.0 )
PROJECT ( BEngine )

include_directories ( 
    include
    D:/Work/Vulkan/B_Vulkan/include
    D:/Work/Vulkan/ThirdParty/glfw-3.3.bin.WIN64/include
    D:/Work/Vulkan/ThirdParty/glm
    C:/VulkanSDK/1.1.126.0/Include )

link_directories (
    C:/VulkanSDK/1.1.126.0/Lib
    D:/Work/Vulkan/ThirdParty/glfw-3.3.bin.WIN64/lib-vc2017 )

find_library ( LIBVULKAN NAMES vulkan-1 HINTS "C:/VulkanSDK/1.1.126.0/Lib" )
if(NOT LIBVULKAN)
  message(FATAL_ERROR "LIBVULKAN library not found")
endif()

find_library ( LIBGLFW3 NAMES glfw3 HINTS "D:/Work/Vulkan/ThirdParty/glfw-3.3.bin.WIN64/lib-vc2017" )
if(NOT LIBGLFW3)
  message(FATAL_ERROR "LIBGLFW3 library not found")
endif()


set ( SOURCE
  src/main.cpp
  src/window.cpp
)

set ( HEADERS
  include/window.h
)

add_executable( ${PROJECT_NAME} 
    ${SOURCE} 
    ${HEADERS} )

target_link_libraries ( ${PROJECT_NAME} 
    ${LIBVULKAN}
    ${LIBGLFW3} )

set ( CMAKE_CXX_FLAGS "-std=c++11" )


