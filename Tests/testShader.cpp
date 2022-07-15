#include "Tracer/CpuTracer.h"
#include "setupTests.h"

// TODO(rudi): shader tests

namespace RAYX {
namespace CPP_TRACER {
double r8_exp(double);
double r8_log(double);
Ray refrac2D(Ray, glm::dvec4, double, double);
Ray refrac(Ray, glm::dvec4, double);
glm::dvec4 normal_cartesian(glm::dvec4, double, double);
glm::dvec4 normal_cylindrical(glm::dvec4, double, double);
double wasteBox(double, double, double, double, double);
void RZPLineDensity(Ray r, glm::dvec4 normal, int IMAGE_TYPE, int RZP_TYPE,
                    int DERIVATION_METHOD, double zOffsetCenter, double risag,
                    double rosag, double rimer, double romer, double alpha,
                    double beta, double Ord, double WL, double& DX, double& DZ);
Ray rayMatrixMult(Ray, glm::dmat4);
void cosini(Ray&, double, double);
double dpow(double, int);
double fact(int);
double bessel1(double);
double r8_sin(double);
double r8_cos(double);
double r8_atan(double);
double vlsGrating(double, double, double[6]);

}  // namespace CPP_TRACER
}  // namespace RAYX

using namespace RAYX;

TEST_F(TestSuite, testUniformRandom) {
	Beamline b = loadBeamline("test"); // this beamline has just one ray -> one thread.
	b.m_testSettings = 1;

	tracer->trace(b);
}

TEST_F(TestSuite, SinTest) {
    std::vector<double> args = {
        -0.5620816275750421,  -0.082699735953560394, -0.73692442452247864,
        -0.93085577907030514, 0.038832744045494971,  0.86938579245347758,
        0.35772943348137098,  0.059400386282114415,  0.86087298993938566,
        0.3735454248180905,   -0.8663155254748649,   -0.98460362787680167,
        0.40238118899835329,  0.3078379243610454,    0.053857555519812195,
        0.5128209722651722,
    };

    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_sin(x), sin(x));
    }
}

TEST_F(TestSuite, CosTest) {
    std::vector<double> args = {
        -0.5620816275750421,  -0.082699735953560394, -0.73692442452247864,
        -0.93085577907030514, 0.038832744045494971,  0.86938579245347758,
        0.35772943348137098,  0.059400386282114415,  0.86087298993938566,
        0.3735454248180905,   -0.8663155254748649,   -0.98460362787680167,
        0.40238118899835329,  0.3078379243610454,    0.053857555519812195,
        0.5128209722651722,
    };

    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_cos(x), cos(x));
    }
}

