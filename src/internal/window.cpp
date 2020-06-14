// Copyright 2020 Sungjin.bae

#include <cstdint> // Necessary for UINT32_MAX
#include <set>
#include <sstream>
#include <iostream>
#include <optional>
#include <fstream>

#include <boost/container/map.hpp>

#include "internal/window.h"

static vector<char> ReadFile(const std::string &file_name) {
  // ate : Start reading at the end of the file
  std::ifstream file(file_name, std::ios::ate | std::ios::binary);
  // TODO(sungjin): shader 생성 됬을때 권한 부여 필요.
  BOOST_ASSERT(file.is_open());
  size_t file_size = (size_t)file.tellg();
  vector<char> buffer(file_size);

  // 입력 위치 지정자 값을 파일의 시작으로부터의 위치인 pos 로 설정한다.
  file.seekg(0 /* pos */);
  file.read(buffer.data(), file_size);
  file.close();

  return buffer;
}


uint32_t the_width, the_height;
namespace {

const int MAX_FRAMES_IN_FLIGHT = 2;

const vector<const char*> the_validation_layers = {
  "VK_LAYER_KHRONOS_validation"
};

const vector<const char*> the_device_extensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
  const bool the_enable_validation_layers = false;
#else
  const bool the_enable_validation_layers = true;
#endif


struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  vector<VkSurfaceFormatKHR> formats;
  vector<VkPresentModeKHR> present_modes;
};


VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const vector<VkSurfaceFormatKHR>& available_formats) {
  for (const auto& available_format : available_formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }
  // ranking
  return available_formats[0];
}


VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
     return capabilities.currentExtent;
  } else {
    VkExtent2D actual_extent = {the_width, the_height};
    actual_extent.width =
        std::max(capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actual_extent.width));
    actual_extent.height =
        std::max(capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actual_extent.height));
    return actual_extent;
  }
}


VkPresentModeKHR ChooseSwapPresentMode(
    const vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available_present_mode : available_present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
}


struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  bool IsComplete() {
    return graphics_family.has_value() &&
           present_family.has_value();
  }
};

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, surface, &format_count, nullptr);
  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &format_count, details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_mode_count, details.present_modes.data());
  }
  return details;
}


QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device,
                                           &queue_family_count,
                                           nullptr);
  vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device,
                                           &queue_family_count,
                                           queue_families.data());

  int i=0;
  for (const auto& queue_family : queue_families) {
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
                                         surface, &present_support);
    if (present_support) {
      indices.present_family = i;
    }

    if (indices.IsComplete()) {
      break;
    }
    i++;
  }
  return indices;
}


bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr,
                                       &extension_count, nullptr);
  vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extension_count, available_extensions.data());
  std::set<std::string> required_extensions(
      the_device_extensions.begin(), the_device_extensions.end());
  for (const auto& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }
  return required_extensions.empty();
}


bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
  VkPhysicalDeviceProperties device_properties;
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  vkGetPhysicalDeviceFeatures(device, &device_features);
  // 내장 그래픽 카드, Virtual GPU 지원.
  if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
    return false;
  }

  QueueFamilyIndices indices = FindQueueFamilies(device, surface);
  if (!indices.IsComplete()) {
    return false;
  }

  bool extensions_supported = CheckDeviceExtensionSupport(device);
  bool swap_ahain_adequate = false;
  if (extensions_supported) {
    SwapChainSupportDetails swap_chain_support =
        QuerySwapChainSupport(device, surface);
    swap_ahain_adequate = !swap_chain_support.formats.empty() &&
                          !swap_chain_support.present_modes.empty();
  }
  return swap_ahain_adequate;
}


int RateDeviceSuitability(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties device_properties;
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  vkGetPhysicalDeviceFeatures(device, &device_features);

  int score = 0;
  // Discrete GPUs have a significant performance advantage
  if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }
  // Maximum possible size of textures affects graphics quality
  score += device_properties.limits.maxImageDimension2D;
  return score;
}


bool CheckValidationLayerSupport() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : the_validation_layers) {
    bool found = false;
    for (const auto& layer_properties : available_layers) {
      if (strcmp(layer_name, layer_properties.layerName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }

  return true;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
  }
  return VK_SUCCESS;
}


void PopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& create_info) {
  create_info = {};
  create_info.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = DebugCallback;
}


VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, create_info, allocator, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}


void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}


