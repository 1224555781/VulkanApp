#pragma once
#include <vulkan/vulkan_core.h>

class VulkanApplication
{
public:

    void RenderWindow();

    void InitApplication();
    void CreateVkInstance();
    void Destroy();

    void FindVulkanFamilyQueue(VkPhysicalDevice* physical_device);

    void FindPhysicalDevice();
private:

    VkResult InitVulkan();
    void InitWindow();
    bool IsDeviceSuitable(const VkPhysicalDevice& device);
    void Tick();
private:

    VkPhysicalDevice physical_device_;
    /**
     * \brief Vulkan Instance
     */
    VkInstance Instance;

    /**
     * \brief Main Window
     */
    struct GLFWwindow* window;
};

