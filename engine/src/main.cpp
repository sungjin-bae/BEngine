// Copyright 2020 Sungjin.bae


#include <boost/make_shared.hpp>
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
    Sleep(1);
  }
  return 0;
}
