#include "setupTests.h"

TEST(Quadric, testTransformationMatrices) {
    double width = 68.12;
    double height = 123.6;
    double incidenceAngle = 13.2;
    double azimuthalAngle = 0.0;
    double dist = 12005;
    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    std::vector<double> sE = {1, 2, 3, 4, 5, 6, 7};
    std::vector<double> mis = {1, 2, 3, 0.03, 0.02, 0.01};  // psi, phi, chi

    RAYX::GeometricUserParams g_params =
        RAYX::GeometricUserParams(incidenceAngle);
    RAYX::WorldUserParams w_coord = RAYX::WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), azimuthalAngle, dist, mis);

    RAYX::PlaneMirror plM = RAYX::PlaneMirror(
        "planemirror", geometricalShape, width, height,
        w_coord.getAzimuthalAngle(), w_coord.calcPosition(),
        w_coord.calcOrientation(), sE);  // {1,2,3,0.01,0.02,0.03}
    // RAYX::PlaneMirror plM1 = RAYX::PlaneMirror("planemirror", width, height,
    // incidenceAngle, azimuthalAngle, dist, mis, sE, NULL, true); //
    // {1,2,3,0.01,0.02,0.03}

    std::vector<double> correctInMat = {
        0.9997500170828264,    -0.0093954937290516224, 0.02028861849598634, 0,
        0.0051669667654668724, 0.97994613741550907,    0.199195557728752,   0,
        -0.0217532939421341,   -0.19904093162465691,   0.97974971382524823, 0,
        260.14829377531987,    2387.4863841540064,     -11764.895314472104, 1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, plM.getInMatrix(),
                              correctInMat);

    std::vector<double> correctOutMat = {
        0.9997500170828264,     0.0051669667654668724, -0.0217532939421341,  0,
        -0.0093954937290516224, 0.97994613741550907,   -0.19904093162465691, 0,
        0.02028861849598634,    0.199195557728752,     0.97974971382524823,  0,
        1.0418248851126821,     2.562645914782741,     12007.519413984284,   1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, plM.getOutMatrix(),
                              correctOutMat);
}

