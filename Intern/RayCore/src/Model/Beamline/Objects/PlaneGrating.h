#pragma once
#include <Data/xml.h>
#include <Tracer/Vulkan/Material.h>

#include "Model/Beamline/OpticalElement.h"
#include "Model/Surface/Quadric.h"

namespace RAYX {

class RAYX_API PlaneGrating : public OpticalElement {
  public:
    PlaneGrating(const char* name, Geometry::GeometricalShape geometricalShape,
                 const double width, const double height,
                 const double azimuthalAngle, glm::dvec4 position,
                 glm::dmat4x4 orientation, const double designEnergyMounting,
                 const double lineDensity, const double orderOfDiffraction,
                 const int additionalZeroOrder, const std::vector<double> vls,
                 const std::vector<double> slopeError, Material mat);
    PlaneGrating();
    ~PlaneGrating();

    static std::shared_ptr<PlaneGrating> createFromXML(
        rapidxml::xml_node<>*, const std::vector<xml::Group>& group_context);

    double getFixFocusConstantCFF();
    double getDesignEnergyMounting();
    double getLineDensity();
    double getOrderOfDiffraction();
    std::vector<double> getVls();

  private:
    int m_additionalOrder;
    double m_designEnergyMounting;
    double m_lineDensity;
    double m_orderOfDiffraction;
    std::vector<double> m_vls;
};

}  // namespace RAYX
