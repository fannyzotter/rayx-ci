#pragma once

#include "Device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>

class SwapChain {
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device& deviceRef, VkExtent2D windowExtent);
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    void operator=(const SwapChain&) = delete;

    VkFramebuffer getFrameBuffer(int index) const { return m_framebuffers[index]; }
    VkRenderPass getRenderPass() const { return m_RenderPass; }
    VkImageView getImageView(int index) const { return m_imageViews[index]; }
    size_t imageCount() const { return m_images.size(); }
    VkFormat getImageFormat() const { return m_ImageFormat; }
    VkExtent2D getExtent() const { return m_Extent; }
    uint32_t width() const { return m_Extent.width; }
    uint32_t height() const { return m_Extent.height; }

    float extentAspectRatio() { return static_cast<float>(m_Extent.width) / static_cast<float>(m_Extent.height); }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

  private:
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkFormat m_ImageFormat;
    VkExtent2D m_Extent;

    std::vector<VkFramebuffer> m_framebuffers;
    VkRenderPass m_RenderPass;

    std::vector<VkImage> m_depthImages;
    std::vector<VkDeviceMemory> m_depthImageMemorys;
    std::vector<VkImageView> m_depthImageViews;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    Device& m_Device;
    VkExtent2D m_WindowExtent;

    VkSwapchainKHR m_SwapChain;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame = 0;
};