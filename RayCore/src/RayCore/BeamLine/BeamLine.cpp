#include "Beamline.h"
#include "Debug.h"

#include <iostream>

namespace RAY
{

    Beamline::Beamline()
    {
        DEBUG(std::cout << "Creating Beamline..." << std::endl);
        //std::vector<double> temp(16, 1);
        //BeamLineObject firstQuadric(temp);
        //m_objects.push_back(firstQuadric);
    }

    Beamline::~Beamline()
    {
        DEBUG(std::cout << "Deleting Beamline..." << std::endl);
    }

    void Beamline::addBeamlineObject(BeamLineObject newObject)
    {
        m_objects.push_back(newObject);
    }

    void Beamline::replaceNthObject(uint32_t index, BeamLineObject newObject)
    {
        assert(m_objects.size() >= index);
        m_objects[index] = newObject;
    }

    std::vector<BeamLineObject> Beamline::getObjects()
    {
        return m_objects;
    }

} // namespace RAY