// ref 를 받아서 처리하는걸로 변경.
vector<const char*> GetRequiredExtensions() {
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  vector<const char*> extensions(glfw_extensions,
                                 glfw_extensions + glfw_extension_count);
  if (the_enable_validation_layers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

}  // unnamed space


namespace BEngine {

Window::Window(const int height, const int width) {
  InitGLFW(height, width);
  InitVulkan();
}


Window::~Window() {
  CleanVulkan();
  CleanGLFW();
}


void Window::InitGLFW(const int height, const int width) {
  int ret = glfwInit();
  BOOST_ASSERT(ret == GLFW_TRUE);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(height, width, "Vulkan", nullptr, nullptr);
  BOOST_ASSERT(window_ != nullptr);

  the_width = width;
  the_height = height;
}


void Window::InitVulkan() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "BEngine";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  if (the_enable_validation_layers) {
    BOOST_ASSERT(CheckValidationLayerSupport());
    create_info.enabledLayerCount =
        static_cast<uint32_t>(the_validation_layers.size());
    create_info.ppEnabledLayerNames = the_validation_layers.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    PopulateDebugMessengerCreateInfo(debug_create_info);
    create_info.pNext =
        (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
  } else {
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
  }

  vector<const char*> glfw_extension = GetRequiredExtensions();
  create_info.ppEnabledExtensionNames = glfw_extension.data();
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(glfw_extension.size());

  VkResult result = vkCreateInstance(&create_info, nullptr, &vk_instance_);
  BOOST_ASSERT(result == VK_SUCCESS);

  if (the_enable_validation_layers) {
    SetupDebugMessenger();
  }

  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFramebuffers();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateSyncObjects();
}


void Window::SetupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  PopulateDebugMessengerCreateInfo(debug_create_info);

  VkResult result =
      CreateDebugUtilsMessengerEXT(vk_instance_, &debug_create_info,
                                   nullptr, &debuger_messenger_);
  BOOST_ASSERT(result == VK_SUCCESS);
}


void Window::CleanGLFW() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}


void Window::CleanVulkan() {
  for (int i = 0 ; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
    vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
    vkDestroyFence(device_, in_flight_fences_[i], nullptr);
  }
  vkDestroyCommandPool(device_, command_pool_, nullptr);
  for (auto framebuffer : swap_chain_framebuffers_) {
    vkDestroyFramebuffer(device_, framebuffer, nullptr);
  }
  vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyRenderPass(device_, render_pass_, nullptr);
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyShaderModule(device_, frag_shader_module_, nullptr);
  vkDestroyShaderModule(device_, vert_shader_module_, nullptr);

  for (auto image_view : swap_chain_image_views_) {
    vkDestroyImageView(device_, image_view, nullptr);
  }

 vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
 vkDestroyDevice(device_, nullptr);
 if (the_enable_validation_layers) {
    DestroyDebugUtilsMessengerEXT(vk_instance_, debuger_messenger_, nullptr);
  }
  vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);
  vkDestroyInstance(vk_instance_, nullptr);
}


void Window::Update() {
  BOOST_ASSERT(window_ != nullptr);
  // This function returns the value of the close flag of
  // the specified Window.
  if (!glfwWindowShouldClose(window_)) {
    // polling glfw event and retrun immediately
    glfwPollEvents();
    DrawFrame();
  }

  vkDeviceWaitIdle(device_);
}


void Window::PickPhysicalDevice() {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(vk_instance_, &device_count, nullptr);
  BOOST_ASSERT(device_count != 0);

  vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(vk_instance_, &device_count, devices.data());
  for (const auto& device : devices) {
    if (IsPhysicalDeviceSuitable(device, surface_)) {
      physical_device_ = device;
      break;
    }
  }

  BOOST_ASSERT(physical_device_ != VK_NULL_HANDLE);

  // Use an ordered map to automatically sort candidates by increasing score
  multimap<int, VkPhysicalDevice> candidates;

  for (const auto& device : devices) {
    int score = RateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }
  BOOST_ASSERT(candidates.rbegin()->first > 0);
  physical_device_ = candidates.rbegin()->second;
}


