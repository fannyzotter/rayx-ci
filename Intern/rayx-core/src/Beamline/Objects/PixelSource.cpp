#include "PixelSource.h"

#include "Data/xml.h"
#include "Debug/Debug.h"
#include "Debug/Instrumentor.h"
#include "Random.h"
#include "Shared/Constants.h"

namespace RAYX {

PixelSource::PixelSource(const DesignObject& dobj) : LightSource(dobj) {
    m_verDivergence = dobj.parseVerDiv();
    m_horDivergence = dobj.parseHorDiv();
    m_sourceDepth = dobj.parseSourceDepth();

    m_linearPol_0 = dobj.parseLinearPol0();
    m_linearPol_45 = dobj.parseLinearPol45();
    m_circularPol = dobj.parseCircularPol();
}

/**
 * get deviation from main ray according to specified distribution 
 * (uniform or Thrids for the x, y position)) 
 * and extent (eg specified width/height of source)
 */
double getPosInDistribution(SourceDist l, double extent) {
    if (l == SourceDist::Uniform) {
        return (randomDouble() - 0.5) * extent;
    } else if (l == SourceDist::Thirds) {
        double temp = (randomDouble() - 0.5) * 2/3 * extent;
        return temp + copysign(1.0, temp) * 1/6 * extent;
    } else {
        return 0;
    }
}

/**
 * Creates random rays from pixel source with specified distributed width
 * & height in 4 distinct pixels
 * position and directions are distributed uniform
 *
 * @returns list of rays
 */
std::vector<Ray> PixelSource::getRays() const {
    RAYX_PROFILE_FUNCTION_STDOUT();
    double x, y, z, psi, phi, en;  // x,y,z pos, psi,phi direction cosines, en=energy

    int n = m_numberOfRays;
    std::vector<Ray> rayList;
    rayList.reserve(m_numberOfRays);

    // create n rays with random position and divergence within the given span
    // for width, height, depth, horizontal and vertical divergence
    for (int i = 0; i < n; i++) {
        x = getPosInDistribution(SourceDist::Thirds, m_sourceWidth);
        x += m_position.x;
        y = getPosInDistribution(SourceDist::Thirds, m_sourceHeight);
        y += m_position.y;
        z = getPosInDistribution(SourceDist::Uniform, m_sourceDepth);
        z += m_position.z;
        en = selectEnergy();  // LightSource.cpp
        // double z = (rn[2] - 0.5) * m_sourceDepth;
        glm::dvec3 position = glm::dvec3(x, y, z);

        // get random deviation from main ray based on divergence
        psi = getPosInDistribution(SourceDist::Uniform, m_verDivergence);
        phi = getPosInDistribution(SourceDist::Uniform, m_horDivergence);
        // get corresponding angles based on distribution and deviation from
        // main ray (main ray: xDir=0,yDir=0,zDir=1 for phi=psi=0)
        glm::dvec3 direction = getDirectionFromAngles(phi, psi);
        glm::dvec4 tempDir = m_orientation * glm::dvec4(direction, 0.0);
        direction = glm::dvec3(tempDir.x, tempDir.y, tempDir.z);
        glm::dvec4 stokes = glm::dvec4(1, m_linearPol_0, m_linearPol_45, m_circularPol);

        Ray r = {position, ETYPE_UNINIT, direction, en, stokes, 0.0, 0.0, -1.0, -1.0};

        rayList.push_back(r);
    }
    return rayList;
}

}  // namespace RAYX
