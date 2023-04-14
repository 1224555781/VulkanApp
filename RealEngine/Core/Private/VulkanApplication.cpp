
// 对齐规则======
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define  MM ()


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/Public/VulkanApplication.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>


#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <algorithm>
#include <chrono>
#include <set>
#include <GLFW/glfw3native.h>
#include "Core/Public/PlatformFile.h"


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};


const std::vector<Vertex> vertices = {
    {{0.5f, .5f}, {1.0f, 1.0f, 1.0f},{1.f,0.f}},
    {{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f},{0.f,0.f}},
    {{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f},{0.f,1.f}},
    {{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f},{1.f,1.f}}
};

const std::vector<uint32> indices = {
    0,1,2,0,2,3
};

VulkanApplication::VulkanApplication()
    :MaxFramInFight(2)
    ,CurrentFrame(0)
{
    
}

void VulkanApplication::RenderWindow()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(device_);
}

void VulkanApplication::InitApplication()
{
    InitWindow();
    InitVulkan();
}

void VulkanApplication::FindPhysicalDevice()
{

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        RUNTIME_ERROR("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            GPU = device;
            break;
        }
    }

    if (GPU == VK_NULL_HANDLE) {
        RUNTIME_ERROR("failed to find a suitable GPU!");
    }


}

void VulkanApplication::DrawFrame()
{
    //等待上一帧完成
    vkWaitForFences(device_, 1, &inFlightFence[CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult SwapChainCheckResult = vkAcquireNextImageKHR(device_, swapChain, UINT64_MAX, imageAvailableSemaphore[CurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if (SwapChainCheckResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        ReCreateSwapChain();
        return;
    }

    vkResetFences(device_, 1, &inFlightFence[CurrentFrame]);

   

    vkResetCommandBuffer(commandBuffer[CurrentFrame], 0);

    UpdateUniformBuffer(CurrentFrame);
    RecordCommandBuffer(commandBuffer[CurrentFrame], imageIndex);


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer[CurrentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[CurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence[CurrentFrame]) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    VkResult PresentKHRCheck = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (PresentKHRCheck == VK_ERROR_OUT_OF_DATE_KHR)
    {
        ReCreateSwapChain();
    }
    CurrentFrame = (CurrentFrame + 1) % MaxFramInFight;
}

void VulkanApplication::CreateLogicDevice()
{
    FVulkanQueueFamily indices = FindVulkanFamilyQueue(GPU);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};
    std::set<uint32_t> uniqueQueueFamilies = { indices.GetGraphicsValue(), indices.GetPresentValue() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if ( enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(GPU, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create logical device!");
    }

    vkGetDeviceQueue(device_, indices.GetGraphicsValue(), 0, &graphicsQueue);
    vkGetDeviceQueue(device_, indices.GetPresentValue(), 0, &presentQueue);
}

VkResult VulkanApplication::InitVulkan()
{
    CreateVkInstance();

    SetupDebugMessenger();


    CreateSurface();
    FindPhysicalDevice();
    CreateLogicDevice();
    CreateSwapChain();
    CreateImageView();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFrameBuffer();
    CreateCommandPool();

    //show image
    CreateTextureImage("Image\\MLB.png");
    CreateTextureImageView();
    CreateTextureSampler();

    CreateVertexBuffer();
    CreateIndexBufffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();

    CreateCommandBuffer();
    CreateSyncObjects();
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

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }


    return device_features.geometryShader && properties.deviceType == 2 && extensionsSupported &&swapChainAdequate;
}


std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void VulkanApplication::CreateSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(GPU);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_khr_;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    FVulkanQueueFamily indices = FindVulkanFamilyQueue(GPU);
    uint32_t queueFamilyIndices[] = { indices.Get(), indices.GetPresentValue() };

    if (indices.GetGraphicsValue() != indices.GetPresentValue()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    return capabilities.currentExtent;
}

bool VulkanApplication::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_khr_, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_khr_, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_khr_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_khr_, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_khr_, &presentModeCount, details.presentModes.data());
    }

    return details;
}
void VulkanApplication::Tick()
{
    RenderWindow();
}

void VulkanApplication::CreateImageView()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat);
    }
    
}

void VulkanApplication::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create render pass!");
    }
}