TEST_F(TestSuite, AtanTest) {
    std::vector<double> args = {
        -0.5620816275750421,  -0.082699735953560394, -0.73692442452247864,
        -0.93085577907030514, 0.038832744045494971,  0.86938579245347758,
        0.35772943348137098,  0.059400386282114415,  0.86087298993938566,
        0.3735454248180905,   -0.8663155254748649,   -0.98460362787680167,
        0.40238118899835329,  0.3078379243610454,    0.053857555519812195,
        0.5128209722651722,
    };

    for (auto x : args) {
        CHECK_EQ(CPP_TRACER::r8_atan(x), atan(x));
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

    CHECK_EQ(input.size(), correct.size());
    for (uint i = 0; i < input.size(); i++) {
        auto r = input[i];

        glm::dvec4 normal = glm::dvec4(r.m_position, 0);
        double az = r.m_stokes.x;
        double ax = r.m_stokes.y;

        auto out = CPP_TRACER::refrac2D(r, normal, az, ax);

        CHECK_EQ(out, correct[i]);
    }
}

TEST_F(TestSuite, testNormalCartesian) {
    struct InOutPair {
        glm::dvec4 in_normal;
        double in_slopeX;
        double in_slopeZ;

        glm::dvec4 out;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(0, 1, 0, 0),
        },
        {
            .in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                    -28.532199794465537, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(5.0465463027123736, 10470.451695989539,
                              -28.532199794465537, 0),
        },
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 2,
            .in_slopeZ = 3,
            .out = glm::dvec4(-0.90019762973551742, 0.41198224566568298,
                              -0.14112000805986721, 0),
        },
        {
            .in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                    -28.532199794465537, 0),
            .in_slopeX = 2,
            .in_slopeZ = 3,
            .out = glm::dvec4(-9431.2371568647086, 4310.7269916467494,
                              -1449.3435640204684, 0),
        }};

    for (auto p : inouts) {
        auto out =
            CPP_TRACER::normal_cartesian(p.in_normal, p.in_slopeX, p.in_slopeZ);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testNormalCylindrical) {
    struct InOutPair {
        glm::dvec4 in_normal;
        double in_slopeX;
        double in_slopeZ;

        glm::dvec4 out;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(0, 1, 0, 0),
        },
        {
            .in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                    -28.532199794465537, 0),
            .in_slopeX = 0,
            .in_slopeZ = 0,
            .out = glm::dvec4(5.0465463027115769, 10470.451695989539,
                              -28.532199794465537, 0),
        },
        {
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_slopeX = 2,
            .in_slopeZ = 3,
            .out = glm::dvec4(0.90019762973551742, 0.41198224566568292,
                              -0.14112000805986721, 0),
        },
        {.in_normal = glm::dvec4(5.0465463027123736, 10470.451695989539,
                                 -28.532199794465537, 0),
         .in_slopeX = 2,
         .in_slopeZ = 3,
         .out = glm::dvec4(9431.2169472441783, 4310.7711493493844,
                           -1449.3437356459144, 0)}};

    for (auto p : inouts) {
        auto out = CPP_TRACER::normal_cylindrical(p.in_normal, p.in_slopeX,
                                                  p.in_slopeZ);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testRefrac) {
    std::vector<Ray> input = {
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.00049999991666667084, -0.99558611855684065,
                           0.093851108341926226),
            .m_energy = 0.01239852,
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction = glm::dvec3(-1.6666664506172892e-05,
                                      -0.995586229182718, 0.093851118714515264),
            .m_energy = 0.01239852,
        },
        {
            .m_position = glm::dvec3(0.0027574667592826954, 0.99999244446428082,
                                     -0.0027399619384214182),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.00049999991666667084, -0.99558611855684065,
                           0.093851108341926226),
            .m_energy = 0.01239852,
        },
        {
            .m_position = glm::dvec3(0, 1, 0),
            .m_weight = 1,
            .m_direction =
                glm::dvec3(-0.99991341437509562, 0.013149667401360443,
                           -0.00049999997222215965),
            .m_energy = -0.038483898782123105,
        },
    };

    // the correct rays should only be altered in weight & direction.
    std::vector<Ray> correct = input;

    correct[0].m_weight = 1;
    correct[0].m_direction = glm::dvec3(
        -0.00049999991666667084, 0.99667709206767885, 0.08145258834192623);

    correct[1].m_weight = 1;
    correct[1].m_direction = glm::dvec3(
        -1.6666664506160695e-05, 0.9966772027014974, 0.081452598714515267);

    correct[2].m_weight = 1;
    correct[2].m_direction = glm::dvec3(
        0.0049947959329671825, 0.99709586573547515, 0.07599267429701162);

    correct[3].m_weight = 0;
    correct[3].m_direction = glm::dvec3(
        -0.99991341437509562, 0.013149667401360443, -0.00049999997222215965);

    CHECK_EQ(input.size(), correct.size());
    for (uint i = 0; i < input.size(); i++) {
        auto r = input[i];

        glm::dvec4 normal = glm::dvec4(r.m_position, 0);
        double a = r.m_energy;

        auto out = CPP_TRACER::refrac(r, normal, a);

        CHECK_EQ(out, correct[i]);
    }
}

