#pragma once

#include <Data/xml.h>

#include "Model/Beamline/LightSource.h"

namespace RAYX {

class RAYX_API PointSource : public LightSource {
  public:
    PointSource() = default;
    PointSource(const DesignObject&);
    virtual ~PointSource() = default;

    std::vector<Ray> getRays() const override;

  private:
    // Geometric Parameters
    SourceDist m_widthDist;
    SourceDist m_heightDist;
    SourceDist m_horDist;
    SourceDist m_verDist;
    // std::vector<SOURCE_LENGTH> m_source_lengths;
};

}  // namespace RAYX
