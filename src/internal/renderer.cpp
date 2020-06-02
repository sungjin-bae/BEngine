// Copyright 2020 Sungjin.bae

#include "internal/renderer.h"

#include <boost/make_shared.hpp>


namespace BEngine {

Renderer::Renderer() {
}

Renderer::~Renderer() {
}


void Renderer::Init(int additional_thread) {
  window_ = boost::make_shared<Window>(window_height_, window_width_);
}


void Renderer::Update() {
  window_->Update();
}

}  // namespace BEngine