TEST_F(TestSuite, testWasteBox) {
    struct InOutPair {
        double in_x;
        double in_z;
        double in_xLength;
        double in_zLength;
        double in_w;

        double out;
    };

    std::vector<InOutPair> inouts = {{
                                         .in_x = -5.0466620698997637,
                                         .in_z = 28.760236725599515,
                                         .in_xLength = 50,
                                         .in_zLength = 200,
                                         .in_w = 1,
                                         .out = 1,
                                     },
                                     {
                                         .in_x = -5.0466620698997637,
                                         .in_z = 28.760236725599515,
                                         .in_xLength = 5,
                                         .in_zLength = 20,
                                         .in_w = 1,
                                         .out = 0,
                                     },
                                     {
                                         .in_x = -1.6822205656320104,
                                         .in_z = 28.760233508097873,
                                         .in_xLength = 5,
                                         .in_zLength = 20,
                                         .in_w = 1,
                                         .out = 0,
                                     },
                                     {
                                         .in_x = -5.0466620698997637,
                                         .in_z = 28.760236725599515,
                                         .in_xLength = 50,
                                         .in_zLength = 200,
                                         .in_w = 0,
                                         .out = 0,
                                     }};

    for (auto p : inouts) {
        auto out = CPP_TRACER::wasteBox(p.in_x, p.in_z, p.in_xLength,
                                        p.in_zLength, p.in_w);

        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testRZPLineDensityDefaulParams) {
    struct InOutPair {
        Ray in_ray;
        glm::dvec4 in_normal;
        int in_imageType;
        int in_rzpType;
        int in_derivationMethod;
        double in_zOffsetCenter;
        double in_risag;
        double in_rosag;
        double in_rimer;
        double in_romer;
        double in_alpha;
        double in_beta;
        double in_Ord;
        double in_WL;

        double out_DX;
        double out_DZ;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_ray =
                {
                    .m_position =
                        glm::dvec3(-5.0805095016939532, 0, 96.032788311782269),
                    .m_direction = glm::dvec3(0, 1, 0),
                },
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_imageType = 0,
            .in_rzpType = 0,
            .in_derivationMethod = 0,
            .in_zOffsetCenter = 0,
            .in_risag = 100,
            .in_rosag = 500,
            .in_rimer = 100,
            .in_romer = 500,
            .in_alpha = 0.017453292519943295,
            .in_beta = 0.017453292519943295,
            .in_Ord = -1,
            .in_WL = 1.239852e-05,

            .out_DX = 3103.9106911246749,
            .out_DZ = 5.0771666330055218,
        },
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6935030407867075, 0, 96.032777495754004),
                 .m_direction = glm::dvec3(0, 1, 0),
             },
         .in_normal = glm::dvec4(0, 1, 0, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1034.8685185321938,
         .out_DZ = -13.320120179862876},
        {.in_ray = {.m_position =
                        glm::dvec3(-5.047050067282087, 4.4859372100394515,
                                   29.182033770349552),
                    .m_direction =
                        glm::dvec3(0.05047050067282087, 0.95514062789960552,
                                   -0.29182033770349552)},
         .in_normal = glm::dvec4(0.05047050067282087, 0.95514062789960552,
                                 -0.29182033770349552, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 4045.0989844091882,
         .out_DZ = -174.2085626048659},
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6802365843267262, 1.3759250917712356,
                                16.445931214643075),
                 .m_direction =
                     glm::dvec3(0.016802365843267261, 0.98624074908228765,
                                -0.16445931214643075),
             },
         .in_normal = glm::dvec4(0.016802365843267261, 0.98624074908228765,
                                 -0.16445931214643075, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1418.1004208892475,
         .out_DZ = 253.09836635775162},
    };

    for (auto p : inouts) {
        double DX;
        double DZ;
        CPP_TRACER::RZPLineDensity(p.in_ray, p.in_normal, p.in_imageType,
                                   p.in_rzpType, p.in_derivationMethod,
                                   p.in_zOffsetCenter, p.in_risag, p.in_rosag,
                                   p.in_rimer, p.in_romer, p.in_alpha,
                                   p.in_beta, p.in_Ord, p.in_WL, DX, DZ);
        CHECK_EQ(DX, p.out_DX);
        CHECK_EQ(DZ, p.out_DZ);
    }
}

