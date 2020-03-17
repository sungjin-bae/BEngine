#include <iostream>

#include "window.h"


int main() {  

  std::cout << "hellow" << std::endl;

  auto a = BEngine::Window::Create();

  while (true) {
    a->Update();
  }

  return 0;
}