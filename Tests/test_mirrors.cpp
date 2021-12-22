#include "setupTests.h"

TEST(PlaneMirror, testSimpleParams) {
    // arrange
    // act
    // assert

    // user parameters for test session
    double width = 68.12;
    double height = 123.6;
    double incidenceAngle = 13.2;
    double azimuthalAngle = 0.0;
    double dist = 12005;
    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    int icurv = 1;
    std::vector<double> mis = {0, 0, 0, 0, 0, 0};
    std::vector<double> sE = {0, 0, 0, 0, 0, 0, 0};
    std::vector<double> surface = {
        0, 0, 0, 0, double(icurv), 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0};

    // world user parems to calculate position and orientation of object
    RAYX::WorldUserParams g_params = RAYX::WorldUserParams(
        degToRad(incidenceAngle), degToRad(incidenceAngle),
        degToRad(azimuthalAngle), dist, mis);
    glm::dvec4 position = g_params.calcPosition();
    glm::dmat4x4 orientation = g_params.calcOrientation();

    // create mirror with specified parameters
    RAYX::PlaneMirror plM =
        RAYX::PlaneMirror("planemirror", geometricalShape, width, height,
                          g_params.getAzimuthalAngle(), position, orientation,
                          sE);  // {1,2,3,0.01,0.02,0.03}

    glm::dmat4x4 correctInMatrix =
        glm::dmat4x4(1, 0, 0, 0, 0, 0.97357890287316029, 0.22835087011065572, 0,
                     0, -0.22835087011065572, 0.97357890287316029, 0, 0,
                     2741.352195678422, -11687.814728992289, 1);
    glm::dmat4x4 correctOutMatrix = glm::dmat4x4(
        1, 0, 0, 0, 0, 0.97357890287316029, -0.22835087011065572, 0, 0,
        0.22835087011065572, 0.97357890287316029, 0, 0, 0, 12005, 1);

    ASSERT_DOUBLE_EQ(plM.getWidth(), width);
    ASSERT_DOUBLE_EQ(plM.getHeight(), height);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, plM.getSlopeError());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, surface,
                              plM.getSurfaceParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, plM.getInMatrix(),
                              glmToVector16(correctInMatrix));
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, plM.getOutMatrix(),
                              glmToVector16(correctOutMatrix));
}

TEST(PlaneMirror, testAdvancedParams) {
    double width = 124.12;
    double height = 26;
    double incidenceAngle = 23;
    double azimuthalAngle = 8.2;
    double dist = 12005;
    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    int icurv = 1;
    std::vector<double> mis = {1, 2, 3, 0.01, 0.02, 0.03};
    std::vector<double> sE = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
    std::vector<double> surface = {
        0, 0, 0, 0, double(icurv), 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0};

    RAYX::WorldUserParams g_params = RAYX::WorldUserParams(
        degToRad(incidenceAngle), degToRad(incidenceAngle),
        degToRad(azimuthalAngle), dist, mis);
    glm::dvec4 position = g_params.calcPosition();
    glm::dmat4x4 orientation = g_params.calcOrientation();

    RAYX::PlaneMirror plM =
        RAYX::PlaneMirror("planemirror", geometricalShape, width, height,
                          g_params.getAzimuthalAngle(), position, orientation,
                          sE);  // {1,2,3,0.01,0.02,0.03}

    glm::dmat4x4 correctInMatrix = glm::dmat4x4(
        0.98631018201912979, -0.16127244932632739, -0.034400900187032908, 0,
        0.16212528089630251, 0.91026081860532748, 0.38097327387397439, 0,
        -0.030126701440516851, -0.38133507470497185, 0.92394585483136815, 0,
        360.67105079340479, 4575.9275718331864, -11094.969987250573, 1);
    glm::dmat4x4 correctOutMatrix = glm::dmat4x4(
        0.98631018201912979, 0.16212528089630251, -0.030126701440516851, 0,
        -0.16127244932632739, 0.91026081860532748, -0.38133507470497185, 0,
        -0.034400900187032908, 0.38097327387397439, 0.92394585483136815, 0,
        0.56056258280537641, 3.1255667397288804, 12006.979040713644, 1);

    ASSERT_DOUBLE_EQ(plM.getWidth(), width);
    ASSERT_DOUBLE_EQ(plM.getHeight(), height);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, plM.getSlopeError());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, surface,
                              plM.getSurfaceParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, plM.getInMatrix(),
                              glmToVector16(correctInMatrix));
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, plM.getOutMatrix(),
                              glmToVector16(correctOutMatrix));
}

