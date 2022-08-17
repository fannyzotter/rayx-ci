#include "setupTests.h"

TEST_F(TestSuite, PlaneMirror) { compareAgainstRayUI("PlaneMirror"); }
TEST_F(TestSuite, PlaneMirrorDef) {
    compareAgainstRayUI("PlaneMirrorDef");

    // additional path length test
    auto rays = traceRML("PlaneMirrorDef");
    for (auto r : rays) {
        CHECK_IN(r.m_pathLength, 11000, 11001);
    }
}
TEST_F(TestSuite, PlaneMirrorMis) { compareAgainstRayUI("PlaneMirrorMis"); }

TEST_F(TestSuite, SphereMirrorDefault) {
    compareAgainstRayUI("SphereMirrorDefault",
                        1e-7);  // TODO(Rudi) this tolerance is not so great.
}
TEST_F(TestSuite, PlaneGratingDeviationDefault) {
    compareAgainstRayUI("PlaneGratingDeviationDefault");
}
TEST_F(TestSuite, PlaneGratingDeviationAz) {
    compareAgainstRayUI("PlaneGratingDeviationAz");
}
TEST_F(TestSuite, PlaneGratingDeviationAzMis) {
    compareAgainstRayUI("PlaneGratingDeviationAzMis");
}

TEST_F(TestSuite, BoringImagePlane) { compareAgainstRayUI("BoringImagePlane"); }

TEST_F(TestSuite, CylinderDefault) {
    compareAgainstRayUI("CylinderDefault", 1e-7);
}

TEST_F(TestSuite, PlaneGratingDevAzMisVLS) {
    compareAgainstRayUI("PlaneGratingDevAzMisVLS", 1e-7);
}
TEST_F(TestSuite, PlaneGratingIncAzMis) {
    compareAgainstRayUI("PlaneGratingIncAzMis");
}
TEST_F(TestSuite, ReflectionZonePlateAzim200) {
    compareAgainstRayUI("ReflectionZonePlateAzim200", 1e-7);
}
TEST_F(TestSuite, ReflectionZonePlateDefault) {
    compareAgainstRayUI("ReflectionZonePlateDefault");
}
TEST_F(TestSuite, ReflectionZonePlateDefault200) {
    compareAgainstRayUI("ReflectionZonePlateDefault200", 1e-7);
}
TEST_F(TestSuite, ReflectionZonePlateDefault200Toroid) {
    compareAgainstRayUI("ReflectionZonePlateDefault200Toroid", 1e-7);
}
TEST_F(TestSuite, ReflectionZonePlateMis) {
    compareAgainstRayUI("ReflectionZonePlateMis", 1e-7);
}

TEST_F(TestSuite, globalCoordinates_20rays) {
    compareAgainstRayUI("globalCoordinates_20rays");
}
TEST_F(TestSuite, pm_ell_ip_200mirrormis) {
    compareAgainstRayUI("pm_ell_ip_200mirrormis");
}

TEST_F(TestSuite, Ellipsoid) {
    auto rayx = traceRML("Ellipsoid", Filter::OnlySequentialRays);

    writeToOutputCSV(rayx, "Ellipsoid.rayx");
    CHECK_EQ(rayx.rayAmount(), 92);

    for (auto r : rayx) {
        CHECK_EQ(r.m_position, glm::dvec3(0, 0, 0), 1e-11);
    }
}

TEST_F(TestSuite, Slit) {
    auto rays = traceRML("slit");
    for (auto r : rays) {
        if (!intclose(r.m_weight, 1)) {
            continue;
        }

        CHECK_IN(abs(r.m_position.x), 0, 6);
        CHECK_IN(abs(r.m_position.y), 0.5, 1.3);
    }
}

TEST_F(TestSuite, toroid_MS) { compareAgainstRayUI("toroid_MS"); }
TEST_F(TestSuite, Ellipsoid_DGIA) { compareAgainstRayUI("Ellipsoid_DGIA"); }
/*
TEST_F(TestSuite, toroid_MS_T) { compareAgainstRayUI("toroid_MS_T"); }
TEST_F(TestSuite, toroid) { compareAgainstRayUI("toroid"); }
*/

// TODO(Rudi): fix and re-enable tests, Why do they fail?
/*
TEST_F(TestSuite, Ellipsoid_MB) { compareAgainstRayUI("Ellipsoid_MB"); }
TEST_F(TestSuite, Ellipsoid_PM) { compareAgainstRayUI("Ellipsoid_PM"); }

TEST_F(TestSuite, ellipsoid_ip_200ellipsmis_simpl) { // this is a temporary
_simpl test, it is the same as the next test but without the ImagePlane.
    compareAgainstRayUI("ellipsoid_ip_200ellipsmis_simpl");
}
TEST_F(TestSuite, ellipsoid_ip_200ellipsmis) {
    compareAgainstRayUI("ellipsoid_ip_200ellipsmis");
}
TEST_F(TestSuite, ellipsoid_ip_200mirrormis) {
    compareAgainstRayUI("ellipsoid_ip_200mirrormis");
}
*/