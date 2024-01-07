//the Vulkan header from the LunarG SDK, which provides the functions, structures and enumerations
//#include <vulkan/vulkan.h> but we dont need it because define GLFW_INCLUDE_VULKAN gives it to us


#define GLFW_INCLUDE_VULKAN //gives us <vulkan/vulkan.h> in side <GLFW/glfw3.h>
#include <GLFW/glfw3.h>//to help us create a window

// The stdexcept and iostream headers are included for reporting and propagating errors
#include <iostream>
#include <stdexcept>

// The cstdlib header provides the EXIT_SUCCESS and EXIT_FAILURE macros
#include <cstdlib>


#include <vector>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

#ifdef NDEBUG//a valuable that indecate if we want to be in debug mode or not 
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//validation Layers that we whant to enable
const std::vector<const char*> validationLayers = {
    // All of the useful standard validation is bundled into a layer included in the SDK that is known as VK_LAYER_KHRONOS_validation
    "VK_LAYER_KHRONOS_validation"
};

// All of the device extensions that we need
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
class HelloTriangleApplication {
private:
    GLFWwindow*              window;//a reference to the window
    VkInstance               instance;//a vulkan instance
    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;    //physicalDevice handle
    uint32_t                 graphicsFamily; //the graphicsFamily
    VkDevice                 logicalDevice;//logical device hanlde
    unsigned int             graphicsQueueIndex;
    VkQueue                  graphicsQueue;
    unsigned int             presentQueueIndex;
    VkQueue                  presentQueue;
    VkSurfaceKHR             surface;

    VkExtent2D               swapChainExtent;
    VkSurfaceFormatKHR       swapChainFormat;
    VkPresentModeKHR         swapChainPresentMode;
    uint32_t                 swapChainImageCount;

