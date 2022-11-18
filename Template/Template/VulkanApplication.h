#pragma once
#include <optional>
#include <vulkan/vulkan_core.h>


struct FVulkanQueueFamily
{
    std::optional<uint32_t> Indices;
    std::optional<uint32_t> SupportKHR;

    bool IsValid()
    {
        return Indices.has_value()&& SupportKHR.has_value();
    }

    uint32_t Get()
    {
        if (IsValid())
        {
            return Indices.value();
        }

        return 0;
    }
};

class VulkanApplication
{
public:

    void RenderWindow();

    void InitApplication();
    void CreateVkInstance();
    void CreateSurface();
    void Destroy();

    FVulkanQueueFamily FindVulkanFamilyQueue(VkPhysicalDevice* physical_device);

    void FindPhysicalDevice();
private:
    void CreateLogicDevice();
    VkResult InitVulkan();
    void InitWindow();
    bool IsDeviceSuitable(const VkPhysicalDevice& device);
    void Tick();
private:

    /**
     * \brief 物理设备
     */
    VkPhysicalDevice physical_device_;
    /**
     * \brief Vulkan Instance
     */
    VkInstance Instance;

    /**
     * \brief Main Window
     */
    struct GLFWwindow* window;

    /**
     * \brief 逻辑设备
     */
    VkDevice device_;


    /**
     * \brief 桥接特定系统
     */
    VkSurfaceKHR surface_khr_;
};

