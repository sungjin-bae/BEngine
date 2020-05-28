// Copyright 2020 Sungjin.bae


#include <boost/make_shared.hpp>
#ifdef __linux__
#include <unistd.h>
#endif  // __linux__
#include <iostream>


#include "internal/engine.h"
#include "internal/renderer.h"
#include "internal/window.h"


int main() {
  std::cout << "Start BEngine" << std::endl;
  auto engine = boost::make_shared<BEngine::Engine>();
  engine->Init();
  engine->Start();
  while (engine->IsRunning()) {
    // 임시로 정지.
#if defined(__linux__) || defined(__APPLE__)
    sleep(/* seconds */1);
#elif _WIN64
    Sleep(/* seconds */1);
#endif  // __linux__
  }
  return 0;
}
