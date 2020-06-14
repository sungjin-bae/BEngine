// Copyright 2020 Sungjin.bae

#ifndef INCLUDE_INTERNAL_WINDOW_H_
#define INCLUDE_INTERNAL_WINDOW_H_

// GLFW will include its own definitions and
// automatically load the vulakn header
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include <boost/container/vector.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <vulkan/vulkan.h>

using namespace boost::container;


namespace BEngine {

class Window : boost::noncopyable {
 public:
  Window(const int height, const int width);
  ~Window();

  void Update();
 private:
  void InitGLFW(const int height, const int width);
  void InitVulkan();
  void SetupDebugMessenger();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSurface();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateGraphicsPipeline();
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void DrawFrame();
  void CreateSyncObjects();

  void CleanGLFW();
  void CleanVulkan();
  GLFWwindow* window_ = nullptr;
  VkInstance vk_instance_ = nullptr;
  VkDebugUtilsMessengerEXT debuger_messenger_ = nullptr;
  VkPhysicalDevice physical_device_ = nullptr;
  VkDevice device_ = nullptr;
  // Device queues are implicitly cleaned up when the device is destroyed, so we
  // don’t need to do anything in cleanup.
  VkQueue graphics_queue_;
  VkQueue present_queue_;
  VkSurfaceKHR surface_;
  VkSwapchainKHR swap_chain_;
  vector<VkImage> swap_chain_images_;
  VkFormat swap_chain_image_format_;
  VkExtent2D swap_chain_extent_;
  vector<VkImageView> swap_chain_image_views_;
  VkShaderModule frag_shader_module_;
  VkShaderModule vert_shader_module_;
  VkPipelineLayout pipeline_layout_;
  VkRenderPass render_pass_;
  VkPipeline graphics_pipeline_;
  vector<VkFramebuffer> swap_chain_framebuffers_;
  VkCommandPool command_pool_;
  vector<VkCommandBuffer> command_buffers_;

  vector<VkSemaphore> image_available_semaphores_;
  vector<VkSemaphore> render_finished_semaphores_;
  vector<VkFence> in_flight_fences_;
  vector<VkFence> images_in_flight_;
  size_t current_frame_ = 0;
};

}  // namespace BEngine

#endif  // INCLUDE_INTERNAL_WINDOW_H_
