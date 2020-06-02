// Copyright 2020 Sungjin.bae

#ifndef INCLUDE_INTERNAL_WINDOW_H_
#define INCLUDE_INTERNAL_WINDOW_H_

// GLFW will include its own definitions and
// automatically load the vulakn header
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <vulkan/vulkan.h>


namespace BEngine {

class Window : boost::noncopyable {
 public:
  Window(const int height, const int width);
  ~Window();

  void Update();
 private:
  void InitGLFW(const int height, const int width);
  void InitVulkan();
  void SetupDebugMessenger();

  void ClearGLFW();
  void ClearVulkan();
  GLFWwindow* window_ = nullptr;
  VkInstance vk_instance_ = nullptr;
  VkDebugUtilsMessengerEXT debuger_messenger_ = nullptr;
};

}  // namespace BEngine

#endif  // INCLUDE_INTERNAL_WINDOW_H_