    VkSwapchainKHR           swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan() 
        {
        //testing ValidationLayerSupport and enableValidationLayers
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        
#pragma region initializes_glfw_and_create_window

        glfwInit();//initializes the GLFW library
        //  Because GLFW was originally designed to create an OpenGL context, we need to tell it to not create an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        //Because handling resized windows takes special care that we'll look into later, disable it for now with another window hint call
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//create the window with the name vulkan

        //initializes_glfw_and_craete_window
#pragma endregion     
#pragma region vulkan_instans
        
        //VkApplicationInfo struct is technically optional, but it may provide some useful information to the driver in order to optimize our specific application
        VkApplicationInfo appInfo{};//create a struct to help us create an vulkan instans
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        //VkInstanceCreateInfo struct is not optional and tells the Vulkan driver which global extensions and validation layers we want to use
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;


        /*will hold the number of extensions that we need for glfw
        it will hole the number 2*/
        uint32_t glfwExtensionCount = 0;
        /*will hold the extensions that we need for glfw
        an glfw function that returns the extensions that we need for glfw and there number*/
        /* it will hold this value:VK_KHR_surface,VK_KHR_win32_surface
           and you can print it with
          for (int i=0;i< glfwExtensionCount;i++)
        {
            for(int j = 0; j < 20; j++)
            {
            std::cout << glfwExtensions[i][j];
            }
            std::cout <<"" << std::endl;

        }
        */
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        //the number of global extensions to enable for glfw
        createInfo.enabledExtensionCount = glfwExtensionCount;
        //a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the names of extensions to enable for glfw
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        //idk
        createInfo.enabledLayerCount = 0;
        //we are creating the instance and we get VK_SUCCESS or an error code 
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        //vulkan_instans
#pragma endregion
#pragma region extensions_supported

        //the number of supported extensions(it will be 15)
        uint32_t extensionCount = 0;
        //put in extensionCount the number of supported extensions(it will be 15)
        /*vkEnumerateInstanceExtensionProperties will put in the number of supported extensions because
        the 3th paramater is null*/
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        
        //an array to hold the extension details
        /*it will hold:
        VK_KHR_device_group_creation
        VK_KHR_external_fence_capabilities
        VK_KHR_external_memory_capabilities
        VK_KHR_external_semaphore_capabilities
        VK_KHR_get_physical_device_properties2
        VK_KHR_get_surface_capabilities2
        VK_KHR_surface
        VK_KHR_surface_protected_capabilities
        VK_KHR_win32_surface
        VK_EXT_debug_report
        VK_EXT_debug_utils
        VK_EXT_swapchain_colorspace
        VK_NV_external_memory_capabilities
        VK_KHR_portability_enumeration
        VK_LUNARG_direct_driver_loading
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        */
        std::vector<VkExtensionProperties> extensions(extensionCount);
        //will put in the vector extensions all of the suported extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        //extensions_supported
#pragma endregion
#pragma region choosing_physicalDevice

        //the number of gpu we have
        uint32_t deviceCount = 0;
        //put in deviceCount the number of gpu we have(because the 3th parameter is nullptr)
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        //if the number of gpu is 0 then throw an error
        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        //holds all the gpu handles
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

   //we will not look for the best gpu because we only have one and he is NVIDIA GeForce GTX 1060 6GB

        if (!isDeviceSuitable(devices[0]))
        {
            throw std::runtime_error("gpu is not suitable!");
        }
        else
        {
            physicalDevice = devices[0];
        }
        //choosing_physicalDevice
#pragma endregion
#pragma region Window_surface_creation
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        //Window_surface_creation
#pragma endregion
#pragma region choosing_presentQueue_and_queueFamily

        uint32_t queueFamilyCount = 0;//number of queue familys(it will be 4)
        //puts in queueFamilyCount the number of queue familys(because the 3th parameter is nullptr)
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        /*we have 4 queues and the first queue has
       VK_QUEUE_GRAPHICS_BIT
       VK_QUEUE_COMPUTE_BIT
       VK_QUEUE_SPARSE_BINDING_BIT
       VK_QUEUE_TRANSFER_BIT
       */
       /*
      for (int i=0;i < queueFamilyCount;i++)
      {
          std::cout<<i<<std::endl;
          std::cout << "VK_QUEUE_GRAPHICS_BIT" << std::endl;
          std::cout << (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) << std::endl;
          std::cout << "VK_QUEUE_COMPUTE_BIT" << std::endl;
          std::cout << (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) << std::endl;
          std::cout << "VK_QUEUE_SPARSE_BINDING_BIT " << std::endl;
          std::cout << (queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) << std::endl;
          std::cout << "VK_QUEUE_TRANSFER_BIT " << std::endl;
          std::cout << (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) << std::endl;
      }
      */
      //holds all of the queue familys
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        //we choose the first queue to be the graphics queue
        graphicsQueueIndex = 0;

         /*
        for (int i = 0; i < queueFamilyCount; i++)
        {
            VkBool32 presentSupport = false;
            std::cout << i << std::endl;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if(presentSupport)
                std::cout << "this queueFamily has presentSupport" << std::endl;
            else
                std::cout << "this queueFamily doesnt has presentSupport" << std::endl;
        }

           0   i choose 0 to be my presentQueue
            yes
            1
            no
            2
            yes
            3
            no
        */
        //we choose the first queue to be the present queue
        /*the presentQueueIndex and the graphicsQueueIndex must be differnt.if you want them to be the
        same queue you need to create just one queue and he will be present and graphics queue*/
        presentQueueIndex = 2; 
       




        //choosing_presentQueue_and_queueFamily
#pragma endregion
#pragma region logical_device

        //will hold all of the queueCreateInfo to help us build the logical device
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;//setting the priority the same for all of my queues
       
        //creating a struct to fill information to create a logical device about the graphicsQueue
        VkDeviceQueueCreateInfo queueCreateInfo_graphicsQueue{};
        queueCreateInfo_graphicsQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //this is the first queue family =0
        queueCreateInfo_graphicsQueue.queueFamilyIndex = graphicsQueueIndex;
        queueCreateInfo_graphicsQueue.queueCount = 1;
        queueCreateInfo_graphicsQueue.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo_graphicsQueue);

