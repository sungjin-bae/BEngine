// Copyright 2020 Sungjin.bae

#ifndef BENGINE_CORE_INTERNAL_WINDOW_H_
#define BENGINE_CORE_INTERNAL_WINDOW_H_


#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <memory>
#include <mutex>

#include <GLFW/glfw3.h>


namespace BEngine {

class Window : boost::noncopyable {
 public:
  Window();
  ~Window();

  boost::thread *Make(const int height, const int width);
 private:
  void Draw(const int height, const int width);
  GLFWwindow* window_;
  std::mutex mutex_;

  static const char** GetRequiredInstanceExtensions(uint32_t *extension_count);
};

} // namespace BEngine

#endif // BENGINE_CORE_INTERNAL_WINDOW_H_