VkShaderModule VulkanApplication::CreateShaderModule(const std::vector<uint8>& code)
{
    VkShaderModule shader_module;
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    
    if(vkCreateShaderModule(device_, &createInfo, nullptr, &shader_module)!=VK_SUCCESS)
    {
        Print("Error [createShaderModuleFailed]");
    }

    return shader_module;

}

void VulkanApplication::CreateGraphicsPipeline()
{
    FPlatformFile* PlateformFile = FPlatformFile::Get();
    auto vertShaderCode = PlateformFile->ReadFileToBinary("RealEngine/Shader/vert.spv");
    auto fragShaderCode = PlateformFile->ReadFileToBinary("RealEngine/Shader/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo ShaderStageCreateInfos[2] ={ vertShaderStageInfo ,fragShaderStageInfo };
    auto AttributeDescriptions = Vertex::GetAttributeDescriptions();
    auto BindDescription = Vertex::GetBindingDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =static_cast<uint32>( AttributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &BindDescription; // Optional
    vertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions.data(); // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkRect2D scissor;
    scissor.extent = swapChainExtent;
    scissor.extent = { 0,0 };

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    // 禁用光栅器,禁用了对帧缓冲区的任何输出。
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    //VK_POLYGON_MODE_FILL：用片段填充多边形的区域
    //VK_POLYGON_MODE_LINE：多边形边被绘制为线
	//VK_POLYGON_MODE_POINT：多边形顶点绘制为点
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    // 它根据片段数描述线条的粗细。支持的最大线宽取决于硬件和任何比1.0f要求您启用wideLinesGPU 功能更粗的线
    rasterizer.lineWidth = 1.0f;

    
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    //多重采样  --- 
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional


    // blendEnable 选择是否开启 alpha通道的blend
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT ;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
    

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = ShaderStageCreateInfos;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create graphics pipeline!");
    }

}

void VulkanApplication::CreateFrameBuffer()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    
    for (int32 i = 0; i< swapChainImageViews.size();++i)
    {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            RUNTIME_ERROR("failed to create framebuffer!");
        }
        
    }
}

void VulkanApplication::CreateCommandPool()
{
    FVulkanQueueFamily queueFamilyIndices = FindVulkanFamilyQueue(GPU);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    //VK_COMMAND_POOL_CREATE_TRANSIENT_BIT：提示命令缓冲区经常用新命令重新记录（可能会改变内存分配行为)
    //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 允许单独重新记录命令缓冲区，如果没有这个标志，它们都必须一起重置
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GetGraphicsValue();

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create command pool!");
    }
}

void VulkanApplication::CreateCommandBuffer()
{
    commandBuffer.resize(MaxFramInFight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;

    //VK_COMMAND_BUFFER_LEVEL_PRIMARY: 可以提交到队列执行，但不能从其他命令缓冲区调用。
    //VK_COMMAND_BUFFER_LEVEL_SECONDARY: 不能直接提交，但可以从主命令缓冲区调用。
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MaxFramInFight;

    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffer.data()) != VK_SUCCESS)
    {
        RUNTIME_ERROR("failed to allocate command buffers!");
    }
    
}

void VulkanApplication::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();


    VkBuffer StagingVertexBuffer;
    VkDeviceMemory StagingVertexMem;

    CreateBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingVertexBuffer, StagingVertexMem);



    void* data = nullptr;
    vkMapMemory(device_, StagingVertexMem, 0, bufferSize, 0, &data);
    //将数据拷贝到内存映射到GPU的区域
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(device_, StagingVertexMem);

    
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer,VertexMem);


    CopyBuffer(VertexBuffer, StagingVertexBuffer, bufferSize);

    vkDestroyBuffer(device_, StagingVertexBuffer, nullptr);
    vkFreeMemory(device_, StagingVertexMem, nullptr);
}

void VulkanApplication::CreateIndexBufffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();


    VkBuffer StagingVertexBuffer;
    VkDeviceMemory StagingVertexMem;

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingVertexBuffer, StagingVertexMem);



    void* data = nullptr;
    vkMapMemory(device_, StagingVertexMem, 0, bufferSize, 0, &data);
    //将数据拷贝到内存映射到GPU的区域
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(device_, StagingVertexMem);


    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndicesBuffer, IndicesMem);


    CopyBuffer(IndicesBuffer, StagingVertexBuffer, bufferSize);

    vkDestroyBuffer(device_, StagingVertexBuffer, nullptr);
    vkFreeMemory(device_, StagingVertexMem, nullptr);
}

