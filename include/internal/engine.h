// Copyright 2020 Sungjin.bae

#ifndef INCLUDE_INTERNAL_ENGINE_H_
#define INCLUDE_INTERNAL_ENGINE_H_

#include <boost/noncopyable.hpp>

#include "internal/renderer.h"

namespace BEngine {

/*
 * 앱의 초기화, 시작, 종료를 담당한다.
 */
class Engine : boost::noncopyable {
 public:
  Engine();
  ~Engine();

  void Init();
  bool Start();
  bool IsRunning();
 private:
  boost::shared_ptr<Renderer> renderer_;
};

}  // namespace BEngine

#endif  // INCLUDE_INTERNAL_ENGINE_H_
