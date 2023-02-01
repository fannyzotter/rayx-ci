#include "VulkanEngine/VulkanEngine.h"

namespace RAYX {
void VulkanEngine::init(VulkanEngineInitSpec_t spec) {
    if (m_state != VulkanEngineStates_t::PREINIT) {
        RAYX_ERR << "VulkanEngine was already initialized!";
    }

    m_shaderfile = spec.shaderFileName;

    createInstance();
    setupDebugMessenger();
    pickDevice();
    createCommandPool();
    createCommandBuffers();
    createDescriptorSetLayout();
    createAllocateDescriptorPool(m_buffers.size());
    prepareVma();
    createStagingBuffer();
    createCache();

    m_state = VulkanEngineStates_t::PRERUN;
}

}  // namespace RAYX
