#include "SphereGrating.h"

#include "Data/xml.h"
#include "Debug/Debug.h"
#include "Material/Material.h"
#include "Model/Surface/Quadric.h"

namespace RAYX {

SphereGrating::SphereGrating(const DesignObject& dobj) : OpticalElement(dobj) {
    m_designEnergyMounting = dobj.parseDesignEnergy();
    m_lineDensity = dobj.parseLineDensity();
    m_orderOfDiffraction = dobj.parseOrderDiffraction();
    m_vls = dobj.parseVls();

    m_gratingMount = dobj.parseGratingMount();
    Material mat = dobj.parseMaterial();
    auto matd = (double)static_cast<int>(mat);
    auto radius = dobj.parseRadius();
    setSurface(std::make_unique<Quadric>(glm::dmat4x4{1, 0, 0, 0, 1, 1, 0, -radius, 0, 0, 1, 0, 2, 0, matd, 0}));
}

double SphereGrating::getRadius() const { return m_radius; }

double SphereGrating::getExitArmLength() const { return m_exitArmLength; }
double SphereGrating::getEntranceArmLength() const { return m_entranceArmLength; }

double SphereGrating::getDeviation() const { return m_deviation; }
GratingMount SphereGrating::getGratingMount() const { return m_gratingMount; }
double SphereGrating::getDesignEnergyMounting() const { return m_designEnergyMounting; }
double SphereGrating::getLineDensity() const { return m_lineDensity; }
double SphereGrating::getOrderOfDiffraction() const { return m_orderOfDiffraction; }
double SphereGrating::getA() const { return m_a; }

glm::dmat4x4 SphereGrating::getElementParameters() const {
    return {0,
            0,
            m_lineDensity,
            m_orderOfDiffraction,
            abs(hvlam(m_designEnergyMounting)),
            0,
            m_vls[0],
            m_vls[1],
            m_vls[2],
            m_vls[3],
            m_vls[4],
            m_vls[5],
            0,
            0,
            0,
            0};
}

}  // namespace RAYX