TEST_F(TestSuite, testRZPLineDensityAstigmatic) {
    struct InOutPair {
        Ray in_ray;
        glm::dvec4 in_normal;
        int in_imageType;
        int in_rzpType;
        int in_derivationMethod;
        double in_zOffsetCenter;
        double in_risag;
        double in_rosag;
        double in_rimer;
        double in_romer;
        double in_alpha;
        double in_beta;
        double in_Ord;
        double in_WL;

        double out_DX;
        double out_DZ;
    };

    std::vector<InOutPair> inouts = {

        {
            .in_ray =
                {
                    .m_position =
                        glm::dvec3(-5.0805095016939532, 0, 96.032788311782269),
                    .m_direction = glm::dvec3(0, 1, 0),
                },
            .in_normal = glm::dvec4(0, 1, 0, 0),
            .in_imageType = 0,
            .in_rzpType = 0,
            .in_derivationMethod = 0,
            .in_zOffsetCenter = 0,
            .in_risag = 100,
            .in_rosag = 500,
            .in_rimer = 100,
            .in_romer = 500,
            .in_alpha = 0.017453292519943295,
            .in_beta = 0.017453292519943295,
            .in_Ord = -1,
            .in_WL = 1.239852e-05,

            .out_DX = 3103.9106911246749,
            .out_DZ = 5.0771666330055218,
        },
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6935030407867075, 0, 96.032777495754004),
                 .m_direction = glm::dvec3(0, 1, 0),
             },
         .in_normal = glm::dvec4(0, 1, 0, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1034.8685185321938,
         .out_DZ = -13.320120179862876},
        {.in_ray = {.m_position =
                        glm::dvec3(-5.047050067282087, 4.4859372100394515,
                                   29.182033770349552),
                    .m_direction =
                        glm::dvec3(0.05047050067282087, 0.95514062789960552,
                                   -0.29182033770349552)},
         .in_normal = glm::dvec4(0.05047050067282087, 0.95514062789960552,
                                 -0.29182033770349552, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 4045.0989844091882,
         .out_DZ = -174.2085626048659},
        {.in_ray =
             {
                 .m_position =
                     glm::dvec3(-1.6802365843267262, 1.3759250917712356,
                                16.445931214643075),
                 .m_direction =
                     glm::dvec3(0.016802365843267261, 0.98624074908228765,
                                -0.16445931214643075),
             },
         .in_normal = glm::dvec4(0.016802365843267261, 0.98624074908228765,
                                 -0.16445931214643075, 0),
         .in_imageType = 0,
         .in_rzpType = 0,
         .in_derivationMethod = 0,
         .in_zOffsetCenter = 0,
         .in_risag = 100,
         .in_rosag = 500,
         .in_rimer = 100,
         .in_romer = 500,
         .in_alpha = 0.017453292519943295,
         .in_beta = 0.017453292519943295,
         .in_Ord = -1,
         .in_WL = 1.239852e-05,
         .out_DX = 1418.1004208892475,
         .out_DZ = 253.09836635775162},
    };

    for (auto p : inouts) {
        double DX;
        double DZ;
        CPP_TRACER::RZPLineDensity(p.in_ray, p.in_normal, p.in_imageType,
                                   p.in_rzpType, p.in_derivationMethod,
                                   p.in_zOffsetCenter, p.in_risag, p.in_rosag,
                                   p.in_rimer, p.in_romer, p.in_alpha,
                                   p.in_beta, p.in_Ord, p.in_WL, DX, DZ);
        CHECK_EQ(DX, p.out_DX);
        CHECK_EQ(DZ, p.out_DZ);
    }
}

