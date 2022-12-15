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
     * \brief �����л��ӿڴ�С- ���´���������
     */
    void ReCreateSwapChain();

    void CleanSwapChain();


    /**
     * \brief �������㻺����, �����ݴ滺����
     */
    void CreateBuffer(VkDeviceSize DeviceSize,VkBufferUsageFlags Usage,VkMemoryPropertyFlags Property_flags,VkBuffer& Buffer,VkDeviceMemory& BufferMemory);

    void CopyBuffer(VkBuffer DstBuffer, VkBuffer SrcBuffer, VkDeviceSize Size);
private:
    VkDebugUtilsMessengerEXT debugMessenger;
    /**
     * \brief �����豸
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
     * \brief �߼��豸
     */
    VkDevice device_;


    /**
     * \brief �Ž��ض�ϵͳ
     */
    VkSurfaceKHR surface_khr_;


    /**
     * \brief ��Ⱦ����
     */
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    /**
     * \brief ����������
     */
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;


    /**
     * \brief ͼ����ʾ
     */
    std::vector<VkImageView> swapChainImageViews;


    /**
     * \brief �ܵ�����
     */
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    // ͼ�ιܵ�
    VkPipeline graphicsPipeline;

    
    /**
     * \brief ֡������, ֡�������������� VkImageView�����������ж���
     */
    std::vector<VkFramebuffer> swapChainFramebuffers;


    //�������--���ͻ�������
    VkCommandPool commandPool;

    //
    std::vector< VkCommandBuffer> commandBuffer;


    /**
     * \brief ͨ�û������� ��������
     */
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    /**
     * \brief ͨ�û�����---���� ʾ��: ʵʱˢ��MVP �������� - view ʵʱ���
     */
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    /**
     * \brief ����buffer
     */
    VkBuffer VertexBuffer;
    VkDeviceMemory VertexMem;

    /**
     * \brief ����buffer
     */
    VkBuffer IndicesBuffer;
    VkDeviceMemory IndicesMem;

    //�ź��� ����������Ⱦ
    std::vector < VkSemaphore >imageAvailableSemaphore;
    std::vector < VkSemaphore> renderFinishedSemaphore;
    std::vector < VkFence >inFlightFence;

    int32 MaxFramInFight;
    int32 CurrentFrame;
};