TEST(Quadric, testGlobalCoordinates) {
    std::vector<double> sE = {0, 0, 0, 0, 0, 0, 0};
    RAYX::GeometricUserParams g_params = RAYX::GeometricUserParams(10);
    RAYX::WorldUserParams w_coord = RAYX::WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), degToRad(7), 2000,
        std::vector<double>{0.5, 0.7, 1.0, 0.04, 0.03, 0.05});
    glm::dvec4 pos = w_coord.calcPosition();
    glm::dmat4x4 or1 = w_coord.calcOrientation();
    std::shared_ptr<RAYX::PlaneMirror> p1 = std::make_shared<RAYX::PlaneMirror>(
        "PlaneMirror1", RAYX::Geometry::GeometricalShape::RECTANGLE, 50, 200,
        w_coord.getAzimuthalAngle(), pos, or1,
        sE);  // {1,2,3,0.01,0.02,0.03}

    RAYX::GeometricUserParams g_params2 = RAYX::GeometricUserParams(15);
    RAYX::WorldUserParams w_coord2 = RAYX::WorldUserParams(
        g_params2.getAlpha(), g_params2.getBeta(), degToRad(4), 7000,
        std::vector<double>{2, 4, 6, 0.04, 0.01,
                            0.06});  // std::vector<double>{0,0,0, 0,0,0});
    glm::dvec4 pos2 = w_coord2.calcPosition(w_coord, pos, or1);
    glm::dmat4x4 or2 = w_coord2.calcOrientation(w_coord, or1);
    std::shared_ptr<RAYX::PlaneMirror> p2 = std::make_shared<RAYX::PlaneMirror>(
        "PlaneMirror2", RAYX::Geometry::GeometricalShape::RECTANGLE, 50, 200,
        w_coord2.getAzimuthalAngle(), pos2, or2,
        sE);  // {1,2,3,0.01,0.02,0.03}

    std::cout << "MIRROR 3" << std::endl;
    g_params = RAYX::GeometricUserParams(7);
    RAYX::WorldUserParams w_coord3 = RAYX::WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), degToRad(10), 8000,
        std::vector<double>{4, 5, 3, 0.01, 0.02, 0.03});
    glm::dvec4 pos3 = w_coord3.calcPosition(w_coord2, pos2, or2);
    glm::dmat4x4 or3 = w_coord3.calcOrientation(w_coord2, or2);
    std::shared_ptr<RAYX::PlaneMirror> p3 = std::make_shared<RAYX::PlaneMirror>(
        "PlaneMirror1", RAYX::Geometry::GeometricalShape::RECTANGLE, 50, 200,
        w_coord3.getAzimuthalAngle(), pos3, or3,
        sE);  // {1,2,3,0.01,0.02,0.03}

    std::cout << "MIRROR 4" << std::endl;
    g_params = RAYX::GeometricUserParams(22);
    RAYX::WorldUserParams w_coord4 = RAYX::WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), degToRad(17), 1000,
        std::vector<double>{10, 3, 2, 0.01, 0.03, 0.06});
    glm::dvec4 pos4 = w_coord4.calcPosition(w_coord3, pos3, or3);
    glm::dmat4x4 or4 = w_coord4.calcOrientation(w_coord3, or3);
    std::shared_ptr<RAYX::PlaneMirror> p4 = std::make_shared<RAYX::PlaneMirror>(
        "PlaneMirror1", RAYX::Geometry::GeometricalShape::RECTANGLE, 50, 200,
        w_coord4.getAzimuthalAngle(), pos4, or4,
        sE);  // {1,2,3,0.01,0.02,0.03}

    std::vector<double> correctInMat = {
        0.98549875516199115,   -0.16900296657661762, 0.015172371682388559, 0,
        0.16532344425841758,   0.97647242956597469,  0.13845487740074897,  0,
        -0.038214687656708428, -0.13393876058044285, 0.99025251631160971,  0,
        75.929375313416855,    267.1775211608857,    -1981.5050326232194,  1};
    std::vector<double> correctOutMat = {
        0.98549875516199115,  0.16532344425841758, -0.038214687656708428, 0,
        -0.16900296657661762, 0.97647242956597469, -0.13393876058044285,  0,
        0.015172371682388559, 0.13845487740074897, 0.99025251631160971,   0,
        0.38961967265975178,  0.90464730022614015, 2000.877388040077,     1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p1->getInMatrix(),
                              correctInMat);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p1->getOutMatrix(),
                              correctOutMat);

    correctInMat = {
        0.99293120507151145,   -0.11097960343033685, -0.042085028426756203, 0,
        0.11609472530507925,   0.8343464348616344,   0.53887664765697407,   0,
        -0.024690823225925775, -0.5399532889575962,  0.84133275758899373,   0,
        223.62868377282729,    2612.60301401578,     -8515.6341722842517,   1};
    correctOutMat = {
        0.99293120507151167,  0.11609472530507926, -0.024690823225925786, 0,
        -0.11097960343033686, 0.8343464348616344,  -0.5399532889575962,   0,
        -0.04208502842675621, 0.53887664765697396, 0.84133275758899373,   0,
        -290.48295826317462,  2383.0982743679397,  8580.687147244611,     1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p2->getInMatrix(),
                              correctInMat);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p2->getOutMatrix(),
                              correctOutMat);

    correctInMat = {
        0.98543342847582116, -0.15825553171931203, -0.06225869194438529, 0,
        0.13887040159030783, 0.53751266264821851,  0.83174223714740403,  0,
        -0.0981629747131509, -0.82827249391311997, 0.55165995524635025,  0,
        983.58748092948872,  6671.4190423504515,   -14677.230524351115,  1};
    correctOutMat = {
        0.98543342847582127,   0.13887040159030789, -0.098162974713150941, 0,
        -0.15825553171931206,  0.53751266264821851, -0.82827249391312019,  0,
        -0.062258691944385297, 0.83174223714740392, 0.55165995524635025,   0,
        -827.25618948103272,   8485.1091498802671,  13719.145095369648,    1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p3->getInMatrix(),
                              correctInMat);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p3->getOutMatrix(),
                              correctOutMat);

    correctInMat = {
        0.95892551908867896,  -0.24148295699748484, -0.14882147130121315, 0,
        0.16352170556644466,  0.041905034073013991, 0.98564933923174713,  0,
        -0.23178114817462611, -0.96949984507429876, 0.079671511544351148, 0,
        2630.3284873799857,   13109.341026432205,   -10506.281074894196,  1,
    };
    correctOutMat = {
        0.95892551908867907,  0.16352170556644474, -0.23178114817462619, 0,
        -0.24148295699748484, 0.04190503407301399, -0.96949984507429876, 0,
        -0.14882147130121315, 0.98564933923174713, 0.079671511544351137, 0,
        -920.16688225314476,  9376.0458164086922,  14156.215944980175,   1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p4->getInMatrix(),
                              correctInMat);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p4->getOutMatrix(),
                              correctOutMat);
}