#ifndef BV_CORE_INTERNAL_WINDOW_H_
#define BV_CORE_INTERNAL_WINDOW_H_

#include <memory>
#include <mutex>

#include <GLFW/glfw3.h>

namespace BEngine {

  enum WindowState {
    kRun,
    kError
  };

  // 부스트 noncopy_able 적용필요.
  class Window {
  public:
    Window();
    virtual ~Window();

    static std::shared_ptr<Window> Create();
    static const char** GetRequiredInstanceExtensions(uint32_t *extension_count);

    // 상태에 따른 배열 필요
    static const int WIDTH = 800;
    static const int HEIGHT = 600;

    void Update();

  private:
    void SetState(WindowState state);
    WindowState GetState();

    // 하나에 하나만 가지는 구조를 가진다.
    GLFWwindow* window_;

    WindowState state_;

    std::mutex mutex_;
  }; // class Window

} // namespace VB



#endif // BV_CORE_INTERNAL_WINDOW_H_
