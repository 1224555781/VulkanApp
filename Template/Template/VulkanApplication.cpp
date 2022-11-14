
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "VulkanApplication.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "template_test.h"
#include "glm/fwd.hpp"
#include "glm/vec4.hpp"
#include "glm/glm.hpp"

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
        }
    }
}

VkResult VulkanApplication::InitVulkan()
{
   

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    Print(extensionCount, " extensions supported\n");

    CreateVkInstance();

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

void VulkanApplication::Destroy()
{
    vkDestroyInstance(Instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void VulkanApplication::FindVulkanFamilyQueue(VkPhysicalDevice* physical_device)
{

}
