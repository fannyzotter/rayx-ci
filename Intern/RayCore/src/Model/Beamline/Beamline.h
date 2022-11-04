#pragma once

#include <Material/Material.h>

#include <memory>
#include <vector>

#include "Core.h"
#include "Tracer/Ray.h"

namespace RAYX {
class OpticalElement;
class LightSource;

/*
 * The Beamline class is a container for OpticalElements and LightSources.
 * It represents a central structure in our simulation process.
 */
// TODO(Jannis): after we reworked the beamlineobjects we want to have
// m_OpticalElements and m_LightSources private again and have a sensible
// constructor for this class.
class RAYX_API Beamline {
  public:
    Beamline();
    ~Beamline();

    std::vector<Ray> getInputRays() const;

    /**
     * @brief Quality-of-life function to calculate the smallest possible
     * MaterialTables which cover all materials from this beamline
     *
     * @return MaterialTables
     */
    MaterialTables calcMinimalMaterialTables() const;

    std::vector<std::shared_ptr<OpticalElement>> m_OpticalElements;
    std::vector<std::shared_ptr<LightSource>> m_LightSources;
};

}  // namespace RAYX
