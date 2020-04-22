#include <cassert>
#include <sstream>

#include <iostream>

#include "internal/window.h"

// static initilaize 시간에 GLWF 를 초기화한다.
struct Static_Initialize_Time_GlFW_Init {
  Static_Initialize_Time_GlFW_Init() {
    auto ret = glfwInit();
    if (ret == GLFW_TRUE) {
      // Failed
      //std::stringstream ss;
      //ss << "Failed to GLFW Init, glfw error : " << ret;
      //ss.str().c_str();
      // 로그 출력 기능이 필요함.
      //assert(false);
    }

    // OK
  }
} GLFW_Init;


namespace BEngine {

Window::Window() {


  // NOTE(sungjin)
  // 크기가 조정된 창을 처리할 때에는 나중에 살펴볼 주의가 필요하다.
  // 추후 확인하고 변경하던지 말던지.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(HEIGHT, WIDTH, "Vulkan", nullptr, nullptr);

  // ..;;;
  SetState(WindowState::kRun);
};


Window::~Window() {

};


std::shared_ptr<Window> Window::Create() {
  return std::make_shared<Window>();
}


const char** Window::GetRequiredInstanceExtensions(uint32_t *extension_count) {
  return glfwGetRequiredInstanceExtensions(extension_count);
}


void Window::Update() {
    // 윈도우의 상태를 명시적으로 확인하기위해서
  if (GetState() == kError) {
    std::cout << "Window update error please check ..." << std::endl;
    return;
  }

  // This function returns the value of the close flag of the specified Window.
  if (!glfwWindowShouldClose(window_)) {
    // polling glfw event and retrun immediately
    glfwPollEvents();
  }
}


void Window::SetState(WindowState state) {
  std::unique_lock<std::mutex> lock(mutex_);
  state_ = state;
}


WindowState Window::GetState() {
  std::unique_lock<std::mutex> lock(mutex_);
  return state_;
}

} // namespace VB