        //creating a struct to fill information to create a logical device about the presentQueue
        VkDeviceQueueCreateInfo queueCreateInfo_presentQueue{};
        queueCreateInfo_presentQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //this is the first queue family =1
        queueCreateInfo_presentQueue.queueFamilyIndex = presentQueueIndex;
        queueCreateInfo_presentQueue.queueCount = 1;
        queueCreateInfo_presentQueue.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo_presentQueue);



        //creating a struct to fill information to create a logical device
        VkPhysicalDeviceFeatures deviceFeatures{};


        //creating a struct to fill information to create a logical device
        VkDeviceCreateInfo createInfo_for_device{};
        createInfo_for_device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        
        createInfo_for_device.queueCreateInfoCount = queueCreateInfos.size();
        createInfo_for_device.pQueueCreateInfos = queueCreateInfos.data();

        //Enabling device extensions
        createInfo_for_device.enabledExtensionCount = deviceExtensions.size();
        createInfo_for_device.ppEnabledExtensionNames = deviceExtensions.data();

        //filling the empty struct
        createInfo_for_device.pEnabledFeatures = &deviceFeatures;



        if (enableValidationLayers) {
            //setting validation layers
            createInfo_for_device.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo_for_device.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo_for_device.enabledLayerCount = 0;
        }
        //create a logical device
        if(vkCreateDevice(physicalDevice, &createInfo_for_device, nullptr, &logicalDevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        vkGetDeviceQueue(logicalDevice, graphicsQueueIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, presentQueueIndex, 0, &presentQueue);

        //logical_device
#pragma endregion 
#pragma region swap_chain
        //support details about the swap chain that we need to fill
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
       
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        }
      
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }
        //printing data:
/*
        std::cout << "the capabilities of the swap chain are" << std::endl;
        std::cout << "maxImageCount:" << details.capabilities.maxImageCount << std::endl;
        std::cout << "minImageCount:" << details.capabilities.minImageCount << std::endl;
        std::cout << "maxImageExtent.height:" << details.capabilities.maxImageExtent.height << std::endl;
        std::cout << "maxImageExtent.width:" << details.capabilities.maxImageExtent.width << std::endl;
        std::cout << "minImageExtent.height:" << details.capabilities.maxImageExtent.height << std::endl;
        std::cout << "minImageExtent.width:" << details.capabilities.maxImageExtent.width << std::endl;
        

        std::cout << "the supported formats are:" << std::endl;

        for (int i = 0; i < formatCount; i++)
        {
            std::cout << "format: " ;

            switch (details.formats[i].format)
            {
            case VK_FORMAT_B8G8R8A8_UNORM:
            {
                std::cout << "VK_FORMAT_B8G8R8A8_UNORM" << std::endl;
                break;
            }
            case VK_FORMAT_B8G8R8A8_SRGB:
            {
                std::cout << "VK_FORMAT_B8G8R8A8_SRGB" << std::endl;
                break;
            }
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            {
                std::cout << "VK_FORMAT_A2B10G10R10_UNORM_PACK32  " << std::endl;
                break;
            }
            default:
                std::cout << "unrecognized format:" << details.formats[i].format << std::endl;
                break;
            }

            std::cout << "SRGB color space is " ;
                
            switch (details.formats[i].colorSpace)
            {
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            {
                std::cout << "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR" << std::endl;
                break;
            }
            default:
                std::cout << "unrecognized code:"<< details.formats[i].colorSpace << std::endl;
                break;
            }


        }
        std::cout << "the supported present Modes are:" << std::endl;

        for (int i = 0; i < presentModeCount;i++)
        {
            switch (details.presentModes[i])
            {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
            {
                std::cout << "VK_PRESENT_MODE_IMMEDIATE_KHR is supported" << std::endl;
                break;

            }
            case VK_PRESENT_MODE_FIFO_KHR:
            {
                std::cout << "VK_PRESENT_MODE_FIFO_KHR is supported" << std::endl;
                break;

            }
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            {
                std::cout << "VK_PRESENT_MODE_FIFO_RELAXED_KHR is supported" << std::endl;
                break;

            }

            case VK_PRESENT_MODE_MAILBOX_KHR:
            {
                std::cout << "VK_PRESENT_MODE_MAILBOX_KHR is supported" << std::endl;
                break;
            }
            default:
                std::cout << "unrecognized code:" << details.presentModes[i] << std::endl;
                break;
            }
        }
      */
        if (details.formats.empty() || details.presentModes.empty())
        {
            throw std::runtime_error("do not support swap chain");
        }


      //those are the swap chain properties that we choose(without swapChainCapabilities/Extent)
        swapChainFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapChainFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
        swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