void Window::CreateLogicalDevice() {
  QueueFamilyIndices indices = FindQueueFamilies(physical_device_,
                                                 surface_);
  BOOST_ASSERT(indices.IsComplete());

  vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = {
      indices.graphics_family.value(),
      indices.present_family.value()
  };

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  VkDeviceCreateInfo create_info{};
  VkPhysicalDeviceFeatures physical_device_features{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.pEnabledFeatures = &physical_device_features;
  create_info.enabledExtensionCount = 0;

  create_info.enabledExtensionCount =
      static_cast<uint32_t>(the_device_extensions.size());
  create_info.ppEnabledExtensionNames = the_device_extensions.data();

  if (the_enable_validation_layers) {
    create_info.enabledLayerCount =
        static_cast<uint32_t>(the_validation_layers.size());
    create_info.ppEnabledLayerNames = the_validation_layers.data();
  } else {
    create_info.enabledLayerCount = 0;
  }

  int ret = vkCreateDevice(physical_device_, &create_info, nullptr, &device_);
  BOOST_ASSERT(ret == VK_SUCCESS);

  vkGetDeviceQueue(device_, indices.graphics_family.value(),
                   0, &graphics_queue_);
  vkGetDeviceQueue(device_, indices.present_family.value(),
                   0, &present_queue_);
}


void Window::CreateSurface() {
  int ret = glfwCreateWindowSurface(vk_instance_, window_, nullptr, &surface_);
  BOOST_ASSERT(ret == VK_SUCCESS);

}


void Window::CreateSwapChain() {
  SwapChainSupportDetails swap_chain_support =
      QuerySwapChainSupport(physical_device_, surface_);
  VkSurfaceFormatKHR surface_format =
      ChooseSwapSurfaceFormat(swap_chain_support.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapPresentMode(swap_chain_support.present_modes);
  VkExtent2D extent =
      ChooseSwapExtent(swap_chain_support.capabilities);

  // 최소 지원보다 하나 더 늘려서 대기하는 일을 막도록한다.
  uint32_t image_count =
      swap_chain_support.capabilities.minImageCount + 1;

  if (swap_chain_support.capabilities.maxImageCount > 0 &&
      image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface_;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(physical_device_, surface_);
  uint32_t queue_family_indices[] = {
    indices.graphics_family.value(),
    indices.present_family.value()
  };

  if (indices.graphics_family != indices.present_family) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0; // Optional
    create_info.pQueueFamilyIndices = nullptr; // Optional
  }

  create_info.preTransform =
      swap_chain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  // 윈도우 창 리사이즈 같은 경우에 핸들을 통해 재생성이 필요한데 거기에 대한 처리가능.
  create_info.oldSwapchain = VK_NULL_HANDLE;

  int ret = vkCreateSwapchainKHR(device_, &create_info,
                                 nullptr, &swap_chain_);
  BOOST_ASSERT(ret == VK_SUCCESS);

  vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, nullptr); swap_chain_images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_, swap_chain_,
                          &image_count, swap_chain_images_.data());

  swap_chain_image_format_ = surface_format.format;
  swap_chain_extent_ = extent;
}


void Window::CreateImageViews() {
  swap_chain_image_views_.resize(swap_chain_images_.size());
  for (int i = 0; i < swap_chain_images_.size(); ++i) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = swap_chain_images_[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = swap_chain_image_format_;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // 이미지의 목적을 구분짓는 필드.
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; create_info.subresourceRange.baseMipLevel = 0; create_info.subresourceRange.levelCount = 1; create_info.subresourceRange.baseArrayLayer = 0; create_info.subresourceRange.layerCount = 1;

    int ret = vkCreateImageView(device_, &create_info,
                                nullptr, &swap_chain_image_views_[i]);
    BOOST_ASSERT(ret == VK_SUCCESS);
  }
}

VkShaderModule CreateShaderModule(VkDevice device,
                                  const vector<char>& code) {
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shader_module;
  int ret = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
  BOOST_ASSERT(ret == VK_SUCCESS);
  return shader_module;
}


void Window::CreateGraphicsPipeline() {
  // TODO(sungjin): 실행 파일 경로에 따라 경로 수정필요.
  auto vert_shader_code = ReadFile("../../shaders/intermediate/vert.spv");
  auto frag_shader_code = ReadFile("../../shaders/intermediate/frag.spv");

  vert_shader_module_ = CreateShaderModule(device_, vert_shader_code);
  frag_shader_module_ = CreateShaderModule(device_, frag_shader_code);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType =
     VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

  vert_shader_stage_info.module = vert_shader_module_;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{}; frag_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module_;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] =
      {vert_shader_stage_info, frag_shader_stage_info};

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional


  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swap_chain_extent_.width;
  viewport.height = (float)swap_chain_extent_.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_extent_;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multi_sampling{};
  multi_sampling.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multi_sampling.sampleShadingEnable = VK_FALSE;
  multi_sampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multi_sampling.minSampleShading = 1.0f; // Optional
  multi_sampling.pSampleMask = nullptr; // Optional
  multi_sampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multi_sampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
  color_blend_attachment.blendEnable = VK_TRUE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blending{};
  color_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment; color_blending.blendConstants[0] = 0.0f; // Optional
  color_blending.blendConstants[1] = 0.0f; // Optional
  color_blending.blendConstants[2] = 0.0f; // Optional
  color_blending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipeline_Layout_info{};
  pipeline_Layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_Layout_info.setLayoutCount = 0; // Optional
  pipeline_Layout_info.pSetLayouts = nullptr; // Optional
  pipeline_Layout_info.pushConstantRangeCount = 0; // Optional
  pipeline_Layout_info.pPushConstantRanges = nullptr; // Optional
  int ret = vkCreatePipelineLayout(device_, &pipeline_Layout_info,
                                   nullptr, &pipeline_layout_);
  BOOST_ASSERT(ret == VK_SUCCESS);

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly; pipeline_info.pViewportState = &viewport_state; pipeline_info.pRasterizationState = &rasterizer; pipeline_info.pMultisampleState = &multi_sampling; pipeline_info.pDepthStencilState = nullptr; // Optional
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = nullptr; // Optional
  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeline_info.basePipelineIndex = -1; // Optional

  ret = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1,
                                  &pipeline_info, nullptr, &graphics_pipeline_);
  BOOST_ASSERT(ret == VK_SUCCESS);
}