TEST(SphereMirror, testParams) {
    double width = 21.62;
    double height = 813.12;
    double incidence = 12.75;
    double azimuthal = 41.2;
    double dist = 12.12;
    double entranceArmLength = 12.7;
    double exitArmLength = 123.1;
    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    int icurv = 1;
    double radius = 104.32651829593351;  // from old RAY
    std::vector<double> mis = {10, 51, 2, 0.1, 5, 0.241};
    std::vector<double> sE = {0.7, 0.5, 0.3, 0.7, 0.3, 3, 2};
    std::vector<double> surface = {
        1, 0, 0, 0, double(icurv), 1, 0, -radius, 0, 0, 1, 0, 0, 0, 0, 0};

    RAYX::GeometricUserParams g_params = RAYX::GeometricUserParams(incidence);
    RAYX::WorldUserParams w_params =
        RAYX::WorldUserParams(g_params.getAlpha(), g_params.getBeta(),
                              degToRad(azimuthal), dist, mis);
    glm::dvec4 position = w_params.calcPosition();
    glm::dmat4x4 orientation = w_params.calcOrientation();
    RAYX::SphereMirror sM =
        RAYX::SphereMirror("spheremirror", geometricalShape, width, height,
                           w_params.getAzimuthalAngle(), incidence, position,
                           orientation, entranceArmLength, exitArmLength, sE);

    glm::dmat4x4 correctInMatrix = glm::dmat4x4(
        0.024368111991334068, -0.85883516451860731, -0.51167211698926318, 0,
        0.39036506969235873, 0.47936537520215317, -0.78601777932905481, 0,
        0.92033760516565755, -0.18058515233428035, 0.34694047799924849, 0,
        -21.15449177460777, -48.811307953708521, -6.2049185933508921, 1);
    glm::dmat4x4 correctOutMatrix = glm::dmat4x4(
        0.024368111991334068, 0.39036506969235873, 0.92033760516565755, 0,
        -0.85883516451860731, 0.47936537520215317, -0.18058515233428035, 0,
        -0.51167211698926318, -0.78601777932905481, 0.34694047799924849, 0,
        -44.580256504514161, 26.779249273575296, 12.807414238606773, 1);

    ASSERT_DOUBLE_EQ(sM.getWidth(), width);
    ASSERT_DOUBLE_EQ(sM.getHeight(), height);
    EXPECT_NEAR(sM.getRadius(), radius, 0.0000000001);
    ASSERT_DOUBLE_EQ(sM.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(sM.getEntranceArmLength(), entranceArmLength);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, sM.getSlopeError());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, surface,
                              sM.getSurfaceParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sM.getInMatrix(),
                              glmToVector16(correctInMatrix));
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sM.getOutMatrix(),
                              glmToVector16(correctOutMatrix));
}