TEST_F(TestSuite, testRayMatrixMult) {
    struct InOutPair {
        Ray in_ray;
        glm::dmat4 in_matrix;

        Ray out_ray;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_ray =
                {
                    .m_position = glm::dvec3(0, 0, 0),
                    .m_direction = glm::dvec3(0, 0, 0),
                },
            .in_matrix = glm::dmat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16),
            .out_ray =
                {
                    .m_position = glm::dvec3(13, 14, 15),
                    .m_direction = glm::dvec3(0, 0, 0),
                },
        },
        {
            .in_ray =
                {
                    .m_position = glm::dvec3(1, 1, 0),
                    .m_direction = glm::dvec3(0, 1, 1),
                },
            .in_matrix = glm::dmat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16),
            .out_ray =
                {
                    .m_position = glm::dvec3(19, 22, 25),
                    .m_direction = glm::dvec3(14, 16, 18),
                },
        },
        {
            .in_ray =
                {
                    .m_position = glm::dvec3(1, 2, 3),
                    .m_direction = glm::dvec3(4, 5, 6),
                },
            .in_matrix = glm::dmat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16),
            .out_ray =
                {
                    .m_position = glm::dvec3(51, 58, 65),
                    .m_direction = glm::dvec3(83, 98, 113),
                },
        },

    };

    for (auto p : inouts) {
        auto out_ray = CPP_TRACER::rayMatrixMult(p.in_ray, p.in_matrix);
        CHECK_EQ(out_ray, p.out_ray);
    }
}

