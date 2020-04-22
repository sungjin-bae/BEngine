// Copyright 2020 Sungjin.bae

#include <iostream>

#include "internal/window.h"


int main() {
  std::cout << "hellow" << std::endl;

  auto a = BEngine::Window::Create();

  while (true) {
    a->Update();
  }

  return 0;
}
