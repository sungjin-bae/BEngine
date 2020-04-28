// Copyright 2020 Sungjin.bae

#include "internal/engine.h"

#include <boost/make_shared.hpp>


namespace BEngine {

Engine::Engine() {
}


Engine::~Engine() {
}


void Engine::Init() {
  renderer_ = boost::make_shared<Renderer>();
}


bool Engine::Start() {
  renderer_->Init(1 /* additional thread num */);
  return true;
}


bool Engine::IsRunning() {
  return true;
}

} // namespace BEngine
