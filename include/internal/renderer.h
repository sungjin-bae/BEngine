// Copyright 2020 Sungjin.bae

#ifndef INCLUDE_INTERNAL_RENDERER_H_
#define INCLUDE_INTERNAL_RENDERER_H_


#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "internal/window.h"

/*
 * 앱의 렌더링을 담당한다.
 */
namespace BEngine {

class Renderer : boost::noncopyable {
 public:
  Renderer();
  ~Renderer();

  void Init(int additional_thread);

 private:
  boost::thread_group threads_;
  boost::shared_ptr<Window> window_ = nullptr;

  int window_height_ = 800;
  int window_width_ = 800;
};

}  // namespace BEngine

#endif  // INCLUDE_INTERNAL_RENDERER_H_
