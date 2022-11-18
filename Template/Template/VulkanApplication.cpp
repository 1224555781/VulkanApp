
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define  MM ()
#include "VulkanApplication.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "template_test.h"
#include "glm/fwd.hpp"
#include "glm/vec4.hpp"
#include "glm/glm.hpp"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>

void VulkanApplication::RenderWindow()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void VulkanApplication::InitApplication()
{
    InitVulkan();
    
    InitWindow();
}

void VulkanApplication::FindPhysicalDevice()
{
    uint32_t* Count = new uint32_t;
    VkPhysicalDevice physical_device;
    std::vector<VkPhysicalDevice> devices{};
    vkEnumeratePhysicalDevices(Instance, Count, devices.data());
    if (devices.capacity() == 0)
    {
        Print("Fatal Error Not Find Suitable GPU");
        return;
    }

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            physical_device = device;
            break;
        }
    }


}

void VulkanApplication::CreateLogicDevice()
{
    FVulkanQueueFamily queue_family = FindVulkanFamilyQueue(&physical_device_);
    if (queue_family.IsValid())
    {
        
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue_family.Get();
        queueCreateInfo.queueCount = 1;
        float priority = 0.f;
        queueCreateInfo.pQueuePriorities = &priority;

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.enabledLayerCount = queue_family.Get();
        create_info.pQueueCreateInfos = &queueCreateInfo;
        //—È÷§≤„  debug
        create_info.enabledLayerCount = 0;
        create_info.enabledExtensionCount = 0;
        VkResult result = vkCreateDevice(physical_device_, &create_info,nullptr,&device_);
        if (result != VK_SUCCESS)
        {
            Print("Create Logic Device Failed");
        };
    }
}

VkResult VulkanApplication::InitVulkan()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    Print(extensionCount, " extensions supported\n");

    CreateVkInstance();
    CreateSurface();
    CreateLogicDevice();
    return VK_SUCCESS;
}

void VulkanApplication::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
}

bool VulkanApplication::IsDeviceSuitable(const VkPhysicalDevice& device)
{
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    return device_features.geometryShader && properties.deviceType == 2;
}

void VulkanApplication::Tick()
{
    RenderWindow();
}

void VulkanApplication::CreateVkInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "RealEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "RealEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;


    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
    if (result == VkResult::VK_SUCCESS)
    {
        Print("Create Instance Success");
    }

}

void VulkanApplication::CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);
    VkResult Result = vkCreateWin32SurfaceKHR(Instance, &createInfo, nullptr, &surface_khr_);
    if (Result == VK_SUCCESS)
    {
        Print("Create Surface Success");
        //glfwCreateWindowSurface(Instance, window, nullptr, &surface_khr_);
    }

}

void VulkanApplication::Destroy()
{
    vkDestroyInstance(Instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

FVulkanQueueFamily VulkanApplication::FindVulkanFamilyQueue(VkPhysicalDevice* physical_device)
{
    FVulkanQueueFamily queue;
    uint32_t Count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &Count,nullptr);
    std::vector<VkQueueFamilyProperties> propertieses(Count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &Count, propertieses.data());
    uint32_t ret = 0;
    for (const auto& property :propertieses)
    {
        if(property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queue.Indices = ret;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, ret, surface_khr_, &presentSupport);
        if (presentSupport)
        {
            queue.SupportKHR = ret;
        }
        ret++;
    }
    return queue;
}
