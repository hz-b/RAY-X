#include "Application.h"

#include <iomanip>
#include <iostream>

#include "Data/Exporter.h"
#include "Debug/Instrumentor.h"
#include "Model/Beamline/Objects/Objects.h"
#include "UserParameter/GeometricUserParams.h"
#include "UserParameter/WorldUserParams.h"
#include "Debug.h"

namespace RAYX {

Application::Application() : m_Beamline(std::make_shared<Beamline>()) {
    #ifdef RAYX_PLATFORM_WINDOWS
    #ifdef RAY_DEBUG_MODE
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif
    #endif
    RAYX_PROFILE_BEGIN_SESSION("RayX Application", "profile_rayx_application.json");
    RAYX_D_LOG << "Creating Application...";
}

Application::~Application() {
    RAYX_D_LOG << "Deleting Application...";
    RAYX_PROFILE_END_SESSION();
}

void Application::loadDummyBeamline() {  // ! objects are created here
                                         // temporarily until reading in file
                                         // works
    RAYX_PROFILE_FUNCTION();

    RAYX::SimulationEnv::get().m_numOfRays = 20;

    EnergyDistribution dist = EnergyDistribution(EnergyRange(100, 0), true);
    std::shared_ptr<MatrixSource> matSourcePtr = std::make_shared<MatrixSource>(
        "matrix source", dist, 0.065, 0.04, 0.0, 0.001, 0.001, 1, 0, 0,
        std::vector<double>{0, 0, 0, 0});
    // std::shared_ptr<Slit> s = std::make_shared<Slit>("slit", 1, 2, 20,
    // 2, 7.5, 10000, 20, 1, m->getPhotonEnergy(), std::vector<double>{2, 1, 0,
    // 0, 0, 0 }, nullptr, GLOBAL);

    // plane mirror with misalignment
    RAYX::GeometricUserParams pm_params = RAYX::GeometricUserParams(10);
    RAYX::WorldUserParams w_coord =
        RAYX::WorldUserParams(pm_params.getAlpha(), pm_params.getBeta(), 0,
                              10000, std::vector<double>{0, 0, 0, 0, 0, 0});
    glm::dvec4 pos1 = w_coord.calcPosition();
    glm::dmat4x4 or1 = w_coord.calcOrientation();
    std::shared_ptr<RAYX::PlaneMirror> pm = std::make_shared<RAYX::PlaneMirror>(
        "pm_ell_ip_200mirrormis", Geometry::GeometricalShape::RECTANGLE, 50,
        200, w_coord.getAzimuthalAngle(), pos1, or1,
        std::vector<double>{0, 0, 0, 0, 0, 0, 0});

    RAYX::Geometry::GeometricalShape geometricalShape =
        RAYX::Geometry::GeometricalShape::RECTANGLE;
    int curvatureType = 0;
    double incidenceAngle =
        0;  // if incidence angle = 0 and mount = 1 use dAlpha as incidenceAngle
    double deviationAngle = 0;  // does not matter bc mount = 1
    int mount = 1;              // incidence
    double exitAngle = 0;
    int imageType = 1;
    int additionalOrder = 1;
    double widthA = 50.0;
    double height = 200.0;
    double designEnergy = 100;
    double orderOfDiffraction = 1;
    double designOrderOfDiffraction = -1;
    double azimuthalAngle = 0;  // rad
    double dAlpha = 1;          // degree
    double dBeta = 1;
    double sEntrance = 100;
    double sExit = 500;
    double mEntrance = 100;
    double mExit = 500;
    double shortRadius = 0;
    double longRadius = 0;
    double fresnelOffset = 0;
    double azimuthal = 10;
    std::vector<double> mis = {1, 2, 3, 0.001, 0.002, 0.003};
    std::vector<double> sE = {1, 2, 3, 4, 5, 6, 7};
    RAYX::GeometricUserParams gu_rzp = RAYX::GeometricUserParams(
        mount, imageType, deviationAngle, incidenceAngle, exitAngle, 100,
        designEnergy, orderOfDiffraction, designOrderOfDiffraction, dAlpha,
        dBeta, mEntrance, mExit, sEntrance, sExit);
    RAYX::WorldUserParams rzp_param = RAYX::WorldUserParams(
        gu_rzp.getAlpha(), gu_rzp.getBeta(), degToRad(azimuthal), 10000,
        std::vector<double>{0, 0, 0, 0, 0, 0});
    glm::dvec4 pos2 = rzp_param.calcPosition();
    glm::dmat4x4 or2 = rzp_param.calcOrientation();

    std::shared_ptr<ReflectionZonePlate> rzpPtr =
        std::make_shared<ReflectionZonePlate>(
            "RZP", geometricalShape, curvatureType, widthA, 60.0, height,
            azimuthalAngle, pos2, or2, designEnergy, orderOfDiffraction,
            designOrderOfDiffraction, dAlpha, dBeta, sEntrance, sExit,
            mEntrance, mExit, shortRadius, longRadius, additionalOrder,
            fresnelOffset, sE);

    // image plane
    RAYX::WorldUserParams ip_w_coord = RAYX::WorldUserParams(
        0, 0, 0, 1000, std::vector<double>{0, 0, 0, 0, 0, 0});
    glm::dvec4 pos3 = ip_w_coord.calcPosition(w_coord, pos1, or1);
    glm::dmat4x4 or3 = ip_w_coord.calcOrientation(w_coord, or1);
    std::shared_ptr<RAYX::ImagePlane> i =
        std::make_shared<RAYX::ImagePlane>("ImagePlane", pos3, or3);

    m_Beamline->addOpticalElement(pm);
    m_Beamline->addOpticalElement(i);

    m_Presenter = Presenter(m_Beamline);
    m_Presenter.addLightSource(matSourcePtr);
}

void Application::run() {
    RAYX_D_LOG << "Application running...";
}

}  // namespace RAYX
