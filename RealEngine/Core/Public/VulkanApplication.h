#pragma once
#include <iostream>
#include <optional>
#include <ostream>
#include <vector>
#include "Vulkan/Include/vulkan/vulkan_core.h"
#include "VulkanTypeDefine.h"


struct FVulkanQueueFamily
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsValid()
    {
        return graphicsFamily.has_value()&& presentFamily.has_value();
    }

    //deprecated
    uint32_t Get()
    {
        return GetGraphicsValue();
    }


    uint32_t GetGraphicsValue()
    {
        if (IsValid())
        {
            return graphicsFamily.value();
        }

        return 0;
    }

    uint32_t GetPresentValue()
    {
        if (IsValid())
        {
            return presentFamily.value();
        }

        return 0;
    }

    
};


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApplication
{
public:
    VulkanApplication();

    void RenderWindow();

    void InitApplication();
    void CreateVkInstance();
    void CreateSurface();
    void Destroy();

    FVulkanQueueFamily FindVulkanFamilyQueue(VkPhysicalDevice physical_device);

    void MainLoop();


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
private:
    void SetupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    void Tick();
    void DrawFrame();
    void CreateLogicDevice();
    VkResult InitVulkan();
    void InitWindow();
    bool IsDeviceSuitable(const VkPhysicalDevice& device);
    void CreateSwapChain();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    
    void CreateImageView();
    void CreateRenderPass();
    VkShaderModule CreateShaderModule(const std::vector<uint8>& code);
    void CreateGraphicsPipeline();
    void CreateFrameBuffer();
    void FindPhysicalDevice();

    void CreateCommandPool();
    void CreateCommandBuffer();

    void CreateVertexBuffer();
    void CreateIndexBufffer();

    void CreateDescriptorSetLayout();

    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t imageIndex);

    void CreateSyncObjects();

    void UpdateUniformBuffer(uint32 InCurrentFrame);

    uint32 FindMemeoryType(uint32 typeFilter, VkMemoryPropertyFlags property);
    /**
     * \brief 处理切换视口大小- 重新创建交换链
     */
    void ReCreateSwapChain();

    void CleanSwapChain();


    /**
     * \brief 创建顶点缓冲区, 顶点暂存缓冲区
     */
    void CreateBuffer(VkDeviceSize DeviceSize,VkBufferUsageFlags Usage,VkMemoryPropertyFlags Property_flags,VkBuffer& Buffer,VkDeviceMemory& BufferMemory);

    void CopyBuffer(VkBuffer DstBuffer, VkBuffer SrcBuffer, VkDeviceSize Size);
private:
    VkDebugUtilsMessengerEXT debugMessenger;
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


    /**
     * \brief 渲染队列
     */
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    /**
     * \brief 创建交换链
     */
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;


    /**
     * \brief 图像显示
     */
    std::vector<VkImageView> swapChainImageViews;


    /**
     * \brief 管道布局
     */
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    // 图形管道
    VkPipeline graphicsPipeline;

    
    /**
     * \brief 帧缓冲区, 帧缓冲区对象引用 VkImageView代表附件的所有对象
     */
    std::vector<VkFramebuffer> swapChainFramebuffers;


    //命令缓冲区--发送绘制命令
    VkCommandPool commandPool;

    //
    std::vector< VkCommandBuffer> commandBuffer;


    /**
     * \brief 通用缓存区域 描述符集
     */
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    /**
     * \brief 通用缓冲区---用于 示例: 实时刷新MVP 矩阵数据 - view 实时会变
     */
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    /**
     * \brief 顶点buffer
     */
    VkBuffer VertexBuffer;
    VkDeviceMemory VertexMem;

    /**
     * \brief 索引buffer
     */
    VkBuffer IndicesBuffer;
    VkDeviceMemory IndicesMem;

    //信号量 用来控制渲染
    std::vector < VkSemaphore >imageAvailableSemaphore;
    std::vector < VkSemaphore> renderFinishedSemaphore;
    std::vector < VkFence >inFlightFence;

    int32 MaxFramInFight;
    int32 CurrentFrame;
};

