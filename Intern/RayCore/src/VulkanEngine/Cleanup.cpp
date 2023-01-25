#include <vk_mem_alloc.h>

#include "VulkanEngine/VulkanEngine.h"

namespace RAYX {

void VulkanEngine::cleanup() {
    if (m_state != VulkanEngineStates_t::POSTRUN) {
        RAYX_ERR << "cleanup() only needs to be called after .run()!";
    }

    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

    for (auto& [name, buf] : m_buffers) {
        // vkDestroyBuffer(m_Device, buf.m_Buffer, nullptr);
        // vkFreeMemory(m_Device, buf.m_Memory, nullptr);
        vmaDestroyBuffer(m_VmaAllocator, buf.buf, buf.alloca);
    }

    vkDestroyShaderModule(m_Device, m_ComputeShaderModule, nullptr);

    m_state = VulkanEngineStates_t::PRERUN;
}

}  // namespace RAYX
