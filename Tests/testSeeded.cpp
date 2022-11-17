#include "Random.h"
#include "setupTests.h"

// This module tests beamlines with randomness.
// As it's hard to check strict properties for a randomly generated output, we choose a fixed seed approach.
//
// in this module tests mostly compare RAY-X's output with an older "correct" output from RAY-X, which was manually checked by a physicist.
//
// Whenever something in the randomness changes, the test will fail and needs to be manually reevaluated.
// In that case, also the corresponding .csv file needs to be updated.
// Note: This .csv file can simply be generated by calling RAY-X with `-i input/<test>_seeded.rml -c`
//
// Additionally to randomness tests, there can also be (non-randomized) regression tests in this module.
// Which compares the current RAY-X the output of a previous RAY-X output which we deemed correct.

// TODO re-generate *.correct.* files for those tests!

TEST_F(TestSuite, PointSource_seeded) {
    RAYX::fixSeed(RAYX::FIXED_SEED);
    compareAgainstCorrect("PointSource_seeded");
}

// Tests sourceDepth of MatrixSource.
TEST_F(TestSuite, MatrixSource_seeded) {
    RAYX::fixSeed(RAYX::FIXED_SEED);
    compareAgainstCorrect("MatrixSource_seeded");
}

// Tests reflectivity of materials of a PlaneMirror.
TEST_F(TestSuite, PlaneMirror_refl_seeded) {
    RAYX::fixSeed(RAYX::FIXED_SEED);
    compareAgainstCorrect("PlaneMirror_refl_seeded");
}

// Tests the Energy Distribution of a MatrixSource.
TEST_F(TestSuite, MatrixSource_distr_seeded) {
    RAYX::fixSeed(RAYX::FIXED_SEED);
    compareAgainstCorrect("MatrixSource_distr_seeded");
}