void VulkanApplication::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

   
    

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    //确保设置 stageFlags 以指示我们打算在片段着色器中使用组合的图像采样器描述符。
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();


    VkResult result = vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        RUNTIME_ERROR("create descriptor layout failed");
    }


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;


}

void VulkanApplication::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(FUniformBufferObject);

    uniformBuffers.resize(MaxFramInFight);
    uniformBuffersMemory.resize(MaxFramInFight);
    uniformBuffersMapped.resize(MaxFramInFight);

    for (size_t i = 0; i < MaxFramInFight; i++) {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device_, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanApplication::CreateDescriptorPool()
{

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MaxFramInFight);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MaxFramInFight);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MaxFramInFight);
   
    vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool);

}

void VulkanApplication::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MaxFramInFight, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32>(MaxFramInFight);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MaxFramInFight);
    VkResult Result = vkAllocateDescriptorSets(device_, &allocInfo, descriptorSets.data());
    if (Result == VK_SUCCESS)
    {
        for (size_t i = 0; i < MaxFramInFight; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(FUniformBufferObject);


            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;


            std::array< VkWriteDescriptorSet ,2>descriptorWrite{};
            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = descriptorSets[i];
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = 1;
            descriptorWrite[0].pBufferInfo = &bufferInfo;
            descriptorWrite[0].pImageInfo = nullptr; // Optional
            descriptorWrite[0].pTexelBufferView = nullptr; // Optional

            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].dstSet = descriptorSets[i];
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].descriptorCount = 1;
            descriptorWrite[1].pImageInfo = &imageInfo; // Optional 

            vkUpdateDescriptorSets(device_, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);


        }
    }
    else
    {
        RUNTIME_ERROR("create descriptor sets failed");
    }
    
}

void VulkanApplication::CopyBuffer(VkBuffer DstBuffer, VkBuffer SrcBuffer, VkDeviceSize Size)
{

    VkCommandBuffer command_buffer = BeginSingleTimeCommands();
    
    VkBufferCopy BufferCopy{};
    BufferCopy.srcOffset = 0;
    BufferCopy.dstOffset = 0;
    BufferCopy.size = Size;
    vkCmdCopyBuffer(command_buffer, SrcBuffer, DstBuffer, 1, &BufferCopy);


    //需要立即提交 推送copybuffer 并等待任务完成,让显卡可以读到最佳内存位置的顶点信息
    EndSingleTimeCommands(command_buffer);
}

void VulkanApplication::RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT：命令缓冲区执行一次后会立即重新记录。
    //VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT：这是一个辅助命令缓冲区，将完全在单个渲染过程中。
    //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT：命令缓冲区可以在它也已经挂起执行时重新提交
    beginInfo.flags = 0; // 画三角形 用不到这些标志----
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(InCommandBuffer, &beginInfo) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to begin recording command buffer!");
    }


    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    //VK_SUBPASS_CONTENTS_INLINE：渲染过程命令将嵌入主命令缓冲区本身，不会执行辅助命令缓冲区。
    //////VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS：渲染过程命令将从辅助命令缓冲区执行
    ///
    vkCmdBeginRenderPass(InCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	    vkCmdBindPipeline(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	    VkViewport viewport{};
	    viewport.x = 0.0f;
	    viewport.y = 0.0f;
	    viewport.width = static_cast<float>(swapChainExtent.width);
	    viewport.height = static_cast<float>(swapChainExtent.height);
	    viewport.minDepth = 0.0f;
	    viewport.maxDepth = 1.0f;
	    vkCmdSetViewport(InCommandBuffer, 0, 1, &viewport);

	    VkRect2D scissor{};
	    scissor.offset = { 0, 0 };
	    scissor.extent = swapChainExtent;
	    vkCmdSetScissor(InCommandBuffer, 0, 1, &scissor);
	    
	    VkBuffer VertexBuffers[] = { VertexBuffer };
	    VkDeviceSize Offsets[] = { 0 };
	    vkCmdBindVertexBuffers(InCommandBuffer, 0, 1, VertexBuffers, Offsets);

	    vkCmdBindIndexBuffer(InCommandBuffer, IndicesBuffer, 0, VK_INDEX_TYPE_UINT32);

	    //绑定描述符集
	    vkCmdBindDescriptorSets(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[CurrentFrame], 0, nullptr);
	    //执行渲染命令
	    vkCmdDrawIndexed(InCommandBuffer,static_cast<uint32>( indices.size()),1,0,0,0);
	vkCmdEndRenderPass(InCommandBuffer);

    if (vkEndCommandBuffer(InCommandBuffer) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to record command buffer!");
    }
}

void VulkanApplication::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    imageAvailableSemaphore.resize(MaxFramInFight);
    renderFinishedSemaphore.resize(MaxFramInFight);
    inFlightFence.resize(MaxFramInFight);
    for (int32 i =0;i< MaxFramInFight;++i)
    {
        if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS) {
            RUNTIME_ERROR("failed to create semaphores!");
        }
    }
    
}

