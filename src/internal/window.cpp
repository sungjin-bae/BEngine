// Copyright 2020 Sungjin.bae

#include <sstream>
#include <iostream>

#include "internal/window.h"


namespace BEngine {

Window::Window() {
  auto ret = glfwInit();
  BOOST_ASSERT(ret == GLFW_TRUE);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}


Window::~Window() {
  glfwTerminate();
}


boost::thread *Window::Make(const int height, const int width) {
  return new boost::thread(
      boost::bind(&Window::Draw, this, height, width));
}


void Window::Draw(const int height, const int width) {
  window_ = glfwCreateWindow(height, width, "Vulkan", nullptr, nullptr);
  BOOST_ASSERT(window_ != nullptr);

  // This function returns the value of the close flag of
  // the specified Window.
  while (!glfwWindowShouldClose(window_)) {
    // polling glfw event and retrun immediately
    glfwPollEvents();
  }
}


// 임시로 남겨둔다 정확한 활용 위치를 확인하기 전까지.
const char** Window::GetRequiredInstanceExtensions(uint32_t *extension_count) {
  return glfwGetRequiredInstanceExtensions(extension_count);
}

}  // namespace BEngine