#pragma region choosing_swap_extent
        

//this is true when vulkan flag this value to tell us sometthing(but now it is false)
        if (details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            //  std::cout << details.capabilities.currentExtent.width; its 800
            swapChainExtent = details.capabilities.currentExtent;
        }
        else
        {
            throw std::runtime_error("details.capabilities.currentExtent is flaged so the extent is not correct");
        }
        //choosing_swap_chain_extent
#pragma endregion

#pragma region swapChainImageCount
        //if maxImageCount == 0 then there is no max limit on the swapChainImageCount
        if (details.capabilities.maxImageCount>0&& details.capabilities.minImageCount + 1> details.capabilities.maxImageCount)
        {
            swapChainImageCount = details.capabilities.minImageCount;
        }
        else
        {
            //this +1 more then the minimum like they say in the tutorial
            swapChainImageCount = details.capabilities.minImageCount + 1;
        }
        //swapChainImageCount
#pragma endregion

        //swap chain creation

        VkSwapchainCreateInfoKHR createInfoSwapChain{};
        createInfoSwapChain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfoSwapChain.surface = surface;
        createInfoSwapChain.minImageCount = swapChainImageCount;
        createInfoSwapChain.imageFormat = swapChainFormat.format;
        createInfoSwapChain.imageColorSpace = swapChainFormat.colorSpace;
        createInfoSwapChain.imageExtent = swapChainExtent;
        createInfoSwapChain.imageArrayLayers = 1;
        createInfoSwapChain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        //we used 2 different queues here
        if (graphicsQueueIndex != presentQueueIndex) {
            createInfoSwapChain.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfoSwapChain.queueFamilyIndexCount = 2;
            uint32_t queueFamilyIndices[] = { graphicsQueueIndex, presentQueueIndex };
            createInfoSwapChain.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfoSwapChain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfoSwapChain.queueFamilyIndexCount = 0; // Optional
            createInfoSwapChain.pQueueFamilyIndices = nullptr; // Optional
        }

        /*you can apply transformations inside the swap chain.
        we are telling that we dont want any transforms applying in the swapchain*/
            createInfoSwapChain.preTransform = details.capabilities.currentTransform;

        /*The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system. You'll almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.*/
            createInfoSwapChain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            createInfoSwapChain.presentMode = swapChainPresentMode;
            createInfoSwapChain.clipped = VK_TRUE;

            createInfoSwapChain.oldSwapchain = VK_NULL_HANDLE;

            //creating swap chain
            if (vkCreateSwapchainKHR(logicalDevice, &createInfoSwapChain, nullptr, &swapChain) != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }


            //Retrieving the swap chain images

            //swapChainImageCount will be change to the finual number that the implementation choosed
            vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, nullptr);
            swapChainImages.resize(swapChainImageCount);
            vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages.data());


        //swap_chain
