#include "Device.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::Rendering
{
namespace
{
VKAPI_ATTR VkBool32 VKAPI_CALL Validation(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                          VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        LOG_ERROR("Vulkan: {}", data->pMessage);
    else
        LOG_WARNING("Vulkan: {}", data->pMessage);
    return VK_FALSE;
}
} // namespace

Device::Device(SDL_Window* window)
{
    try
    {
        uint32_t           count    = 0;
        const char* const* required = SDL_Vulkan_GetInstanceExtensions(&count);
        PICO_ASSERT(required != nullptr, "{}", SDL_GetError());
        std::vector<const char*> extensions(required, required + count);
        std::vector<const char*> layers;
        bool                     validation = SDL_getenv("PICO_VULKAN_VALIDATION") != nullptr;
#ifndef NDEBUG
        validation = true;
#endif
        if (validation)
        {
            Check(vkEnumerateInstanceLayerProperties(&count, nullptr), "Enumerate layers");
            std::vector<VkLayerProperties> available(count);
            Check(vkEnumerateInstanceLayerProperties(&count, available.data()), "Enumerate layers");
            for (const auto& layer : available)
                if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
                    layers.push_back("VK_LAYER_KHRONOS_validation");
            Check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr), "Enumerate extensions");
            std::vector<VkExtensionProperties> instanceExtensions(count);
            Check(vkEnumerateInstanceExtensionProperties(nullptr, &count, instanceExtensions.data()), "Enumerate extensions");
            if (!layers.empty() && std::ranges::any_of(instanceExtensions, [](const auto& e) { return std::strcmp(e.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0; }))
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            else
                layers.clear();
            if (layers.empty()) LOG_WARNING("Vulkan validation unavailable");
        }
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "PicoEngine";
        app.pEngineName      = "PicoEngine";
        app.apiVersion       = VK_API_VERSION_1_0;
        VkDebugUtilsMessengerCreateInfoEXT debug{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debug.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug.pfnUserCallback = Validation;
        VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        info.pApplicationInfo        = &app;
        info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
        info.enabledLayerCount       = static_cast<uint32_t>(layers.size());
        info.ppEnabledLayerNames     = layers.data();
        info.pNext                   = layers.empty() ? nullptr : &debug;
        Check(vkCreateInstance(&info, nullptr, &m_instance), "Create instance");
        if (!layers.empty())
        {
            auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));
            Check(create(m_instance, &debug, nullptr, &m_debug), "Create debug messenger");
        }
        PICO_ASSERT(SDL_Vulkan_CreateSurface(window, m_instance, nullptr, &m_surface), "{}", SDL_GetError());
        Check(vkEnumeratePhysicalDevices(m_instance, &count, nullptr), "Enumerate GPUs");
        std::vector<VkPhysicalDevice> devices(count);
        Check(vkEnumeratePhysicalDevices(m_instance, &count, devices.data()), "Enumerate GPUs");
        for (auto physical : devices)
        {
            uint32_t n = 0;
            Check(vkEnumerateDeviceExtensionProperties(physical, nullptr, &n, nullptr), "Enumerate device extensions");
            std::vector<VkExtensionProperties> supported(n);
            Check(vkEnumerateDeviceExtensionProperties(physical, nullptr, &n, supported.data()), "Enumerate device extensions");
            if (!std::ranges::any_of(supported, [](const auto& e) { return std::strcmp(e.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0; })) continue;
            vkGetPhysicalDeviceQueueFamilyProperties(physical, &n, nullptr);
            std::vector<VkQueueFamilyProperties> queues(n);
            vkGetPhysicalDeviceQueueFamilyProperties(physical, &n, queues.data());
            for (uint32_t i = 0; i < n; ++i)
            {
                VkBool32 present = false;
                Check(vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, m_surface, &present), "Query presentation support");
                if (!(queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) || !present) continue;
                uint32_t formats = 0, modes = 0;
                Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physical, m_surface, &formats, nullptr), "Query formats");
                Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physical, m_surface, &modes, nullptr), "Query present modes");
                if (!formats || !modes) continue;
                m_physical    = physical;
                m_queueFamily = i;
                break;
            }
            if (m_physical) break;
        }
        PICO_ASSERT(m_physical != VK_NULL_HANDLE, "No Vulkan GPU supports graphics and presentation on a shared queue");
        float                   priority = 1.0f;
        VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue.queueFamilyIndex = m_queueFamily;
        queue.queueCount       = 1;
        queue.pQueuePriorities = &priority;
        std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkDeviceCreateInfo       device{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        device.queueCreateInfoCount    = 1;
        device.pQueueCreateInfos       = &queue;
        device.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
        device.ppEnabledExtensionNames = deviceExtensions.data();
        Check(vkCreateDevice(m_physical, &device, nullptr, &m_device), "Create device");
        vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);
        VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool.queueFamilyIndex = m_queueFamily;
        Check(vkCreateCommandPool(m_device, &pool, nullptr, &m_pool), "Create command pool");
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_physical, &properties);
        LOG_DEBUG("Vulkan device: {}", properties.deviceName);
    }
    catch (const std::exception& exception)
    {
        Destroy();
        PICO_ASSERT_FAIL("Vulkan device initialization failed: {}", exception.what());
    }
}

Device::~Device()
{
    Destroy();
}
void Device::Destroy() noexcept
{
    if (m_device)
    {
        vkDeviceWaitIdle(m_device);
        if (m_pool) vkDestroyCommandPool(m_device, m_pool, nullptr);
        vkDestroyDevice(m_device, nullptr);
    }
    if (m_surface) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    if (m_debug)
    {
        auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        destroy(m_instance, m_debug, nullptr);
    }
    if (m_instance) vkDestroyInstance(m_instance, nullptr);
}
void Device::WaitIdle() const
{
    Check(vkDeviceWaitIdle(m_device), "Wait for device");
}
uint32_t Device::FindMemory(uint32_t bits, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(m_physical, &memory);
    for (uint32_t i = 0; i < memory.memoryTypeCount; ++i)
        if ((bits & (1u << i)) && (memory.memoryTypes[i].propertyFlags & properties) == properties) return i;
    PICO_ASSERT_FAIL("No compatible Vulkan memory type");
}
void Device::Immediate(const std::function<void(VkCommandBuffer)>& record)
{
    VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc.commandPool        = m_pool;
    alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc.commandBufferCount = 1;
    VkCommandBuffer command;
    Check(vkAllocateCommandBuffers(m_device, &alloc, &command), "Allocate upload command");
    try
    {
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        Check(vkBeginCommandBuffer(command, &begin), "Begin upload");
        record(command);
        Check(vkEndCommandBuffer(command), "End upload");
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers    = &command;
        Check(vkQueueSubmit(m_queue, 1, &submit, VK_NULL_HANDLE), "Submit upload");
        Check(vkQueueWaitIdle(m_queue), "Wait for upload");
    }
    catch (const std::exception& exception)
    {
        vkFreeCommandBuffers(m_device, m_pool, 1, &command);
        PICO_ASSERT_FAIL("Immediate command submission failed: {}", exception.what());
    }
    vkFreeCommandBuffers(m_device, m_pool, 1, &command);
}
} // namespace PicoEngine::Rendering
