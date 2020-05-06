// Copyright 2020 Sungjin.bae

#include "internal/renderer.h"

#include <boost/make_shared.hpp>


namespace BEngine {

Renderer::Renderer() {
}

Renderer::~Renderer() {
}


void Renderer::Init(int additional_thread) {
  window_ = boost::make_shared<Window>();
  auto window_thread = window_->Make(window_width_, window_height_);
  if (window_thread != nullptr) {
    threads_.add_thread(window_thread);
  }
}

}  // namespace BEngine