void VulkanApplication::UpdateUniformBuffer(uint32 InCurrentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    FUniformBufferObject MVP{};
    MVP.model = glm::rotate(glm::mat4(1.0f),glm::radians(90.f), glm::vec3(0, 0, 1));
    MVP.view = glm::lookAt(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0., 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    MVP.proj = glm::perspective<float>(glm::radians(45.f), static_cast<float>(swapChainExtent.width/swapChainExtent.height), 0.1f, 10.f);
    MVP.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[InCurrentFrame], &MVP, sizeof(MVP));
}

uint32 VulkanApplication::FindMemeoryType(uint32 typeFilter, VkMemoryPropertyFlags property)
{
    

    VkPhysicalDeviceMemoryProperties physical_device_memory;
    vkGetPhysicalDeviceMemoryProperties(GPU, &physical_device_memory);

    for (uint32 i =0; i< physical_device_memory.memoryTypeCount;++i)
    {
        //检测内存位置 和 属性是否满足要求
        if (typeFilter & (1<<i) && (physical_device_memory.memoryTypes[i].propertyFlags & property) == property)
        {
            return i;
        }
    }
    RUNTIME_ERROR("not find suitable mem type");
}

void VulkanApplication::ReCreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_);

    CleanSwapChain();


    CreateSwapChain();
    CreateImageView();
    CreateFrameBuffer();

}

void VulkanApplication::CreateVkInstance()
{
    if(enableValidationLayers && !checkValidationLayerSupport()) {
        RUNTIME_ERROR("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MLB";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Real Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &Instance) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create instance!");
    }

}

void VulkanApplication::CreateSurface()
{

    VkSurfaceKHR* surface = new VkSurfaceKHR;
    if (glfwCreateWindowSurface(Instance, window, nullptr, surface) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to create window surface!");
    }
    surface_khr_ = *surface;
}


void VulkanApplication::CleanSwapChain()
{
    for (auto& framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device_, imageView, nullptr);
    }


    vkDestroySwapchainKHR(device_, swapChain, nullptr);
}

void VulkanApplication::CreateTextureImage(const std::string& Path)
{
    int32 textureWidth, textureHeight, textureChannels;
    std::string AbsPath = "G:/RealEngine/RealEngine/Image/MLB.png";//FPlatformFile::Get()->GetProjectPath() + Path;
    uint8* pixels = stbi_load(AbsPath.data(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);

    if (!pixels)
    {
        RUNTIME_ERROR("Not Find Image [" + AbsPath + "]")
    }

    VkDeviceSize imageSize = textureHeight * textureWidth * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(device_, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device_, stagingBufferMemory);
    stbi_image_free(pixels);


    ////
    /// 将texture数据存储到GPU可读的内存区域

    CreateImage(textureWidth, textureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, textureImage, textureImageMemory);
    TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, textureImage, textureWidth, textureHeight);

    TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device_, stagingBuffer, nullptr);
    vkFreeMemory(device_, stagingBufferMemory, nullptr);

}

void VulkanApplication::CreateTextureImageView()
{
    textureImageView =  CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void VulkanApplication::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

    //使用各向异性过滤
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GPU, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    //采用 [0-1]采用为标准模式 百分比   -  非标准模式则为 [0-width]绝对值
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    //启用了比较功能，则将首先将纹素与值进行比较，并将该比较的结果用于筛选操作。这主要用于阴影贴图上的百分比接近过滤
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    //mipmap
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    vkCreateSampler(device_, &samplerInfo, nullptr, &textureSampler);
}

