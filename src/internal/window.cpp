// Copyright 2020 Sungjin.bae

#include <sstream>
#include <iostream>

#include <boost/container/vector.hpp>

#include "internal/window.h"

using namespace boost::container;

namespace {

const vector<const char*> the_validation_layers = {
  "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
  const bool the_enable_validation_layers = false;
#else
  const bool the_enable_validation_layers = true;
#endif


bool checkValidationLayerSupport() {
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
  ClearVulkan();
  ClearGLFW();
}


void Window::InitGLFW(const int height, const int width) {
  int ret = glfwInit();
  BOOST_ASSERT(ret == GLFW_TRUE);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_ = glfwCreateWindow(height, width, "Vulkan", nullptr, nullptr);
  BOOST_ASSERT(window_ != nullptr);
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
    BOOST_ASSERT(checkValidationLayerSupport());
    create_info.enabledLayerCount = static_cast<uint32_t>(the_validation_layers.size());
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
}


void Window::SetupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  PopulateDebugMessengerCreateInfo(debug_create_info);

  VkResult result =
      CreateDebugUtilsMessengerEXT(vk_instance_, &debug_create_info,
                                   nullptr, &debuger_messenger_);
  BOOST_ASSERT(result == VK_SUCCESS);
}


void Window::ClearGLFW() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}


void Window::ClearVulkan() {
 if (the_enable_validation_layers) {
    DestroyDebugUtilsMessengerEXT(vk_instance_, debuger_messenger_, nullptr);
  }
  vkDestroyInstance(vk_instance_, nullptr);
}


void Window::Update() {
  BOOST_ASSERT(window_ != nullptr);
  // This function returns the value of the close flag of
  // the specified Window.
  if (!glfwWindowShouldClose(window_)) {
    // polling glfw event and retrun immediately
    glfwPollEvents();
  }
}

}  // namespace BEngine
