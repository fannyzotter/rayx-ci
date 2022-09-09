#pragma once

#include "Core.h"
#include "Tracer/Tracer.h"
#include "VulkanEngine/VulkanEngine.h"

namespace RAYX {

class RAYX_API VulkanTracer : public Tracer {
  public:
    VulkanTracer();
    ~VulkanTracer() = default;

    RayList trace(const Beamline&) override;
#ifdef RAYX_DEBUG_MODE
    /**
     * @brief Get the Debug List containing the Debug Matrices
     * (Size heavy)
     *
     * @return std::vector<..> of Debug Struct (MAT4x4)
     */
    auto getDebugList() const { return m_debugBufList; }
#endif

  private:
    VulkanEngine m_engine;

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt
    // stf140 align rules (Stick to only 1 matrix for simplicity)
    struct _debugBuf_t {
        glm::dmat4x4 _dMat;  // Set to identiy matrix in shader.
    };
    std::vector<_debugBuf_t> m_debugBufList;
};
}  // namespace RAYX