VkImageView VulkanApplication::CreateImageView(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;


    VkImageView image_view;
    vkCreateImageView(device_, &viewInfo, nullptr, &image_view);
    return image_view;
}

void VulkanApplication::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer command_buffer = BeginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };
    vkCmdCopyBufferToImage(
        command_buffer,
        buffer, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    EndSingleTimeCommands(command_buffer);
}

VkCommandBuffer VulkanApplication::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanApplication::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}

void VulkanApplication::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkCommandBuffer command_buffer = BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    //If you are using the barrier to transfer queue family ownership, then these two fields should be the ////
    //indices of the queue families. They must be set to VK_QUEUE_FAMILY_IGNORED
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    vkCmdPipelineBarrier(
        command_buffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    EndSingleTimeCommands(command_buffer);
}

void VulkanApplication::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{

    VkImageCreateInfo ImageCreateInfo{};
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.extent.height = height;
    ImageCreateInfo.extent.width = width;
    ImageCreateInfo.extent.depth = 1;
    ImageCreateInfo.mipLevels = 1;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.format = format;
    ImageCreateInfo.tiling = tiling;
    //VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
    //VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage = usage;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.flags = properties;

    vkCreateImage(device_, &ImageCreateInfo, nullptr, &image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);
    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memRequirements.size;
    allocate_info.memoryTypeIndex = FindMemeoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(device_, &allocate_info, nullptr, &imageMemory);
    vkBindImageMemory(device_, image, imageMemory, 0);
}

void VulkanApplication::CreateBuffer(VkDeviceSize DeviceSize, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Property_flags, VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = DeviceSize;
    bufferInfo.usage = Usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device_, &bufferInfo, nullptr, &Buffer);

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device_, Buffer, &mem_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = mem_requirements.size;
    memory_allocate_info.memoryTypeIndex = FindMemeoryType(mem_requirements.memoryTypeBits, Property_flags);

    VkResult MemCreateResult = vkAllocateMemory(device_, &memory_allocate_info, nullptr, &BufferMemory);
    if (MemCreateResult != VK_SUCCESS)
    {
        RUNTIME_ERROR("failed allocate vertex mem");
    }

    vkBindBufferMemory(device_, Buffer, BufferMemory, 0);

}


void VulkanApplication::Destroy()
{
    CleanSwapChain();
    vkDestroySampler(device_, textureSampler, nullptr);
    vkDestroyImageView(device_, textureImageView, nullptr);
    vkDestroyImage(device_, textureImage, nullptr);
    vkFreeMemory(device_, textureImageMemory, nullptr);

    for (size_t i = 0; i < MaxFramInFight; i++) {
        vkDestroyBuffer(device_, uniformBuffers[i], nullptr);
        vkFreeMemory(device_, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device_, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device_, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device_, IndicesBuffer, nullptr);
    vkFreeMemory(device_, IndicesMem, nullptr);

    vkDestroyBuffer(device_, VertexBuffer, nullptr);
    vkFreeMemory(device_, VertexMem, nullptr);
    for (int32 i = 0; i < MaxFramInFight; ++i)
    {
        vkDestroySemaphore(device_, imageAvailableSemaphore[i], nullptr);
        vkDestroySemaphore(device_, renderFinishedSemaphore[i], nullptr);
        vkDestroyFence(device_, inFlightFence[i], nullptr);
    }
    vkDestroyCommandPool(device_, commandPool, nullptr);
   


    vkDestroyPipeline(device_, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device_, pipelineLayout, nullptr);

    vkDestroyRenderPass(device_, renderPass, nullptr);

   

    vkDestroyInstance(Instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

FVulkanQueueFamily VulkanApplication::FindVulkanFamilyQueue(VkPhysicalDevice physical_device)
{
    FVulkanQueueFamily indices;
    

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface_khr_, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.IsValid()) {
            break;
        }

        i++;
    }

    return indices;
}

void VulkanApplication::MainLoop()
{
    Tick();
}

void VulkanApplication::SetupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        RUNTIME_ERROR("failed to set up debug messenger!");
    }
}

void VulkanApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult VulkanApplication::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

