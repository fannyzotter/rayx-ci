#include <fstream>

#include "setupTests.h"

void checkEnergyDistribution(const std::vector<Ray>& rays, double photonEnergy, double energySpread) {
    for (auto r : rays) {
        CHECK_IN(r.m_energy, photonEnergy - energySpread, photonEnergy + energySpread);
    }
}

void checkZDistribution(const std::vector<Ray>& rays, double center, double spread) {
    for (auto r : rays) {
        CHECK_IN(r.m_position.z, center - spread, center + spread);
    }
}

void checkPositionDistribution(const std::vector<Ray>& rays, double sourceWidth, double sourceHeight) {
    for (auto r : rays) {
        CHECK_IN(r.m_position.x, -4.5 * sourceWidth, 4.5 * sourceWidth);
        CHECK_IN(r.m_position.y, -4.5 * sourceHeight, 4.5 * sourceHeight);
    }
}

void checkDirectionDistribution(const std::vector<Ray>& rays, double minAngle, double maxAngle) {
    for (auto r : rays) {
        // we ignore all non-FLY_OFF events, as they are in element-coordinates (and thus have another m_direction).
        if (r.m_eventType != ETYPE_FLY_OFF) {
            continue;
        }

        double psi = asin(r.m_direction.y);
        psi = abs(psi);
        CHECK_IN(psi, minAngle, maxAngle);
    }
}

void roughCompare(std::vector<RAYX::Ray> l, std::vector<RAYX::Ray> r) {
    CHECK_EQ(l.size(), r.size());
    // TODO maybe compare more?
    for (int i = 0; i < l.size(); i++) {
        CHECK_EQ(l[i].m_position, r[i].m_position);
        CHECK_EQ(l[i].m_direction, r[i].m_direction);
        CHECK_EQ(l[i].m_energy, r[i].m_energy);
    }
}

TEST_F(TestSuite, MatrixSource) {
    auto beamline = loadBeamline("MatrixSource");
    auto a = beamline.getInputRays();
    auto b = loadCSVRayUI("MatrixSource");
    roughCompare(a, b);
}

TEST_F(TestSuite, MatrixSourceMoved) {
    auto beamline = loadBeamline("MatrixSourceMoved");
    auto a = beamline.getInputRays();
    auto b = loadCSVRayUI("MatrixSource");
    for (auto& r : b) {
        r.m_position += glm::dvec3(5, -5, 3);
    }
    roughCompare(a, b);
}

/// this tests tracing an only-lightsource beamline. An error-prone edge case.
TEST_F(TestSuite, MatrixSourceTracedRayUI) {
    auto a = extractLastEvents(traceRML("MatrixSource"));
    auto b = loadCSVRayUI("MatrixSource");
    roughCompare(a, b);
}
TEST_F(TestSuite, MatrixSourceTraced) { compareAgainstCorrect("MatrixSource"); }

TEST_F(TestSuite, PointSourceHardEdge) {
    auto rays = loadBeamline("PointSourceHardEdge").getInputRays();
    checkEnergyDistribution(rays, 120.97, 12.1);
}

TEST_F(TestSuite, PointSourceSoftEdge) {
    auto rays = loadBeamline("PointSourceSoftEdge").getInputRays();
    checkEnergyDistribution(rays, 151, 6);
}

TEST_F(TestSuite, MatrixSourceEnergyDistribution) {
    auto rays = loadBeamline("MatrixSourceSpreaded").getInputRays();
    checkEnergyDistribution(rays, 42, 10);
}

TEST_F(TestSuite, DipoleSourcePosition) {
    auto rays = loadBeamline("dipole_plain").getInputRays();
    checkPositionDistribution(rays, 0.065, 0.04);
}

TEST_F(TestSuite, DipoleEnergyDistribution) {
    auto rays = loadBeamline("dipole_energySpread").getInputRays();
    checkEnergyDistribution(rays, 1000, 23000);
}

TEST_F(TestSuite, PixelPositionTest) {
    auto beamline = loadBeamline("PixelSource");
    auto rays = beamline.getInputRays();
    std::shared_ptr<LightSource> src = beamline.m_LightSources[0];
    auto* pixelsource = dynamic_cast<PixelSource*>(&*src);
    auto width = src->getSourceWidth();
    auto height = src->getSourceHeight();
    auto hordiv = src->getHorDivergence();
    for (auto ray : rays) {
        CHECK_IN(abs(ray.m_position.x), width / 6.0, width / 2.0);
        CHECK_IN(abs(ray.m_position.y), height / 6.0, height / 2.0);
        double phi = atan2(ray.m_direction.x, ray.m_direction.z);  // phi in rad from m_direction
        CHECK_IN(abs(phi), 0.0, hordiv / 2.0);
    }
}