TEST(SphereMirror, testPrecalculateRadius) {
    double width = 21.62;
    double height = 813.12;
    double incidence = 12.75;
    double azimuthal = 41.2;
    double dist = 12.12;
    double entranceArmLength = 12.7;
    double exitArmLength = 123.1;
    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    int icurv = 1;
    double radius = 104.32651829593351;  // from old RAY
    std::vector<double> mis = {10, 51, 2, 0.1, 5, 0.241};
    std::vector<double> sE = {0.7, 0.5, 0.3, 0.7, 0.3, 3, 2};
    std::vector<double> surface = {
        1, 0, 0, 0, double(icurv), 1, 0, -radius, 0, 0, 1, 0, 0, 0, 0, 0};

    RAYX::GeometricUserParams g_params = RAYX::GeometricUserParams(incidence);
    g_params.calcMirrorRadius(entranceArmLength, exitArmLength);
    RAYX::WorldUserParams w_params =
        RAYX::WorldUserParams(g_params.getAlpha(), g_params.getBeta(),
                              degToRad(azimuthal), dist, mis);
    glm::dvec4 position = w_params.calcPosition();
    glm::dmat4x4 orientation = w_params.calcOrientation();
    RAYX::SphereMirror sM =
        RAYX::SphereMirror("spheremirror", geometricalShape, width, height,
                           w_params.getAzimuthalAngle(), incidence, position,
                           orientation, entranceArmLength, exitArmLength, sE);

    glm::dmat4x4 correctInMatrix = glm::dmat4x4(
        0.024368111991334068, -0.85883516451860731, -0.51167211698926318, 0,
        0.39036506969235873, 0.47936537520215317, -0.78601777932905481, 0,
        0.92033760516565755, -0.18058515233428035, 0.34694047799924849, 0,
        -21.15449177460777, -48.811307953708521, -6.2049185933508921, 1);
    glm::dmat4x4 correctOutMatrix = glm::dmat4x4(
        0.024368111991334068, 0.39036506969235873, 0.92033760516565755, 0,
        -0.85883516451860731, 0.47936537520215317, -0.18058515233428035, 0,
        -0.51167211698926318, -0.78601777932905481, 0.34694047799924849, 0,
        -44.580256504514161, 26.779249273575296, 12.807414238606773, 1);

    ASSERT_DOUBLE_EQ(sM.getWidth(), width);
    ASSERT_DOUBLE_EQ(sM.getHeight(), height);
    EXPECT_NEAR(g_params.getRadius(), radius, 0.0000000001);
    ASSERT_DOUBLE_EQ(sM.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(sM.getEntranceArmLength(), entranceArmLength);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, sM.getSlopeError());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, surface,
                              sM.getSurfaceParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sM.getInMatrix(),
                              glmToVector16(correctInMatrix));
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sM.getOutMatrix(),
                              glmToVector16(correctOutMatrix));
}