void Window::CreateRenderPass() {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = swap_chain_image_format_;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;
  int ret = vkCreateRenderPass(device_, &render_pass_info,
                               nullptr, &render_pass_);
  BOOST_ASSERT(ret == VK_SUCCESS);
}


void Window::CreateFramebuffers() {
  swap_chain_framebuffers_.resize(swap_chain_image_views_.size());
  for (size_t i = 0; i < swap_chain_image_views_.size(); i++) {
    VkImageView attachments[] = {swap_chain_image_views_[i]};
    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; framebuffer_info.renderPass = render_pass_;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = swap_chain_extent_.width;
    framebuffer_info.height = swap_chain_extent_.height;
    framebuffer_info.layers = 1;
    int ret = vkCreateFramebuffer(device_, &framebuffer_info,
                                  nullptr, &swap_chain_framebuffers_[i]);
    BOOST_ASSERT(ret == VK_SUCCESS);
  }
}


void Window::CreateCommandPool() {
  QueueFamilyIndices queue_familyI_indices =
      FindQueueFamilies(physical_device_, surface_);
  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_familyI_indices.graphics_family.value();
  pool_info.flags = 0; // Optional

  int ret = vkCreateCommandPool(device_, &pool_info,
                               nullptr, &command_pool_);
  BOOST_ASSERT(ret == VK_SUCCESS);
}

void Window::CreateCommandBuffers() {
  command_buffers_.resize(swap_chain_framebuffers_.size());
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t)command_buffers_.size();

  int ret = vkAllocateCommandBuffers(
      device_, &alloc_info, command_buffers_.data());
  BOOST_ASSERT(ret == VK_SUCCESS);

  for (size_t i = 0; i < command_buffers_.size(); ++i) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0; // Optional
    begin_info.pInheritanceInfo = nullptr; // Optional
    int ret = vkBeginCommandBuffer(
        command_buffers_[i], &begin_info);
    BOOST_ASSERT(ret == VK_SUCCESS);

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_;
    render_pass_info.framebuffer = swap_chain_framebuffers_[i];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swap_chain_extent_;

    VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffers_[i], &render_pass_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS,                       graphics_pipeline_);

    vkCmdDraw(command_buffers_[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffers_[i]);

    ret = vkEndCommandBuffer(command_buffers_[i]);
    BOOST_ASSERT(ret == VK_SUCCESS);
  }
}


void Window::CreateSyncObjects() {
  image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT); render_finished_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
  in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);
  images_in_flight_.resize(swap_chain_images_.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    int ret = vkCreateSemaphore(device_, &semaphore_info,
                                nullptr, &image_available_semaphores_[i]);
    BOOST_ASSERT(ret == VK_SUCCESS);
    ret = vkCreateSemaphore(device_, &semaphore_info,
                            nullptr, &render_finished_semaphores_[i]);
    BOOST_ASSERT(ret == VK_SUCCESS);

    ret == vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i]);
    BOOST_ASSERT(ret == VK_SUCCESS);
  }
}


void Window::DrawFrame() {
  vkWaitForFences(
      device_, 1, &in_flight_fences_[current_frame_],
      VK_TRUE, UINT64_MAX);
  vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

  uint32_t image_index;
  vkAcquireNextImageKHR(
      device_, swap_chain_, UINT64_MAX,
      image_available_semaphores_[current_frame_],
      VK_NULL_HANDLE, &image_index);


  // Check if a previous frame is using this image
  // (i.e. there is its fence to wait on)
  if (images_in_flight_[image_index] != VK_NULL_HANDLE) {
    vkWaitForFences(device_, 1, &images_in_flight_[image_index],
                    VK_TRUE, UINT64_MAX);
  }
  // Mark the image as now being in use by this frame
  images_in_flight_[image_index] = in_flight_fences_[current_frame_];

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};
  VkPipelineStageFlags wait_stages[] =
      {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers_[image_index];

  VkSemaphore signal_semaphores[] =
      {render_finished_semaphores_[current_frame_]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

  int ret = vkQueueSubmit(graphics_queue_, 1, &submit_info,
                          in_flight_fences_[current_frame_]);
  BOOST_ASSERT(ret == VK_SUCCESS);

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  VkSwapchainKHR swap_chains[] = {swap_chain_};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;

  present_info.pImageIndices = &image_index;

  vkQueuePresentKHR(present_queue_, &present_info);

  current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

}  // namespace BEngine