#pragma endregion
#pragma region createImageViews
            
            swapChainImageViews.resize(swapChainImages.size());

            for (size_t i = 0; i < swapChainImages.size(); i++)
            {
                VkImageViewCreateInfo createInfoImageViews{};
                createInfoImageViews.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfoImageViews.image = swapChainImages[i];

                createInfoImageViews.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfoImageViews.format = swapChainFormat.format;

                createInfoImageViews.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfoImageViews.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfoImageViews.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfoImageViews.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                
                createInfoImageViews.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfoImageViews.subresourceRange.baseMipLevel = 0;
                createInfoImageViews.subresourceRange.levelCount = 1;
                createInfoImageViews.subresourceRange.baseArrayLayer = 0;
                createInfoImageViews.subresourceRange.layerCount = 1;

                if (vkCreateImageView(logicalDevice, &createInfoImageViews, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create image views!");
                }
            }
        
            
            

        //createImageViews
#pragma endregion

            createRenderPass();
            createGraphicsPipeline();
            createFramebuffers();
            createCommandPool;
            createCommandBuffer();
            createSyncObjects();
    }
    void createSyncObjects() 
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to begin recording command buffer!");
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

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    void drawFrame() 
    {
        vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(logicalDevice, 1, &inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    }
    void createCommandBuffer() 
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    void createCommandPool()
    {

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = graphicsQueueIndex;

        if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    void createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
        VkImageView attachments[] = { swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
        }
    }
    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


        VkAttachmentReference colorAttachmentRef{};
        //we only have one VkAttachmentDescription so its id is 0
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

        if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

    }
    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderModule;

    }
    void createGraphicsPipeline()
    {
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

        vertShaderStageInfo.module = vertShaderModule;
        /*the node of the function we want to inoke(you can use more then one)*/
        vertShaderStageInfo.pName = "main";


        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

       

        std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

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

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional


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
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

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
        //int an index
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional


        if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);


    }
    static std::vector<char> readFile(const std::string& filename) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
    /*always returns true.i need to add required property*///not in use
    bool isDeviceSuitable(const VkPhysicalDevice &device)
    {
       // VkPhysicalDeviceProperties deviceProperties;
       // vkGetPhysicalDeviceProperties(device, &deviceProperties);
       // VkPhysicalDeviceFeatures deviceFeatures;
       // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
       //std::cout << deviceProperties.deviceName;
        return checkDeviceExtensionSupport(device);
    }
    //not in use
    bool isDeviceFamilyQueueSuitable(const VkPhysicalDevice& device)
    {
      // uint32_t queueFamilyCount = 0;
      // vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
      //
      // std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      // vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        return true;
    }

    //checks if all of the requested layers are available
    bool checkValidationLayerSupport() 
    {
        //number of validation layers that we support(will hold 11)
        uint32_t layerCount;
        //when the secound input is nullptr it put in layerCount the number of validation layers that we support(will return 11)
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        //vector of all the validation layers that we support
        /*it will hold:
        VK_LAYER_NV_optimus
        VK_LAYER_VALVE_steam_overlay
        VK_LAYER_VALVE_steam_fossilize
        VK_LAYER_LUNARG_api_dump
        VK_LAYER_LUNARG_gfxreconstruct
        VK_LAYER_KHRONOS_synchronization2
        VK_LAYER_KHRONOS_validation
        VK_LAYER_LUNARG_monitor
        VK_LAYER_LUNARG_screenshot
        VK_LAYER_KHRONOS_profiles
        VK_LAYER_KHRONOS_shader_object
        for (int i = 0; i < layerCount;i++) {
            std::cout << '\t' << availableLayers[i].layerName << '\n';
        }
        */
        std::vector<VkLayerProperties> availableLayers(layerCount);
        //put in "availableLayers" all the validation layers that we support
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        //checks if all of the requested layers are available
        for (int i =0;i<validationLayers.size();i++)
        {
            bool have_all_validationLayers = false;
            for (int j = 0; j < layerCount; j++)
            {
            have_all_validationLayers = strcmp(availableLayers[j].layerName, validationLayers[i])==0;
                if (have_all_validationLayers)
                    break;
            }
            if (!have_all_validationLayers)
                return false;
        }

        return true;
    }
    bool checkDeviceExtensionSupport(const VkPhysicalDevice& device)
    {
        uint32_t extensionCount=0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        
        for (int i=0;i< deviceExtensions.size();i++)
        {
            bool have_all_extantions = false;
            for (int j=0;j< extensionCount;j++)
            {
     have_all_extantions = strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0;
            if (have_all_extantions)
                  {
                   break;
                  }
            }
            if (!have_all_extantions)
                {
                return false;
                }
            
        }

       

        return true;
    }
    void mainLoop() {
        while (!glfwWindowShouldClose(window))//It loops and checks for events like pressing the X button until the window has been closed by the user
        {
         glfwPollEvents();
         drawFrame();
        }
    }

    void cleanup() {
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, nullptr);
        vkDestroyFence(logicalDevice, inFlightFence, nullptr);

        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }
        vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
        vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(logicalDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
        vkDestroyDevice(logicalDevice, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);//destroy instamce
        glfwDestroyWindow(window);//destroy window
        glfwTerminate();//the reverse from initialize glfw "glfwInit()"
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