TEST(Ellips, defaultParams) {
    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    double width = 50;
    double height = 200;
    double incidence = 10;
    double entranceArmLength = 10000;
    double exitArmLength = 1000;

    double a11 = 1;
    int figRot = 0;  // "yes"

    double dist = 10000;
    double chi = 0;
    std::vector<double> mis = {1, 2, 3, 0.004, 0.005, 0.006};
    std::vector<double> sE = {0, 0, 0, 0, 0, 0, 0};

    double alpha = 0.031253965260898464;
    double beta = 0.31781188513796743;
    double correctTangentAngle = 0.14327895993853446;  // alpha1 in old ray code
    RAYX::GeometricUserParams e_params =
        RAYX::GeometricUserParams(incidence, entranceArmLength, exitArmLength);
    ASSERT_DOUBLE_EQ(e_params.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(e_params.getBeta(), beta);
    double tangentAngle = e_params.calcTangentAngle(
        incidence, entranceArmLength, exitArmLength, 1);
    ASSERT_DOUBLE_EQ(correctTangentAngle, tangentAngle);
    tangentAngle = e_params.calcTangentAngle(incidence, entranceArmLength,
                                             exitArmLength, 0);
    ASSERT_DOUBLE_EQ(0, tangentAngle);

    double shortHalfAxisC = 549.1237529650836;
    double y0 = 312.4887730997391;
    double z0 = 4522.597446463379;
    double shortHalfAxisB = 549.1237529650836;
    double longHalfAxisA = 5500;

    double a33 = 0.0099681618535688628;
    double a34 = 45.0819833448842;
    double a44 = -5.8207660913467407e-11;
    std::vector<double> surface = {1, 0, 0,   0,   1, 1, 0, -y0,
                                   0, 0, a33, a34, 7, 0, 0, a44};
    std::vector<double> elementParams = {sin(correctTangentAngle),
                                         cos(correctTangentAngle),
                                         y0,
                                         z0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0};

    RAYX::WorldUserParams w_params =
        RAYX::WorldUserParams(e_params.getAlpha(), e_params.getBeta(),
                              degToRad(chi), dist, mis, tangentAngle);
    RAYX::Ellipsoid eb =
        RAYX::Ellipsoid("ellipsoid", geometricalShape, width, height,
                        w_params.getAzimuthalAngle(), w_params.calcPosition(),
                        w_params.calcOrientation(), incidence,
                        entranceArmLength, exitArmLength, figRot, a11, sE);

    EXPECT_NEAR(eb.getShortHalfAxisB(), shortHalfAxisB, 0.0000001);
    EXPECT_NEAR(eb.getLongHalfAxisA(), longHalfAxisA, 0.0000001);
    EXPECT_NEAR(eb.getHalfAxisC(), shortHalfAxisC, 0.0000001);
    EXPECT_NEAR(eb.getY0(), y0, 0.0000001);
    EXPECT_NEAR(eb.getZ0(), z0, 0.0000001);
    EXPECT_NEAR(eb.getTangentAngle(), correctTangentAngle, 0.0000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, surface,
                              eb.getSurfaceParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, elementParams,
                              eb.getElementParameters());
}

/*
TEST(Ellips, testParamsCSCurvature) {
    double width = 123.51;
    double height = 85.234;
    double incidence = 31.513;
    double azimuthal = 51.2452;
    double dist = 12451.51535;
    double entranceArmLength = 6421.7234;
    double exitArmLength = 1762.622123;
    int geometricalShape = 0;
    int coordSys = 0;  // {CS_CURVATURE, CS_MIRROR};
    int figRot = 0;    // { FR_YES, FR_PLANE, FR_A11};
    double a_11 = 12.62;
    std::vector<double> mis = {12, 72, 1.12, 0.1, 0.7341, 2.5};
    std::vector<double> sE = {1, 2, 3, 4, 5, 6, 7};

    RAYX::GeometricUserParams e_params =
        RAYX::GeometricUserParams(incidence, entranceArmLength, exitArmLength);
    // derived from above parameters
    double beta = 0.55000560718097313;
    double alpha = 0.2141981204294254;
    ASSERT_DOUBLE_EQ(e_params.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(e_params.getBeta(), beta);
    RAYX::WorldUserParams w_params =
        RAYX::WorldUserParams(e_params.getAlpha(), e_params.getBeta(),
                              degToRad(azimuthal), dist, mis);

    RAYX::Ellipsoid eb = RAYX::Ellipsoid(
        "ellipsoid", geometricalShape, width, height,
        w_params.getAzimuthalAngle(), w_params.calcPosition(),
        w_params.calcOrientation(), incidence, entranceArmLength, exitArmLength,
        coordSys, figRot, a_11, mis, sE);
    RAYX::Ellipsoid e =
        RAYX::Ellipsoid("ellipsoid", geometricalShape, width, height, incidence,
                        azimuthal, dist, entranceArmLength, exitArmLength,
                        coordSys, figRot, a_11, mis, sE, NULL, false);

    // in old RAY
    double shortHalfAxis = 1758.53601375;
    double longHalfAxis = 4092.1727615;
    double surfaceCenterY0 = 1365.026842783936;  // actually neg. depending on
    which calc in old RAY is used double surfaceCenterZ0 = 2579.914108547892;
    double tangentAngle = 0.3358074867515476;
    double halfAxisC = 1758.536013753882;
    double radius = -1365.026842783936;
    double a33 = 0.1846692617906655;
    double a34 = 476.4308339088623;
    double a44 = -4.65661e-10;
    double alphaE = 0.2141981204294254;
    double betaE = 0.8858130939325208;
    std::vector<double> correctTempMis = {0, 0, 0, 0, 0, 0};

    ASSERT_DOUBLE_EQ(e.getWidth(), width);
    ASSERT_DOUBLE_EQ(e.getHeight(), height);
    EXPECT_NEAR(e.getRadius(), radius, 0.00000001);
    ASSERT_DOUBLE_EQ(e.getIncidenceAngle(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e.getBeta(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e.getChi(), degToRad(azimuthal));
    ASSERT_DOUBLE_EQ(e.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(e.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(e.getEntranceArmLength(), entranceArmLength);
    EXPECT_NEAR(e.getTangentAngle(), tangentAngle, 0.00000001);
    EXPECT_NEAR(e.getShortHalfAxisB(), shortHalfAxis, 0.0000001);
    EXPECT_NEAR(e.getLongHalfAxisA(), longHalfAxis, 0.0000001);
    EXPECT_NEAR(e.getMy0(), surfaceCenterY0, 0.0000001);
    EXPECT_NEAR(e.getMz0(), surfaceCenterZ0, 0.0000001);
    EXPECT_NEAR(e.getHalfAxisC(), halfAxisC, 0.0000001);
    EXPECT_NEAR(e.getAlpha(), alphaE, 0.000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, correctTempMis,
                              e.getTempMisalignmentParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, e.getSlopeError());

    glm::dmat4x4 correctInMat = {0.6259888054371695,
                                 -0.76201066544902907,
                                 -0.16576417347929398,
                                 0,
                                 0.77983204311399357,
                                 0.61168318281721634,
                                 0.13306264837006021,
                                 0,
                                 0,
                                 -0.21256394238094042,
                                 0.9771471590295252,
                                 0,
                                 0,
                                 2646.7431914127951,
                                 -12166.962849865024,
                                 1};
    glm::dmat4x4 correctOutMat = {0.6259888054371695,
                                  0.77983204311399357,
                                  0,
                                  0,
                                  -0.6648236542987721,
                                  0.53366897251236345,
                                  0.52269200918172165,
                                  0,
                                  0.40761197743954031,
                                  -0.32719934643922,
                                  0.85252159124421889,
                                  0,
                                  0,
                                  0,
                                  0,
                                  1};
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, e.getInMatrix(),
                              glmToVector16(correctInMat));
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, e.getOutMatrix(),
                              glmToVector16(correctOutMat));

    figRot = 1;
    RAYX::Ellipsoid e2 =
        RAYX::Ellipsoid("ellipsoid2", geometricalShape, width, height,
                        w_params.getAzimuthalAngle(), incidence, azimuthal,
                        dist, entranceArmLength, exitArmLength, coordSys,
                        figRot, a_11, mis, sE, NULL, false);
    halfAxisC = INFINITY;

    ASSERT_DOUBLE_EQ(e2.getWidth(), width);
    ASSERT_DOUBLE_EQ(e2.getHeight(), height);
    EXPECT_NEAR(e2.getRadius(), radius, 0.00000001);
    ASSERT_DOUBLE_EQ(e2.getIncidenceAngle(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e2.getBeta(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e2.getChi(), degToRad(azimuthal));
    ASSERT_DOUBLE_EQ(e2.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(e2.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(e2.getEntranceArmLength(), entranceArmLength);
    EXPECT_NEAR(e2.getTangentAngle(), tangentAngle, 0.00000001);
    EXPECT_NEAR(e2.getShortHalfAxisB(), shortHalfAxis, 0.0000001);
    EXPECT_NEAR(e2.getLongHalfAxisA(), longHalfAxis, 0.0000001);
    EXPECT_NEAR(e2.getMy0(), surfaceCenterY0, 0.0000001);
    EXPECT_NEAR(e2.getMz0(), surfaceCenterZ0, 0.0000001);
    ASSERT_DOUBLE_EQ(e2.getHalfAxisC(), halfAxisC);
    EXPECT_NEAR(e2.getAlpha(), alphaE, 0.000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, correctTempMis,
                              e2.getTempMisalignmentParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, e2.getSlopeError());

    figRot = 2;
    RAYX::Ellipsoid e3 =
        RAYX::Ellipsoid("ellipsoid3", geometricalShape, width, height,
                        w_params.getAzimuthalAngle(), incidence, azimuthal,
                        dist, entranceArmLength, exitArmLength, coordSys,
                        figRot, a_11, mis, sE, NULL, false);
    halfAxisC = 495.0186818473859;

    ASSERT_DOUBLE_EQ(e3.getWidth(), width);
    ASSERT_DOUBLE_EQ(e3.getHeight(), height);
    EXPECT_NEAR(e3.getRadius(), radius, 0.00000001);
    ASSERT_DOUBLE_EQ(e3.getIncidenceAngle(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e3.getBeta(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e3.getChi(), degToRad(azimuthal));
    ASSERT_DOUBLE_EQ(e3.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(e3.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(e3.getEntranceArmLength(), entranceArmLength);
    EXPECT_NEAR(e3.getTangentAngle(), tangentAngle, 0.00000001);
    EXPECT_NEAR(e3.getShortHalfAxisB(), shortHalfAxis, 0.0000001);
    EXPECT_NEAR(e3.getLongHalfAxisA(), longHalfAxis, 0.0000001);
    EXPECT_NEAR(e3.getMy0(), surfaceCenterY0, 0.0000001);
    EXPECT_NEAR(e3.getMz0(), surfaceCenterZ0, 0.0000001);
    EXPECT_NEAR(e3.getHalfAxisC(), halfAxisC, 0.00000001);
    EXPECT_NEAR(e3.getAlpha(), alphaE, 0.000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, correctTempMis,
                              e3.getTempMisalignmentParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, e3.getSlopeError());
}

TEST(Ellips, testParamsCSMirror) {
    double width = 123.51;
    double height = 85.234;
    double incidence = 31.513;
    double azimuthal = 51.2452;
    double dist = 12451.51535;
    double entranceArmLength = 6421.7234;
    double exitArmLength = 1762.622123;
    int coordSys = 1;  // {CS_CURVATURE, CS_MIRROR};
    int figRot = 0;    // { FR_YES, FR_PLANE, FR_A11};
    int geometricalShape = 0;
    double a_11 = 12.62;
    std::vector<double> mis = {12, 72, 1.12, 0.1, 0.7341, 2.5};
    std::vector<double> sE = {7, 6, 5, 4, 3, 2, 1};
    RAYX::Ellipsoid e =
        RAYX::Ellipsoid("ellipsoid", geometricalShape, width, height, incidence,
                        azimuthal, dist, entranceArmLength, exitArmLength,
                        coordSys, figRot, a_11, mis, sE, NULL, false);

    // in old RAY
    double shortHalfAxis = 1758.53601375;
    double longHalfAxis = 4092.1727615;
    double surfaceCenterY0 = 1365.026842783936;  // actually neg. depending on
    which calc in old RAY is used double surfaceCenterZ0 = 2579.914108547892;
    double tangentAngle = 0.3358074867515476;
    double halfAxisC = 1758.536013753882;
    double radius = -1365.026842783936;
    double alpha = 0.2141981204294254;
    double a33 = 0.1846692617906655;
    double a34 = 476.4308339088623;
    double a44 = -4.65661e-10;
    double alphaE = 0.2141981204294254;
    double betaE = 0.8858130939325208;
    std::vector<double> correctTempMis = {0, 0, 0, 0, 0, tangentAngle};

    ASSERT_DOUBLE_EQ(e.getWidth(), width);
    ASSERT_DOUBLE_EQ(e.getHeight(), height);
    EXPECT_NEAR(e.getRadius(), radius, 0.00000001);
    ASSERT_DOUBLE_EQ(e.getIncidenceAngle(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e.getBeta(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e.getChi(), degToRad(azimuthal));
    ASSERT_DOUBLE_EQ(e.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(e.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(e.getEntranceArmLength(), entranceArmLength);
    EXPECT_NEAR(e.getTangentAngle(), tangentAngle, 0.00000001);
    EXPECT_NEAR(e.getShortHalfAxisB(), shortHalfAxis, 0.0000001);
    EXPECT_NEAR(e.getLongHalfAxisA(), longHalfAxis, 0.0000001);
    EXPECT_NEAR(e.getMy0(), surfaceCenterY0, 0.0000001);
    EXPECT_NEAR(e.getMz0(), surfaceCenterZ0, 0.0000001);
    EXPECT_NEAR(e.getHalfAxisC(), halfAxisC, 0.0000001);
    EXPECT_NEAR(e.getAlpha(), alphaE, 0.000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, correctTempMis,
                              e.getTempMisalignmentParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, e.getSlopeError());

    figRot = 1;
    RAYX::Ellipsoid e2 =
        RAYX::Ellipsoid("ellipsoid", geometricalShape, width, height, incidence,
                        azimuthal, dist, entranceArmLength, exitArmLength,
                        coordSys, figRot, a_11, mis, sE, NULL, false);
    halfAxisC = INFINITY;

    ASSERT_DOUBLE_EQ(e2.getWidth(), width);
    ASSERT_DOUBLE_EQ(e2.getHeight(), height);
    EXPECT_NEAR(e.getRadius(), radius, 0.00000001);
    ASSERT_DOUBLE_EQ(e2.getIncidenceAngle(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e2.getBeta(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e2.getChi(), degToRad(azimuthal));
    ASSERT_DOUBLE_EQ(e2.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(e2.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(e2.getEntranceArmLength(), entranceArmLength);
    EXPECT_NEAR(e2.getTangentAngle(), tangentAngle, 0.00000001);
    EXPECT_NEAR(e2.getShortHalfAxisB(), shortHalfAxis, 0.0000001);
    EXPECT_NEAR(e2.getLongHalfAxisA(), longHalfAxis, 0.0000001);
    EXPECT_NEAR(e2.getMy0(), surfaceCenterY0, 0.0000001);
    EXPECT_NEAR(e2.getMz0(), surfaceCenterZ0, 0.0000001);
    ASSERT_DOUBLE_EQ(e2.getHalfAxisC(), halfAxisC);
    EXPECT_NEAR(e2.getAlpha(), alphaE, 0.000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, correctTempMis,
                              e2.getTempMisalignmentParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, e2.getSlopeError());

    figRot = 2;
    RAYX::Ellipsoid e3 =
        RAYX::Ellipsoid("ellipsoid", geometricalShape, width, height, incidence,
                        azimuthal, dist, entranceArmLength, exitArmLength,
                        coordSys, figRot, a_11, mis, sE, NULL, false);
    halfAxisC = 495.0186818473859;

    ASSERT_DOUBLE_EQ(e3.getWidth(), width);
    ASSERT_DOUBLE_EQ(e3.getHeight(), height);
    EXPECT_NEAR(e3.getRadius(), radius, 0.00000001);
    ASSERT_DOUBLE_EQ(e3.getIncidenceAngle(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e3.getBeta(), degToRad(incidence));
    ASSERT_DOUBLE_EQ(e3.getChi(), degToRad(azimuthal));
    ASSERT_DOUBLE_EQ(e3.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(e3.getExitArmLength(), exitArmLength);
    ASSERT_DOUBLE_EQ(e3.getEntranceArmLength(), entranceArmLength);
    EXPECT_NEAR(e3.getTangentAngle(), tangentAngle, 0.00000001);
    EXPECT_NEAR(e3.getShortHalfAxisB(), shortHalfAxis, 0.0000001);
    EXPECT_NEAR(e3.getLongHalfAxisA(), longHalfAxis, 0.0000001);
    EXPECT_NEAR(e3.getMy0(), surfaceCenterY0, 0.0000001);
    EXPECT_NEAR(e3.getMz0(), surfaceCenterZ0, 0.0000001);
    EXPECT_NEAR(e3.getHalfAxisC(), halfAxisC, 0.00000001);
    EXPECT_NEAR(e3.getAlpha(), alphaE, 0.000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, correctTempMis,
                              e3.getTempMisalignmentParams());
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, sE, e3.getSlopeError());
}*/