TEST_F(TestSuite, testDPow) {
    struct InOutPair {
        double in_a;
        int in_b;

        double out;
    };

    std::vector<InOutPair> inouts = {{
                                         .in_a = 0,
                                         .in_b = 0,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = 0,
                                         .in_b = 1,
                                         .out = 0,
                                     },
                                     {
                                         .in_a = 0,
                                         .in_b = -1,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = 2,
                                         .in_b = 0,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = 2,
                                         .in_b = 1,
                                         .out = 2,
                                     },
                                     {
                                         .in_a = 3,
                                         .in_b = 7,
                                         .out = 2187,
                                     },
                                     {
                                         .in_a = 0,
                                         .in_b = 4,
                                         .out = 0,
                                     },
                                     {
                                         .in_a = 0,
                                         .in_b = -4,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = 0,
                                         .in_b = 2,
                                         .out = 0,
                                     },
                                     {
                                         .in_a = 0.20000000000000001,
                                         .in_b = 4,
                                         .out = 0.0016000000000000005,
                                     },
                                     {
                                         .in_a = 9.9949999999999992,
                                         .in_b = 3,
                                         .out = 998.50074987499977,
                                     },
                                     {
                                         .in_a = 3.1415926535897931,
                                         .in_b = 6,
                                         .out = 961.38919357530415,
                                     },
                                     {
                                         .in_a = -1,
                                         .in_b = -4,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = -1,
                                         .in_b = 3,
                                         .out = -1,
                                     },
                                     {
                                         .in_a = -1,
                                         .in_b = 0,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = -1,
                                         .in_b = 4,
                                         .out = 1,
                                     },
                                     {
                                         .in_a = -1,
                                         .in_b = 5,
                                         .out = -1,
                                     },
                                     {
                                         .in_a = -1,
                                         .in_b = 6,
                                         .out = 1,
                                     }};

    for (auto p : inouts) {
        auto out = CPP_TRACER::dpow(p.in_a, p.in_b);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testCosini) {
    struct InOutPair {
        double in_phi;
        double in_psi;

        glm::dvec3 out_direction;
    };

    std::vector<InOutPair> inouts = {
        {
            .in_phi = 0,
            .in_psi = 0,
            .out_direction =
                glm::dvec3(6.1257422745431001e-17, -6.1257422745431001e-17, 1),
        },
        {
            .in_phi = 1,
            .in_psi = 1,
            .out_direction = glm::dvec3(
                0.45464871341284091, -0.8414709848078965, 0.29192658172642888),
        },
        {
            .in_phi = 1,
            .in_psi = 0,
            .out_direction =
                glm::dvec3(0.8414709848078965, -6.1257422745431001e-17,
                           0.54030230586813977),
        },
        {
            .in_phi = 0,
            .in_psi = 1,
            .out_direction =
                glm::dvec3(3.3097526760895799e-17, -0.8414709848078965,
                           0.54030230586813977),
        },
        {
            .in_phi = 3.1415926535897931,
            .in_psi = 3.1415926535897931,
            .out_direction =
                glm::dvec3(-6.1257422745430988e-17, -6.1257422745431001e-17,
                           0.99999999999999978),
        },
        {
            .in_phi = 0,
            .in_psi = 1.5707963267948966,
            .out_direction =
                glm::dvec3(3.7524718414124473e-33, -1, 6.1257422745431001e-17),
        },

    };

    for (auto p : inouts) {
        Ray out_ray;
        CPP_TRACER::cosini(out_ray, p.in_phi, p.in_psi);
        CHECK_EQ(out_ray.m_direction, p.out_direction);
    }
}

TEST_F(TestSuite, testFact) {
    struct InOutPair {
        int in;

        double out;
    };

    std::vector<InOutPair> inouts = {
        {
            .in = 2,
            .out = 2,
        },
        {
            .in = 1,
            .out = 1,
        },
        {
            .in = 0,
            .out = 1,
        },
        {
            .in = 17,
            .out = 355687428096000,
        },
        {
            .in = 4,
            .out = 24,
        },
        {
            .in = -1,
            .out = -1,
        },
        {
            .in = -2,
            .out = -2,
        },
        {
            .in = 0,
            .out = 1,
        },
        {
            .in = 12,
            .out = 479001600,
        },
        {
            .in = -4,
            .out = -4,
        },
        {
            .in = 10,
            .out = 3628800,
        },
        {
            .in = 0,
            .out = 1,
        },

    };

    for (auto p : inouts) {
        auto out = CPP_TRACER::fact(p.in);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testBessel1) {
    struct InOutPair {
        double in;

        double out;
    };

    std::vector<InOutPair> inouts = {
        {
            .in = 100,
            .out = 0,
        },
        {
            .in = 20.100000000000001,
            .out = 0,
        },
        {
            .in = -12.122999999999999,
            .out = 0,
        },
        {
            .in = 23.100000000000001,
            .out = 0,
        },
        {
            .in = 0,
            .out = 0,
        },
        {
            .in = 20,
            .out = 0.066833545658411236,
        },
        {
            .in = -0.10000000000000001,
            .out = 0,
        },
        {
            .in = 1e-08,
            .out = 5.0000000000000001e-09,
        },
        {
            .in = 2,
            .out = 0.57672480775687363,
        },
        {
            .in = 12.122999999999999,
            .out = -0.21368198451302897,
        },
        {
            .in = 3.1415926535897931,
            .out = 0.28461534317975273,
        },
        {
            .in = 10.199999999999999,
            .out = -0.0066157432977083167,
        },
        {
            .in = 19.989999999999998,
            .out = 0.065192988349741909,
        },
        {
            .in = 4,
            .out = -0.06604332802354923,
        },

    };

    for (auto p : inouts) {
        auto out = CPP_TRACER::bessel1(p.in);
        CHECK_EQ(out, p.out);
    }
}

TEST_F(TestSuite, testVlsGrating) {
    struct InOutPair {
        double in_lineDensity;
        double in_z;
        double in_vls[6];

        double out;
    };

    std::vector<InOutPair> inouts = {{
                                         .in_lineDensity = 0.01239852,
                                         .in_z = 5.0020783775947848,
                                         .in_vls = {0, 0, 0, 0, 0, 0},
                                         .out = 0.01239852,
                                     },
                                     {
                                         .in_lineDensity = 0.01239852,
                                         .in_z = 5.0020783775947848,
                                         .in_vls = {1, 2, 3, 4, 5, 6},
                                         .out = 9497.4799596119265,
                                     }};

    for (auto p : inouts) {
        auto out = CPP_TRACER::vlsGrating(p.in_lineDensity, p.in_z, p.in_vls);
        CHECK_EQ(out, p.out);
    }
}