TEST_F(TestSuite, DipoleZDistribution) {
    auto beamline = loadBeamline("dipole_plain");
    std::shared_ptr<LightSource> src = beamline.m_LightSources[0];
    auto* dipoleSource = dynamic_cast<DipoleSource*>(&*src);  // TODO : Not used

    auto rays = beamline.getInputRays();
    checkZDistribution(rays, 0, 2.2);
}

TEST_F(TestSuite, CircleSourcetest) {
    auto rays = loadBeamline("CircleSource_default").getInputRays();
    checkPositionDistribution(rays, 0.065, 0.04);
    checkEnergyDistribution(rays, 99.5, 100.5);
}

TEST_F(TestSuite, testCircleSourceDirections) {
    auto bundle = traceRML("CircleSource_default");
    for (auto rays : bundle) {
        checkDirectionDistribution(rays, 0.0, 105.0);
    }
}

TEST_F(TestSuite, testInterpolationFunctionDipole) {
    struct InOutPair {
        double in;
        double out;
    };
    std::vector<InOutPair> inouts = {
        {
            .in = 1.5298292375594387,
            .out = -3.5010758381905855,
        },
        {
            .in = 2,
            .out = -6.0742663050458416,
        },
        {
            .in = -1,
            .out = -0.095123518041340588,
        },
    };

    auto beamline = loadBeamline("dipole_plain");
    std::shared_ptr<LightSource> src = beamline.m_LightSources[0];
    auto* dipolesource = dynamic_cast<DipoleSource*>(&*src);

    for (auto values : inouts) {
        auto result = dipolesource->getInterpolation(values.in);
        CHECK_EQ(result, values.out, 0.01);
    }
}

TEST_F(TestSuite, testVerDivergenceDipole) {
    struct InOutPair {
        double energy;
        double sigv;
        double out;
    };
    std::vector<InOutPair> inouts = {{
        .energy = 100,
        .sigv = 1,
        .out = 1.591581814000419,
    }};

    auto beamline = loadBeamline("dipole_plain");
    std::shared_ptr<LightSource> src = beamline.m_LightSources[0];
    auto* dipolesource = dynamic_cast<DipoleSource*>(&*src);

    for (auto values : inouts) {
        auto result = dipolesource->vDivergence(values.energy, values.sigv);
        CHECK_EQ(result, values.out, 0.1);
    }
}

TEST_F(TestSuite, testLightsourceGetters) {
    struct RmlInput {
        std::string rmlFile;
        double horDivergence;
        double sourceHeight;
        double sourceWidth;
        double sourceDepth;
        double averagePhotonEnergy;
    };

    std::vector<RmlInput> rmlinputs = {{
        .rmlFile = "PointSourceHardEdge",
        .horDivergence = 0.001,  // conversion /1000 in the parser
        .sourceHeight = 0.04,
        .sourceWidth = 0.065,
        .sourceDepth = 1,
        .averagePhotonEnergy = 120.97,
    },{
        .rmlFile = "simpleUndulator",
        .horDivergence = 4.6528002321182891e-05,  // conversion /1000 in the parser
        .sourceHeight = 0.053499116288275229,
        .sourceWidth = 0.22173963435440763,
        .sourceDepth = 1,
        .averagePhotonEnergy = 100,
    }};

    for (auto values : rmlinputs) {
        auto beamline = loadBeamline(values.rmlFile);
        std::shared_ptr<LightSource> src = beamline.m_LightSources[0];
        auto* lightSource = dynamic_cast<LightSource*>(&*src);

        auto horResult = lightSource->getHorDivergence();
        auto heightResult = lightSource->getSourceHeight();
        auto widthResult = lightSource->getSourceWidth();
        auto average = lightSource->getPhotonEnergy();

        CHECK_EQ(horResult, values.horDivergence);
        CHECK_EQ(heightResult, values.sourceHeight);
        CHECK_EQ(widthResult, values.sourceWidth);
        CHECK_EQ(average, values.averagePhotonEnergy);
    }
}


