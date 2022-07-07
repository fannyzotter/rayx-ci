#include "Tracer/CpuTracer.h"
#include "setupTests.h"

// TODO(rudi): shader tests

namespace RAYX {
namespace CPP_TRACER {
double r8_exp(double);
double r8_log(double);
double squaresDoubleRNG(uint64_t&);
Ray refrac2D(Ray, glm::dvec4, double, double);
}  // namespace CPP_TRACER
}  // namespace RAYX

using namespace RAYX;

TEST_F(TestSuite, testUniformRandom) {
    uint64_t ctr = 13;
    double old = 0;

    for (int i = 0; i < 100; i++) {
        double d = CPP_TRACER::squaresDoubleRNG(ctr);
        if (d == old) {
            RAYX_WARN << "repeating number in testUniformRandom! " << d;
        }
        if (d < 0.0 || d > 1.0) {
            RAYX_ERR << "random number out of range [0, 1]: " << d;
        }
        old = d;
    }
}

TEST_F(TestSuite, ExpTest) {
    std::vector<double> args = {10.0, 5.0, 2.0, 1.0, 0.5, 0.0001, 0.0};
    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_exp(x), exp(x));
        CHECK_EQ(CPP_TRACER::r8_exp(-x), exp(-x));
    }
}

// interestingly r8_log and log behave differently on input 0, namely 1 vs -inf.
TEST_F(TestSuite, LogTest) {
    std::vector<double> args = {10.0, 5.0, 2.0, 1.0, 0.5, 0.0001, 0.0000001};
    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_log(x), log(x));
    }
}

TEST_F(TestSuite, testRefrac2D) {
    std::vector<Ray> input = {
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(0.0001666666635802469, -0.017285764670739875,
                           0.99985057611723738),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(0.00016514977645243345, 0.012830838024391771, 0, 0),
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(0.00049999999722222276, -0.017285762731583675,
                           0.99985046502305308),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(-6.2949352042540596e-05, 0.038483898782123105, 0, 0),
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(0.0001666666635802469, -0.017619047234249029,
                           0.99984475864845179),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(-0.077169530850327184, 0.2686127340088395, 0, 0),
        },
        {
            .m_position = glm::dvec3(0.050470500672820856, 0.95514062789960541,
                                     -0.29182033770349547),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.00049999991666666705, -0.016952478247434233,
                           0.99985617139734351),
            .m_energy = 0,
            .m_stokes =
                glm::dvec4(0.0021599283476277926, -0.050153240660177005, 0, 0),
        },
    };

    // the correct rays should only be altered in weight & direction.
    std::vector<Ray> correct = input;

    correct[0].m_weight = 1;
    correct[0].m_direction = glm::dvec3(
        -0.012664171360811521, 0.021648721107426414, 0.99968542634078494);

    correct[1].m_weight = 0;
    correct[1].m_direction = glm::dvec3(
        0.00049999999722222276, -0.017285762731583675, 0.99985046502305308);

    correct[2].m_weight = 0;
    correct[2].m_direction = glm::dvec3(
        0.0001666666635802469, -0.017619047234249029, 0.99984475864845179);

    correct[3].m_weight = 1;
    correct[3].m_direction = glm::dvec3(
        0.080765992839840872, 0.57052382524991363, 0.81730007905468893);

    for (uint i = 0; i < input.size(); i++) {
        auto r = input[i];

        glm::dvec4 normal = glm::dvec4(r.m_position, 0);
        double az = r.m_stokes.x;
        double ax = r.m_stokes.y;

        auto out = CPP_TRACER::refrac2D(r, normal, az, ax);

        CHECK_EQ(out, correct[i]);
    }
}
