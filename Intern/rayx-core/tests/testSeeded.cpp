#include "Random.h"
#include "setupTests.h"

// This module tests beamlines with randomness.
// As it's hard to check strict properties for a randomly generated output, we choose a fixed seed approach.
//
// in this module tests mostly compare rayx's output with an older "correct" output from rayx, which was manually checked by a physicist.
//
// Whenever something in the randomness changes, the test will fail and needs to be manually reevaluated.
// In that case, also the corresponding .csv file needs to be updated.
// Note: This .csv file can simply be generated by calling rayx with `-i input/<test>_seeded.rml -c -f`
//
// Additionally to randomness tests, there can also be (non-randomized) regression tests in this module.
// Which compares the current rayx the output of a previous rayx output which we deemed correct.

TEST_F(TestSuite, randomNumbers) {
    RAYX::fixSeed(RAYX::FIXED_SEED);
    for (int i = 0; i < 4; i++) {
        std::cout << randomDouble() << std::endl;
    }

    RAYX::fixSeed(RAYX::FIXED_SEED);
    double r;
    r = randomDouble();
    CHECK_EQ(r, 0.37454011439684315);
    r = randomDouble();
    CHECK_EQ(r, 0.79654298438610116);
    r = randomDouble();
    CHECK_EQ(r, 0.95071431178383392);
    r = randomDouble();
    CHECK_EQ(r, 0.1834347877147223);
}

TEST_F(TestSuite, randomNumbers_normal) {
    RAYX::fixSeed(RAYX::FIXED_SEED);
    for (int i = 0; i < 4; i++) {
        std::cout << randomNormal(0, 1) << std::endl;
    }

    RAYX::fixSeed(RAYX::FIXED_SEED);
    double r;
    r = randomNormal(0, 1);
    CHECK_EQ(r, 0.40402608730396244);
    r = randomNormal(0, 1);
    CHECK_EQ(r, 0.12913107166033977);
    r = randomNormal(0, 1);
    CHECK_EQ(r, 0.14650860797333812);
    r = randomNormal(0, 1);
    CHECK_EQ(r, -0.83114042984660008);
}

TEST_F(TestSuite, PointSource_seeded) { compareAgainstCorrect("PointSource_seeded"); }

// Tests sourceDepth of MatrixSource.
TEST_F(TestSuite, MatrixSource_seeded) { compareAgainstCorrect("MatrixSource_seeded"); }

// Tests reflectivity of materials of a PlaneMirror.
TEST_F(TestSuite, PlaneMirror_refl_seeded) { compareAgainstCorrect("PlaneMirror_refl_seeded"); }

// Tests the Energy Distribution of a MatrixSource.
TEST_F(TestSuite, MatrixSource_distr_seeded) { compareAgainstCorrect("MatrixSource_distr_seeded"); }

//TEST_F(TestSuite, slit1_seeded) { compareAgainstCorrect("slit1_seeded"); }
//TEST_F(TestSuite, slit2_seeded) { compareAgainstCorrect("slit2_seeded"); }
//TEST_F(TestSuite, slit3_seeded) { compareAgainstCorrect("slit3_seeded"); }
//TEST_F(TestSuite, slit4_seeded) { compareAgainstCorrect("slit4_seeded"); }
