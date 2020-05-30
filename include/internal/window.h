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


namespace BEngine {

class Window : boost::noncopyable {
 public:
  Window();
  ~Window();

  void Init(const int height, const int width);
  void Update();
 private:
  GLFWwindow* window_;
  boost::mutex mutex_;

  static const char** GetRequiredInstanceExtensions(uint32_t *extension_count);
};

}  // namespace BEngine

#endif  // INCLUDE_INTERNAL_WINDOW_H_
