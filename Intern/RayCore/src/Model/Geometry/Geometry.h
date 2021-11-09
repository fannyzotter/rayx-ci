#pragma once

#include "Core.h"
#include <glm.hpp>
#include <vector>

namespace RAYX
{

    class RAYX_API Geometry {

    public:
        enum class GeometricalShape {
            RECTANGLE = 0,
            ELLIPTICAL,
            TRAPEZOID
        }; ///< influences wastebox function in shader

        Geometry(GeometricalShape geometricShape, double width, double height, glm::dvec4 position, glm::dmat4x4 orientation);
        Geometry();
        ~Geometry();

        double getWidth();
        double getHeight();
        std::vector<double> getInMatrix();
        std::vector<double> getOutMatrix();
        void setInMatrix(std::vector<double> inputMatrix);
        void setOutMatrix(std::vector<double> inputMatrix);
        void calcTransformationMatrices(glm::dvec4 position, glm::dmat4x4 orientation);



    private:
        double m_width;
        double m_height;
        std::vector<double> m_inMatrix;
        std::vector<double> m_outMatrix;
        GeometricalShape m_geometricalShape;
    };
}