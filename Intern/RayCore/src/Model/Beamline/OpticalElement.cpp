#include "OpticalElement.h"

#include <cmath>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "Debug.h"
#include "utils.h"

namespace RAYX {
/**
 * constructor for adding elements to tracer. The given arrays contain the
 * values that will actually be moved to the shader
 * @param name                  name of the element
 * @param surfaceParams         parameters that define the surface of the
 * element
 * @param inputInMatrix         16x16 world coordinate to element coordinate
 * transformation matrix
 * @param inputOutMatrix        16x16 element coordinate to world coordinate
 * transformation matrix
 * @param OParameters           Object parameters (width, height, slopeError..)
 * something all elements have
 * @param EParameters           Element specific parameters, depend on which
 * element it is
 */
OpticalElement::OpticalElement(const char* name,
                               const std::array<double, 4 * 4> surfaceParams,
                               const std::array<double, 4 * 4> inputInMatrix,
                               const std::array<double, 4 * 4> inputOutMatrix,
                               const std::array<double, 4 * 4> OParameters,
                               const std::array<double, 4 * 4> EParameters)
    : m_name(name), m_surfaceParams(surfaceParams),
      m_objectParameters(OParameters), m_elementParameters(EParameters){
    m_Geometry = std::make_unique<Geometry>();
    m_Geometry->m_inMatrix = arrayToGlm16(inputInMatrix);
    m_Geometry->m_outMatrix = arrayToGlm16(inputOutMatrix);
}

/* NEW CONSTRUCTORS */

/**
 * @param name                      name of the element
 * @param EParameters               Element specific parameters
 * @param geometricalShape          geometrical Shape of element (0 = rectangle,
 * 1 = elliptical)
 * @param width                     x-dimension of element
 * @param height                    z-dimension of element
 * @param position                  position in world coordinates
 * @param orientation               orientation in world coordinate system
 * @param slopeError                slope error parameters
 */
OpticalElement::OpticalElement(
    const char* name, const std::array<double, 4 * 4> EParameters,
    OpticalElement::GeometricalShape geometricalShape, const double width,
    const double height, const double azimuthalAngle, glm::dvec4 position,
    glm::dmat4x4 orientation, const std::array<double, 7> slopeError)
    : m_name(name), m_slopeError(slopeError), m_elementParameters(EParameters) {
    m_Geometry = std::make_unique<Geometry>();
    m_Geometry->m_geometricalShape = geometricalShape;
    if (m_Geometry->m_geometricalShape == GeometricalShape::ELLIPTICAL) {
        m_Geometry->m_widthA = -width;
        m_Geometry->m_height = -height;
    } else {
        m_Geometry->m_widthA = width;
        m_Geometry->m_height = height;
    }
    m_Geometry->m_azimuthalAngle = azimuthalAngle;
    m_Geometry->m_position = position;
    m_Geometry->m_orientation = orientation;
    m_Geometry->calcTransformationMatrices(position, orientation);
    updateObjectParams();
}

// ! temporary constructors for trapezoid (10/11/2021)
OpticalElement::OpticalElement(
    const char* name, const std::array<double, 4 * 4> EParameters,
    OpticalElement::GeometricalShape geometricalShape, const double width,
    const double widthB, const double height, const double azimuthalAngle,
    glm::dvec4 position, glm::dmat4x4 orientation,
    const std::array<double, 7> slopeError)
    : m_name(name), m_slopeError(slopeError), m_elementParameters(EParameters) {
    m_Geometry = std::make_unique<Geometry>();
    m_Geometry->m_geometricalShape = geometricalShape;
    if (m_Geometry->m_geometricalShape == GeometricalShape::ELLIPTICAL) {
        m_Geometry->m_widthA = -width;
        m_Geometry->m_height = -height;
    } else {
        m_Geometry->m_widthA = width;
        m_Geometry->m_height = height;
    }
    m_Geometry->m_widthB = widthB;
    m_Geometry->m_azimuthalAngle = azimuthalAngle;
    m_Geometry->m_position = position;
    m_Geometry->m_orientation = orientation;
    m_Geometry->calcTransformationMatrices(position, orientation);
    updateObjectParams();
}
OpticalElement::OpticalElement(
    const char* name, OpticalElement::GeometricalShape geometricalShape,
    const double widthA, const double widthB, const double height,
    const double azimuthalAngle, glm::dvec4 position, glm::dmat4x4 orientation,
    const std::array<double, 7> slopeError)
    : m_name(name), m_slopeError(slopeError) {
    m_Geometry = std::make_unique<Geometry>();
    m_Geometry->m_geometricalShape = geometricalShape;
    if (m_Geometry->m_geometricalShape == GeometricalShape::ELLIPTICAL) {
        m_Geometry->m_widthA = -widthA;
        m_Geometry->m_height = -height;
    } else {
        m_Geometry->m_widthA = widthA;
        m_Geometry->m_height = height;
    }
    m_Geometry->m_widthB = widthB;
    m_Geometry->m_azimuthalAngle = azimuthalAngle;
    m_Geometry->m_position = position;
    m_Geometry->m_orientation = orientation;
    m_Geometry->calcTransformationMatrices(position, orientation);
    updateObjectParams();
}

/**
 * @param name                      name of the element
 * @param geometricalShape          geometrical Shape of element (0 = rectangle,
 * 1 = elliptical)
 * @param width                     x-dimension of element
 * @param height                    z-dimension of element
 * @param position                  position in world coordinates
 * @param orientation               orientation in world coordinate system
 * @param slopeError                slope error parameters
 */
OpticalElement::OpticalElement(
    const char* name, OpticalElement::GeometricalShape geometricalShape,
    const double width, const double height, const double azimuthalAngle,
    glm::dvec4 position, glm::dmat4x4 orientation,
    const std::array<double, 7> slopeError)
    : m_name(name), m_slopeError(slopeError) {
    m_Geometry = std::make_unique<Geometry>();
    m_Geometry->m_geometricalShape = geometricalShape;
    if (m_Geometry->m_geometricalShape == GeometricalShape::ELLIPTICAL) {
        m_Geometry->m_widthA = -width;
        m_Geometry->m_height = -height;
    } else {
        m_Geometry->m_widthA = width;
        m_Geometry->m_height = height;
    }
    m_Geometry->m_azimuthalAngle = azimuthalAngle;
    m_Geometry->m_position = position;
    m_Geometry->m_orientation = orientation;
    m_Geometry->calcTransformationMatrices(position, orientation);
    updateObjectParams();
}

OpticalElement::OpticalElement() {}

OpticalElement::~OpticalElement() {}

void OpticalElement::setElementParameters(std::array<double, 4 * 4> params) {
    m_elementParameters = params;
}

void OpticalElement::setInMatrix(std::array<double, 4 * 4> inputMatrix) {
    m_Geometry->m_inMatrix = glm::make_mat4x4(&inputMatrix[0]);
}
void OpticalElement::setOutMatrix(std::array<double, 4 * 4> inputMatrix) {
    m_Geometry->m_outMatrix = glm::make_mat4x4(&inputMatrix[0]);
}

void OpticalElement::setSurface(std::unique_ptr<Surface> surface) {
    m_surfacePtr = std::move(surface);
    if (surface) {
        RAYX_ERR << "surface should be nullptr after move!";
    }
    if (!m_surfacePtr) {
        RAYX_ERR << "m_surfacePtr should NOT be nullptr!";
    }
}

// ! temporary adjustment for trapezoid (10/11/2021)
void OpticalElement::updateObjectParams() {
    double widthA = m_Geometry->m_widthA;
    double widthB = m_Geometry->m_widthB;

    m_objectParameters = {widthA,                // shader:  [0][0]
                          m_Geometry->m_height,  // [0][1]
                          m_slopeError[0],
                          m_slopeError[1],
                          m_slopeError[2],  // [1][0]
                          m_slopeError[3],
                          m_slopeError[4],
                          m_slopeError[5],
                          m_slopeError[6],  // [2][0]
                          widthB,
                          m_Geometry->m_azimuthalAngle,
                          0,
                          0,  // [3][0]
                          0,
                          0,
                          0};
}

// ! temporary adjustment for trapezoid (10/11/2021)
double OpticalElement::getWidth() { return m_Geometry->m_widthA; }

double OpticalElement::getHeight() { return m_Geometry->m_height; }

// TODO(Jannis): make these return a glm::dvec4
std::array<double, 4 * 4> OpticalElement::getInMatrix() const {
    return glmToArray16(m_Geometry->m_inMatrix);
}
std::array<double, 4 * 4> OpticalElement::getOutMatrix() const {
    return glmToArray16(m_Geometry->m_outMatrix);
}
glm::dvec4 OpticalElement::getPosition() const {
    return m_Geometry->m_position;
}
glm::dmat4x4 OpticalElement::getOrientation() const {
    return m_Geometry->m_orientation;
}
std::array<double, 4 * 4> OpticalElement::getObjectParameters() {
    return m_objectParameters;
}

std::array<double, 4 * 4> OpticalElement::getElementParameters() const {
    return m_elementParameters;
}

std::array<double, 4 * 4> OpticalElement::getSurfaceParams() const {
    RAYX_LOG << "return anchor points";
    // assert(m_surfacePtr!=nullptr);
    if (m_surfacePtr != nullptr)
        return m_surfacePtr->getParams();
    else
        return m_surfaceParams;
}

std::array<double, 7> OpticalElement::getSlopeError() const {
    return m_slopeError;
}

}  // namespace RAYX
