// Copyright 2020 Sungjin.bae

#ifndef BENGINE_CORE_INTERNAL_ENGINE_H_
#define BENGINE_CORE_INTERNAL_ENGINE_H_


#include <boost/noncopyable.hpp>
#include <boost/numeric/ublas/vector.hpp>

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

} // namespace BEngine

#endif // BENGINE_CORE_INTERNAL_ENGINE_H_
