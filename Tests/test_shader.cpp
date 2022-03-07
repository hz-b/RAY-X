#include "setupTests.h"

// #if RUN_TEST_SHADER TODO (Jannis): Commented out by OS: error: unterminated
// #if

#include <Tracer/Vulkan/Material.h>

#include <fstream>
#include <functional>
#include <sstream>
#include <type_traits>

#include "Tracer/Vulkan/VulkanTracer.h"

#if RUN_TEST_SHADER
RAYX::VulkanTracer tracer;

/** using this function is preferable to directly adding your test with `#ifndef
 * CI`, because with `if (!shouldDoVulkanTests()) { GTEST_SKIP(); }`
 * your test will still be *compiled* even if CI is enabled
 */
bool shouldDoVulkanTests() {
#ifdef CI
    return false;
#else
    return true;
#endif
}

/**
 * testing suite "Tracer"
 */
class Tracer : public ::testing::Test {
  protected:
    /** this is run before the first test in the testing Suite
     * before first test in test suite "Tracer" is run, initialize the tracer
     * tracer will be a shared instance among all tests
     */
    static void SetUpTestSuite() {
        std::cout << "initialize Vulkantracer instance" << std::endl;
        tracer = RAYX::VulkanTracer();
    }

    /**
     * SetUp is run directly before each test
     */
    virtual void SetUp() {
        if (!shouldDoVulkanTests()) {
            GTEST_SKIP();
        }
    }
    /** this is run after the last test of the testing suite
     * run after last test of suite "Tracer", cleans up the shared instance of
     * tracer
     */
    static void TearDownTestSuite() {
        tracer.cleanup();
        std::cout << "clear tracer instance" << std::endl;
    }
};

std::array<double, 4 * 4> zeros = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0};  // 16 zeros for filling the optical elements for test
                           // cases
std::array<double, 7> zeros7 = {0, 0, 0, 0, 0, 0, 0};  // for slope error

/** runs beamline in "elements" with rays in "testValues"
 * @param testValues        contains rays
 * @param elements          contains optical elements that form the beamline
 * @return output rays, result from tracer after tracing the given rays in the
 * given beamline
 */
std::list<double> runTracer(
    std::vector<RAYX::Ray> testValues,
    std::vector<std::shared_ptr<RAYX::OpticalElement>> elements) {
    std::list<std::vector<RAYX::Ray>> rayList;
    // set beamline parameters (number of beamlines (1), number of elements,
    // number of rays)
    std::cout << "set beamline parameters" << std::endl;
    tracer.setBeamlineParameters(1, elements.size(), testValues.size());

    // add rays
    std::cout << "testValues.size(): " << testValues.size() << std::endl;
    (tracer).addRayVector(std::move(testValues));
    std::cout << "add rays to tracer done" << std::endl;

    // add elements
    for (std::shared_ptr<RAYX::OpticalElement> element : elements) {
        tracer.addArrays(element->getSurfaceParams(), element->getInMatrix(),
                         element->getOutMatrix(),
                         element->getObjectParameters(),
                         element->getElementParameters());
    }
    // execute tracing
    tracer.run();  // run tracer
    std::list<double> outputRays;
    // get resulting rays from tracer
    std::vector<RAYX::Ray> outputRayVector = *(tracer.getOutputIteratorBegin());
    // convert to a list of doubles in order pos, weight, dir, energy, stokes,
    // pathlength, order, lastElement, extraParam
    for (auto iter = outputRayVector.begin(); iter != outputRayVector.end();
         iter++) {
        outputRays.push_back((*iter).getxPos());
        outputRays.push_back((*iter).getyPos());
        outputRays.push_back((*iter).getzPos());
        outputRays.push_back((*iter).getWeight());
        outputRays.push_back((*iter).getxDir());
        outputRays.push_back((*iter).getyDir());
        outputRays.push_back((*iter).getzDir());
        outputRays.push_back((*iter).getEnergy());
        outputRays.push_back((*iter).getS0());
        outputRays.push_back((*iter).getS1());
        outputRays.push_back((*iter).getS2());
        outputRays.push_back((*iter).getS3());
        outputRays.push_back((*iter).getPathLength());
        outputRays.push_back((*iter).getOrder());
        outputRays.push_back((*iter).getLastElement());
        outputRays.push_back((*iter).getExtraParam());
    }
    std::cout << "got " << outputRays.size() << " values from shader"
              << std::endl;
    // empties buffers etc to reuse the tracer instance with a new beamline and
    // new rays
    tracer.cleanTracer();
    return outputRays;
}

/** writes doubles in outputRays to a file with name "rays"
 * the list of output rays is expected to be in the order pos, weight, dir,
 * energy, stokes, pathLength, order, lastElement, extraParam
 * @param outputRays        list of doubles containing ray paramters
 * @param name              name of csv file in which the rays are written
 * (without ".csv" ending)
 */
void writeToFile(std::list<double> outputRays, std::string name) {
    std::cout << "writing to file..." << name << std::endl;
    std::ofstream outputFile;
    // set the precision to 17
    outputFile.precision(17);
    std::cout.precision(17);

    // relative path to folder from bin/build
    std::string filename = resolvePath("Tests/output/");
    filename.append(name);
    filename.append(".csv");
    // create file
    outputFile.open(filename);
    char sep = ';';  // file is saved in .csv (comma seperated value), excel
                     // compatibility is manual right now
    outputFile << "Index" << sep << "Xloc" << sep << "Yloc" << sep << "Zloc"
               << sep << "Weight" << sep << "Xdir" << sep << "Ydir" << sep
               << "Zdir" << sep << "Energy" << sep << "S0" << sep << "S1" << sep
               << "S2" << sep << "S3" << sep << "pathLength" << sep << "order"
               << sep << "lastElement" << sep << "extraParam" << std::endl;
    // outputFile << "Index,Xloc,Yloc,Zloc,Weight,Xdir,Ydir,Zdir" << std::endl;

    size_t counter = 0;
    bool print = false;  // whether to print on std::out
    //
    for (std::list<double>::iterator i = outputRays.begin();
         i != outputRays.end(); i++) {
        // beginning of ray -> put index
        if (counter % RAY_DOUBLE_COUNT == 0) {
            outputFile << counter / VULKANTRACER_RAY_DOUBLE_AMOUNT;
            if (print) std::cout << ")" << std::endl;
            if (print) std::cout << "(";
        }
        // print seperator and current value
        outputFile << sep << *i;
        // end of ray -> new line
        if (counter % RAY_DOUBLE_COUNT == RAY_DOUBLE_COUNT - 1) {
            outputFile << std::endl;
            counter++;
            continue;
        }
        // for better readability in std::out
        if (counter % RAY_DOUBLE_COUNT == 3) {
            if (print) std::cout << ") ";
        } else if (counter % RAY_DOUBLE_COUNT == 4) {
            if (print) std::cout << " (";
        } else if (counter % RAY_DOUBLE_COUNT != 0) {
            if (print) std::cout << ", ";
        }
        // print current value to std out
        if (print) std::cout << *i;
        counter++;
    }
    if (print) std::cout << ")" << std::endl;
    // close the file
    outputFile.close();
    std::cout << "done!" << std::endl;
}

/**
 * encodes test values (max. 16 values) in a Ray because that is how they can be
put to the shader
 * you need to make sure that the corresponding test on shader side reads the
values correctly!
 * @param testValues        vector of rays in which the test settings are stored
 * @param pos               holds 3 doubles
 * @param dir               holds 3 doubles
 * @param stokes            holds 4 doubles
 * @param energy            1 double
 * @param weight            1 double
 * @param pathLength        1 double
 * @param order             1 double
 * @param lastElement       1 double
 * @param extraParam        1 double
 */
std::vector<RAYX::Ray> addTestSetting(
    std::vector<RAYX::Ray> testValues, glm::dvec3 pos = glm::dvec3(0, 0, 0),
    glm::dvec3 dir = glm::dvec3(0, 0, 0),
    glm::dvec4 stokes = glm::dvec4(0, 0, 0, 0), double energy = 0,
    double weight = 0, double pathLength = 0, double order = 0,
    double lastElement = 0, double extraParam = 0) {
    RAYX::Ray r =
        RAYX::Ray::makeRayFrom(pos, dir, stokes, energy, weight, pathLength,
                               order, lastElement, extraParam);
    testValues.push_back(r);
    return testValues;
}

/**
 * use this to run the Unit test with the id "unittestid" ("settings" on shader)
 * with the given testValues. The function return the resulting values. test
 * values are always encoded in rays since this is the only way to get values to
 * the shader
 * @param unittestid            defines which unit test should be executed on
 * shader. if = 0, then the normal tracing is done
 * @param testValues            vector of rays that contains the input values
 * for the unit test, which value is stored where has to be the same on both
 * shader and c++ test setup.
 * @return list of doubles that contains the resulting values after running the
 * shader
 *
 */
std::list<double> runUnitTest(double unittestid,
                              std::vector<RAYX::Ray> testValues) {
    std::shared_ptr<RAYX::OpticalElement> q =
        std::make_shared<RAYX::OpticalElement>(
            "qq",
            std::array<double, 4 * 4>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      unittestid, 0, 0},
            zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, {q});
    std::cout << "got " << outputRays.size() << " values from shader"
              << std::endl;
    return outputRays;
}

/**
 * runs given beamline on shader with deterministic matrix source (no randomness
 * in rays) writes resulting rays to a csv file with the name "testFile_"+name
 * of first element in beamline
 * @param elements      vector of optical elements that form the beamline to
 * test
 * @param n             number of rays that should be used
 *
 */
void testOpticalElement(
    std::vector<std::shared_ptr<RAYX::OpticalElement>> elements, int n) {
    RAYX::SimulationEnv::get().m_numOfRays = n;
    RAYX::EnergyDistribution dist(RAYX::EnergyRange(100, 0), true);
    std::shared_ptr<RAYX::MatrixSource> m =
        std::make_shared<RAYX::MatrixSource>(
            "Matrix source 1", dist, 0.065, 0.04, 0.0, 0.001, 0.001, 1, 0, 0,
            std::array<double, 6>{0, 0, 0, 0, 0, 0});
    // run tracer with rays from source and elements from vector
    std::list<double> outputRays = runTracer(m->getRays(), elements);
    // write to file "testFile_"+name of first element in beamlin
    std::string filename = "testFile_";
    std::cout << elements[0]->getName();
    filename.append(elements[0]->getName());
    writeToFile(outputRays, filename);
}

/**
 * loads beamline from rml file, traces beamline and writes resulting rays in
 * csv file with the name "testFile_" + filename
 * @param filename      name of rml file without ending .rml
 */
void testBeamline(const char* filename) {
    std::string beamline_file = resolvePath("Tests/rml_files/test_shader/");
    beamline_file.append(filename);
    beamline_file.append(".rml");
    std::shared_ptr<RAYX::Beamline> beamline = std::make_shared<RAYX::Beamline>(
        RAYX::importBeamline(beamline_file.c_str()));

    std::string outfile = "testFile_";
    outfile.append(filename);

    std::vector<std::shared_ptr<RAYX::OpticalElement>> elements =
        beamline->m_OpticalElements;
    std::vector<RAYX::Ray> testValues = beamline->m_LightSources[0]->getRays();

    std::list<double> outputRays = runTracer(testValues, elements);
    // write to file "testFile_"+name of first element in beamlin
    writeToFile(outputRays, outfile);
}

/** used to compare correct values and results in unit tests that use the shader
 * @param correct           vector of rays that we expect from the tracer
 * @param outputRays        list of doubles that contains ray parameters for
 * each ray in the order pos, weight, dir, energy, stokes, pathLength, order,
 * lastElement, extraParam
 * @param tolerance         the max. difference that is allowed between expected
 * and result
 */
void compareFromCorrect(std::vector<RAYX::Ray> correct,
                        std::list<double> outputRays, double tolerance) {
    int counter = 0;
    for (std::list<double>::iterator i = outputRays.begin();
         i != outputRays.end();) {
        if (counter % RAY_DOUBLE_COUNT == 0) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_position.x,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 1) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_position.y,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 2) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_position.z,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 3) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_weight,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 4) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_direction.x,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 5) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_direction.y,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 6) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_direction.z,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 7) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_energy,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 8) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_stokes.x,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 9) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_stokes.y,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 10) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_stokes.z,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 11) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_stokes.w,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 12) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_pathLength,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 13) {
            EXPECT_NEAR(*i, correct[int(counter / RAY_DOUBLE_COUNT)].m_order,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 14) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_lastElement,
                        tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 15) {
            EXPECT_NEAR(*i,
                        correct[int(counter / RAY_DOUBLE_COUNT)].m_extraParam,
                        tolerance);
        }
        counter++;
        i++;
    }
}

template <typename ret, typename par>
using fn = std::function<ret(par)>;

/** if we test a simple function of the form y = f(x) like exp, sin, log,..
 * on the shader we can use this function to verify its result
 * @param func          the function
 * @param testValues    the values for which we test the function. vector of
 * rays but the parameters are meaningless, just store values there to get
 * them to the shader
 * @param outputRays    the rays that we get from the shader. contain the
 * results for applying the function to each of the testValues on the shader
 *
 * applies func to each value in testValues and compares with outputRays to
 * verify that our implementation of func on the shader is correct
 */
template <typename ret, typename par>
void compareFromFunction(fn<ret, par> func, std::vector<RAYX::Ray> testValues,
                         std::list<double> outputRays, double tolerance) {
    int counter = 0;
    for (std::list<double>::iterator i = outputRays.begin();
         i != outputRays.end();) {
        if (counter % RAY_DOUBLE_COUNT == 0) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_position.x),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 1) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_position.y),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 2) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_position.z),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 3) {
            EXPECT_NEAR(
                *i, func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_weight),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 4) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_direction.x),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 5) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_direction.y),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 6) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_direction.z),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 7) {
            EXPECT_NEAR(
                *i, func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_energy),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 8) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_stokes.x),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 9) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_stokes.y),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 10) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_stokes.z),
                tolerance);
        } else if (counter % RAY_DOUBLE_COUNT == 11) {
            EXPECT_NEAR(
                *i,
                func(testValues[int(counter / RAY_DOUBLE_COUNT)].m_stokes.w),
                tolerance);
        }
        counter++;
        i++;
    }
}

// UNIT TESTS

/** test random uniform number generator on shader
 * does not actually test the randomness but only if between 0 and 1
 */
TEST_F(Tracer, testUniformRandom) {
    double settings = 17;

    // we want 2000 * RAY_DOUBLE_COUNT test values
    RAYX::SimulationEnv::get().m_numOfRays = 2000;
    RAYX::EnergyDistribution dist(RAYX::EnergyRange(100, 0), true);
    std::shared_ptr<RAYX::MatrixSource> m =
        std::make_shared<RAYX::MatrixSource>(
            "Matrix source 1", dist, 0.065, 0.04, 0.0, 0.001, 0.001, 1, 0, 0,
            std::array<double, 6>{0, 0, 0, 0, 0, 0});
    // create 2000 rays that are put to the shader. they will be overwritten
    // by the random numbers
    std::vector<RAYX::Ray> testValues = m->getRays();

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double sum = 0;
    // expect all values to be between 0 and 1
    for (std::list<double>::iterator i = outputRays.begin();
         i != outputRays.end(); i++) {
        sum += *i;
        ASSERT_TRUE(*i <= 1.0);
        ASSERT_TRUE(*i >= 0.0);
    }
    // expect the mean of all values to be about 0.5
    EXPECT_NEAR(sum / (RAYX::SimulationEnv::get().m_numOfRays * 12), 0.5, 1e-2);
    std::string filename = "testFile_randomUniform";
    writeToFile(outputRays, filename);
}

/** test the exponential function e^x
 */
TEST_F(Tracer, ExpTest) {
    double settings = 18;
    // create n random rays (pos, dir, energy, weight and stokes of the ray
    // is each a uniformly distributed random number between low and high)
    int n = 10;
    int low = -4;
    int high = 4;
    RAYX::SimulationEnv::get().m_numOfRays = n;
    RAYX::RandomRays random = RAYX::RandomRays(low, high);

    std::vector<RAYX::Ray> testValues = random.getRays();
    // add some values manually
    RAYX::Ray r = {0,  1,        -3, 5,     PI, 2, 3, 4,
                   10, -4.41234, 0,  1.224, 0,  0, 0, 0};
    testValues.push_back(r);

    std::list<double> outputRays = runUnitTest(settings, testValues);

    // compare result from shader (outputRays) with CPU function exp(x) on
    // testValues
    double tolerance = 1e-13;
    auto expfun = fn<double, double>([](double x) { return exp(x); });
    compareFromFunction(expfun, testValues, outputRays, tolerance);
}

/** test natural logarithm function
 * @see ExpTest
 */
TEST_F(Tracer, LogTest) {
    double settings = 19;

    int n = 10;
    int low = 1;
    int high = 4;
    RAYX::SimulationEnv::get().m_numOfRays = n;
    RAYX::RandomRays random = RAYX::RandomRays(low, high);

    std::vector<RAYX::Ray> testValues = random.getRays();
    testValues = addTestSetting(
        testValues, glm::dvec3(0.1, 1, 0.3), glm::dvec3(PI, 2, 3),
        glm::dvec4(0.2345, 100, 3.423453, 0.00000001), 2.1, 5);

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-13;
    auto logfun = fn<double, double>([](double x) { return log(x); });
    compareFromFunction(logfun, testValues, outputRays, tolerance);
}

/**
 * test refrac2D function
 * for grating with lines in both dimensions (RZP eg?)
 * each test case:
 * input:   normal at intersection point
 *          direction of incoming ray
 *          weight of incoming ray
 *          az refraction parameter (Wavelength * line density * order *
 * 1e-6) in one dimension ax refraction parameter in other dimension output:
 * direction of ray after refraction weight of ray after refraction
 */
TEST_F(Tracer, testRefrac2D) {
    double settings = 16;

    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    glm::dvec3 direction = glm::dvec3(
        0.0001666666635802469, -0.017285764670739875, 0.99985057611723738);
    double weight = 1.0;
    double az = 0.00016514977645243345;
    double ax = 0.012830838024391771;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(az, ax, 0, 0), 0, weight);
    glm::dvec3 expected_dir = glm::dvec3(
        -0.012664171360811521, 0.021648721107426414, 0.99968542634078494);
    double expected_weight = 1.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_dir,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 2nd test case
    normal = glm::dvec3(0, 1, 0);
    direction = glm::dvec3(0.00049999999722222275, -0.017285762731583675,
                           0.99985046502305308);
    weight = 1.0;
    az = -6.2949352042540596e-05;
    ax = 0.038483898782123105;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(az, ax, 0, 0), 0, weight);
    expected_dir = glm::dvec3(0.00049999999722222275, -0.017285762731583675,
                              0.99985046502305308);
    expected_weight = 0.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_dir,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 3rd test case
    normal = glm::dvec3(0, 1, 0),
    direction = glm::dvec3(0.0001666666635802469, -0.017619047234249029,
                           0.99984475864845179);
    weight = 1.0;
    az = -0.077169530850327184;
    ax = 0.2686127340088395;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(az, ax, 0, 0), 0, weight);
    expected_dir = glm::dvec3(0.0001666666635802469, -0.017619047234249029,
                              0.99984475864845179);
    expected_weight = 0.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_dir,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 4th test case
    // normal != 0 (spherical RZP)
    normal = glm::dvec3(0.050470500672820856, 0.95514062789960541,
                        -0.29182033770349547);
    direction = glm::dvec3(-0.000499999916666667084, -0.016952478247434233,
                           0.99985617139734351);
    weight = 1.0;
    az = 0.0021599283476277926;
    ax = -0.050153240660177005;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(az, ax, 0, 0), 0, weight);
    expected_dir = glm::dvec3(0.080765992839840872, 0.57052382524991363,
                              0.81730007905468893);
    expected_weight = 1.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_dir,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    std::list<double> outputRays = runUnitTest(settings, testValues);
    double tolerance = 1e-12;
    compareFromCorrect(correct, outputRays, tolerance);
}

/** test normal_cartesian function that rotates a given normal around the x
 * and z axis with two given angles
 * @param normal        given normal vector
 * @param slopeX        given angle for x rotation
 * @param slopeZ        given angle for z rotation
 * @return normal       resulting normal
 */
TEST_F(Tracer, testNormalCartesian) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    double slopeX = 0;
    double slopeZ = 0;
    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    // encode: ray.position.x = slopeX, ray.position.z = slopeZ.
    // ray.direction = normal at intersection point from eg quad fct.
    glm::dvec3 expected_normal = glm::dvec3(0, 1, 0);
    correct = addTestSetting(correct, expected_normal);

    // normal != (0,1,0), slope still = 0
    slopeX = 0;
    slopeZ = 0;
    normal =
        glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    // normal unchanged
    expected_normal =
        glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537);
    correct = addTestSetting(correct, expected_normal);

    // normal = (0,1,0), slopeX = 2, slopeZ = 3
    slopeX = 2;
    slopeZ = 3;
    normal = glm::dvec3(0, 1, 0);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    expected_normal = glm::dvec3(-0.90019762973551742, 0.41198224566568298,
                                 -0.14112000805986721);
    correct = addTestSetting(correct, expected_normal);

    slopeX = 2;
    slopeZ = 3;
    normal =
        glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    expected_normal = glm::dvec3(-9431.2371568647086, 4310.7269916467494,
                                 -1449.3435640204684);
    correct = addTestSetting(correct, expected_normal);

    double settings = 13;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;  // smallest possible
    // return format = pos (0=x,1=y,2=z), 3=weight, dir (4=x,5=y,6=z), 7=0
    compareFromCorrect(correct, outputRays, tolerance);
}

/** test normal_cylindrical function that rotates a given normal cylindrical
 * @param normal        given normal vector
 * @param slopeX        given angle for x rotation
 * @param slopeZ        given angle for z rotation
 * @return normal       resulting normal
 */
TEST_F(Tracer, testNormalCylindrical) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    // encode: ray.position.x = slopeX, ray.position.z = slopeZ.
    // ray.direction = normal at intersection point from quad fct.
    double slopeX = 0;
    double slopeZ = 0;
    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    glm::dvec3 expected_normal = glm::dvec3(0, 1, 0);
    correct = addTestSetting(correct, expected_normal);

    // normal != (0,1,0), slope still = 0
    slopeX = 0;
    slopeZ = 0;
    normal =
        glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    expected_normal =
        glm::dvec3(5.0465463027115769, 10470.451695989539, -28.532199794465537);
    correct = addTestSetting(correct, expected_normal);

    // normal = (0,1,0), different slopes
    slopeX = 2;
    slopeZ = 3;
    normal = glm::dvec3(0, 1, 0);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    expected_normal = glm::dvec3(0.90019762973551742, 0.41198224566568292,
                                 -0.14112000805986721);
    correct = addTestSetting(correct, expected_normal);

    // different normal
    slopeX = 2;
    slopeZ = 3;
    normal =
        glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537);
    testValues =
        addTestSetting(testValues, glm::dvec3(slopeX, 0, slopeZ), normal);
    expected_normal =
        glm::dvec3(9431.2169472441783, 4310.7711493493844, -1449.3437356459144);
    correct = addTestSetting(correct, expected_normal);

    double settings = 14;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-11;  // smallest possible
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * test refrac function of gratings
 * @param normal                normal at intersection point
 * @param direction             direction of incoming ray
 * @param weight                weight of incoming ray
 * @param a                     paramter calculated from line density,
 * wavelength, order of diffraction
 * @return expected_direction   direction of diffracted ray
 * @return expected_weight      weight of diffracted ray
 */
TEST_F(Tracer, testRefrac) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    double a = 0.01239852;
    // encode: ray.position = normal at intersection point. ray.direction =
    // direction of ray, ray.weigth = weight of ray plane surface
    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    glm::dvec3 direction = glm::dvec3(
        -0.00049999991666667084, -0.99558611855684065, 0.09385110834192622);
    double weight = 1;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), a, weight);
    // store correct resulting weight in weight and calculated direction in
    // direction
    glm::dvec3 expected_direction = glm::dvec3(
        -0.00049999991666667084, 0.99667709206767885, 0.08145258834192623);
    double expected_weight = 1.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 2nd test case, different direction
    normal = glm::dvec3(0, 1, 0);
    direction = glm::dvec3(-0.000016666664506172893, -0.995586229182718,
                           0.093851118714515264);
    weight = 1.0;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), a, weight);
    expected_direction = glm::dvec3(-0.000016666664506160693,
                                    0.9966772027014974, 0.081452598714515267);
    expected_weight = 1.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 3rd case: spherical grating (different normal), same a
    a = 0.01239852;
    normal = glm::dvec3(0.0027574667592826954, 0.99999244446428082,
                        -0.0027399619384214182);
    direction = glm::dvec3(-0.00049999991666667084, -0.99558611855684065,
                           0.093851108341926226);
    weight = 1.0;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), a, weight);
    expected_direction = glm::dvec3(0.0049947959329671825, 0.99709586573547515,
                                    0.07599267429701162);
    expected_weight = 1.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 4th test case: ray beyond horizon -> weight to 0
    a = -0.038483898782123105;
    normal = glm::dvec3(0.0, 1.0, 0.0);
    direction = glm::dvec3(-0.99991341437509562, 0.013149667401360443,
                           -0.00049999997222215965);
    weight = 1.0;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), a, weight);
    expected_direction = glm::dvec3(-0.99991341437509562, 0.013149667401360443,
                                    -0.00049999997222215965);
    expected_weight = 0.0;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    double settings = 15;
    std::list<double> outputRays = runUnitTest(settings, testValues);
    double tolerance = 1e-12;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * test wastebox function that checks if intersection is withing the bounds of
 * the optical element
 * @param pos                   intersection with element
 * @param xLength               width of element
 * @param yLength               height of element
 * @param weight                weight of incoming ray
 * @param expected_weight       the same as "weight" when intersection within
 * bound of element, 0 else
 */
TEST_F(Tracer, testWasteBox) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    // encode: ray.position = position of intersection point.
    // ray.direction.x = xLength of opt. element, ray.direction.z = zLength
    // of optical element, ray.weigth = weight of ray before calling
    // wastebox case: intersection point on surface
    glm::dvec3 pos = glm::dvec3(-5.0466620698997637, 0, 28.760236725599515);
    double xLength = 50;
    double zLength = 200;
    double weight = 1.0;
    testValues =
        addTestSetting(testValues, pos, glm::dvec3(xLength, 0, zLength),
                       glm::dvec4(0, 0, 0, 0), 0, weight);
    double expected_weight = 1.0;
    // store correct resulting weight in position of resulting ray
    correct = addTestSetting(correct, glm::dvec3(expected_weight, 0, 0));

    // 2nd test case: intersection point not on surface
    pos = glm::dvec3(-5.0466620698997637, 0, 28.760236725599515);
    xLength = 5;
    zLength = 20;
    weight = 1.0;
    testValues =
        addTestSetting(testValues, pos, glm::dvec3(xLength, 0, zLength),
                       glm::dvec4(0, 0, 0, 0), 0, weight);
    expected_weight = 0.0;
    correct = addTestSetting(correct, glm::dvec3(expected_weight, 0, 0));

    // 3rd test case: intersection point not on surface
    pos = glm::dvec3(-1.6822205656320104, 0, 28.760233508097873);
    xLength = 5;
    zLength = 20;
    weight = 1;
    testValues =
        addTestSetting(testValues, pos, glm::dvec3(xLength, 0, zLength),
                       glm::dvec4(0, 0, 0, 0), 0, weight);
    expected_weight = 0.0;
    correct = addTestSetting(correct, glm::dvec3(expected_weight, 0, 0));

    // 4th case: ray already had weight 0
    pos = glm::dvec3(-5.0466620698997637, 0, 28.760236725599515);
    xLength = 50;
    zLength = 200;
    weight = 0;
    testValues =
        addTestSetting(testValues, pos, glm::dvec3(xLength, 0, zLength),
                       glm::dvec4(0, 0, 0, 0), 0, weight);
    expected_weight = 0.0;
    // store correct resulting weight in weight of c
    correct = addTestSetting(correct, glm::dvec3(expected_weight, 0, 0));

    double settings = 11;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-10;
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * test calculation of RZP line density at intersection point
 * @param inputValues       (see comment in test)
 * @param position          intersection
 * @param normal            normal at intersection
 * @return expected_DX, DZ  line density in x and z dimension of
 * element at intersection point
 */
TEST_F(Tracer, testRZPLineDensityDefaulParams) {  // point to point
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // {1st column, 2nd column, 3rd column, 4th column}
    // {image_type, rzp_type, derivation_method, zOffsetCenter}, ->
    // point2point(0), elliptical(0), formulas(0), 0 {risag, rosag, rimer,
    // romer}, {d_alpha, d_beta, d_ord, wl}, {0,0,0,0}
    std::array<double, 4 * 4> inputValues = {0,
                                             0,
                                             0,
                                             0,
                                             100,
                                             500,
                                             100,
                                             500,
                                             0.017453292519943295,
                                             0.017453292519943295,
                                             -1,
                                             12.39852 * 1e-06,
                                             0,
                                             0,
                                             0,
                                             0};

    // encode: ray.position = position of test ray. ray.direction = normal
    // at intersection point.
    glm::dvec3 position =
        glm::dvec3(-5.0805095016939532, 0, 96.032788311782269);
    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    testValues = addTestSetting(testValues, position, normal);
    double expected_DX = 3103.9106911246745;
    double expected_DZ = 5.0771666329965663;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    // 2nd test case
    position = glm::dvec3(-1.6935030407867075, 0, 96.032777495754004);
    normal = glm::dvec3(0, 1, 0);
    testValues = addTestSetting(testValues, position, normal);
    expected_DX = 1034.8685185321933;
    expected_DZ = -13.320120179862874;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    // 3rd test case: spherical (normal != (0,1,0))
    position =
        glm::dvec3(-5.047050067282087, 4.4859372100394515, 29.182033770349552);
    normal = glm::dvec3(0.05047050067282087, 0.95514062789960552,
                        -0.29182033770349552);
    testValues = addTestSetting(testValues, position, normal);
    expected_DX = 4045.0989844091873;
    expected_DZ = -174.20856260487483;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    // 4th test case
    position =
        glm::dvec3(-1.6802365843267262, 1.3759250917712356, 16.445931214643075);
    normal = glm::dvec3(0.016802365843267261, 0.98624074908228765,
                        -0.16445931214643075);
    testValues = addTestSetting(testValues, position, normal);
    expected_DX = 1418.1004208892471;
    expected_DZ = 253.09836635775156;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    double settings = 12;
    std::shared_ptr<RAYX::OpticalElement> q1 =
        std::make_shared<RAYX::OpticalElement>(
            "testRZPpoint2point",
            std::array<double, 4 * 4>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      settings, 0, 0},
            inputValues, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, {q1});

    double tolerance = 1e-10;
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * similar as before except that "type of imaging" is astigmatic to astigmatic
 * instead of point to point
 */
TEST_F(Tracer, testRZPLineDensityAstigmatic) {  // astigmatic 2 astigmatic
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // {1st column, 2nd column, 3rd column, 4th column} ->
    // astigmatic2astigmatic(1), elliptical(0), formulas(0), 0 {image_type,
    // rzp_type, derivation_method, zOffsetCenter}, {risag, rosag, rimer,
    // romer}, {d_alpha, d_beta, d_ord, wl}, {0,0,0,0}
    std::array<double, 4 * 4> inputValues = {1,
                                             0,
                                             0,
                                             0,
                                             100,
                                             500,
                                             100,
                                             500,
                                             0.017453292519943295,
                                             0.017453292519943295,
                                             -1,
                                             12.39852 * 1e-06,
                                             0,
                                             0,
                                             0,
                                             0};

    // encode: ray.position = position of test ray. ray.direction = normal
    // at intersection point.
    glm::dvec3 position =
        glm::dvec3(-5.0805095016939532, 0, 96.032788311782269);
    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    testValues = addTestSetting(testValues, position, normal);
    double expected_DX = 3103.9106911246745;
    double expected_DZ = 5.0771666329965663;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    // 2nd test case
    position = glm::dvec3(-1.6935030407867075, 0, 96.032777495754004);
    normal = glm::dvec3(0, 1, 0);
    testValues = addTestSetting(testValues, position, normal);
    expected_DX = 1034.8685185321933;
    expected_DZ = -13.320120179862874;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    // 3rd test case: spherical (normal != (0,1,0))
    position =
        glm::dvec3(-5.047050067282087, 4.4859372100394515, 29.182033770349552);
    normal = glm::dvec3(0.05047050067282087, 0.95514062789960552,
                        -0.29182033770349552);
    testValues = addTestSetting(testValues, position, normal);
    expected_DX = 4045.0989844091873;
    expected_DZ = -174.20856260487483;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    // 4th test case
    position =
        glm::dvec3(-1.6802365843267262, 1.3759250917712356, 16.445931214643075);
    normal = glm::dvec3(0.016802365843267261, 0.98624074908228765,
                        -0.16445931214643075);
    testValues = addTestSetting(testValues, position, normal);
    expected_DX = 1418.1004208892471;
    expected_DZ = 253.09836635775156;
    correct = addTestSetting(correct, glm::dvec3(expected_DX, 0, expected_DZ));

    double settings = 12;
    std::shared_ptr<RAYX::OpticalElement> q1 =
        std::make_shared<RAYX::OpticalElement>(
            "testRZPAstigmatic",
            std::array<double, 4 * 4>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      settings, 0, 0},
            inputValues, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, {q1});
    std::cout << "got " << outputRays.size() << " values from shader"
              << std::endl;

    double tolerance = 1e-10;
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * test multiplication of Ray with a transformation matrix. used e.g. to
 * transform ray from global to element coordinates
 * @param matrix        transformation matrix 4x4
 * @param position      pos of ray
 * @param direction     dir of ray
 * @return expected_pos position after multiplication
 * @return expected_dir direction after multiplication
 */
TEST_F(Tracer, testRayMatrixMult) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // {1st column, 2nd column, 3rd column, 4th column}
    std::array<double, 4 * 4> matrix = {1, 2,  3,  4,  5,  6,  7,  8,
                                        9, 10, 11, 12, 13, 14, 15, 16};

    // multiply direction and position with matrix
    // in homogeneous coord (dvec4 from dvec3): (pos,1) and (dir, 0)
    glm::dvec3 position = glm::dvec3(0, 0, 0);
    glm::dvec3 direction = glm::dvec3(0, 0, 0);
    testValues = addTestSetting(testValues, position, direction);
    glm::dvec3 expected_pos = glm::dvec3(13, 14, 15);
    glm::dvec3 expected_dir = glm::dvec3(0, 0, 0);
    correct = addTestSetting(correct, expected_pos, expected_dir);

    position = glm::dvec3(1, 1, 0);
    direction = glm::dvec3(0, 1, 1);
    testValues = addTestSetting(testValues, position, direction);
    expected_pos = glm::dvec3(1 + 5 + 13, 2 + 6 + 14, 3 + 7 + 15);
    expected_dir = glm::dvec3(5 + 9, 6 + 10, 7 + 11);
    correct = addTestSetting(correct, expected_pos, expected_dir);

    position = glm::dvec3(1, 2, 3);
    direction = glm::dvec3(4, 5, 6);
    testValues = addTestSetting(testValues, position, direction);
    expected_pos =
        glm::dvec3(1 * 1 + 2 * 5 + 3 * 9 + 13, 1 * 2 + 2 * 6 + 3 * 10 + 14,
                   1 * 3 + 2 * 7 + 3 * 11 + 15);
    expected_dir = glm::dvec3(4 * 1 + 5 * 5 + 6 * 9, 4 * 2 + 5 * 6 + 6 * 10,
                              4 * 3 + 5 * 7 + 6 * 11);
    correct = addTestSetting(correct, expected_pos, expected_dir);

    double settings = 10;
    std::shared_ptr<RAYX::OpticalElement> q1 =
        std::make_shared<RAYX::OpticalElement>(
            "testRayMatrixMult",
            std::array<double, 4 * 4>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      settings, 0, 0},
            matrix, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, {q1});
    std::cout << "got " << outputRays.size() << " values from shader"
              << std::endl;

    double tolerance = 1e-12;
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * test pow(a,b) = a^b = bases ^ exponents function. ray position[i] ^ ray
 * direction[i] for i in {0,1,2}
 * @param bases         3 test values for basis (stored in position)
 * @param exponents     3 test values for exponend (stored in direction)
 * @param expected      3 result values
 */
TEST_F(Tracer, testDPow) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    glm::dvec3 bases = glm::dvec3(0.0, 0, 0);
    glm::dvec3 exponents = glm::dvec3(0, 1, -1);
    testValues = addTestSetting(testValues, bases, exponents);
    glm::dvec3 expected = glm::dvec3(1, 0, 1);
    correct = addTestSetting(correct, expected);

    bases = glm::dvec3(2, 2, 3);
    exponents = glm::dvec3(0, 1, 7);
    testValues = addTestSetting(testValues, bases, exponents);
    expected = glm::dvec3(1, 2, 2187);
    correct = addTestSetting(correct, expected);

    bases = glm::dvec3(0, 0, 0);
    exponents = glm::dvec3(4, -4, 2);
    testValues = addTestSetting(testValues, bases, exponents);
    expected = glm::dvec3(0, 1, 0);
    correct = addTestSetting(correct, expected);

    bases = glm::dvec3(0.2, 19.99 / 2, PI);
    exponents = glm::dvec3(4, 3, 6);
    testValues = addTestSetting(testValues, bases, exponents);
    expected = glm::dvec3(0.0016, 998.50074987499977, 961.38919357530415);
    correct = addTestSetting(correct, expected);

    bases = glm::dvec3(-1.0, -1.0, -1.0);
    exponents = glm::dvec3(-4, 3, 0);
    testValues = addTestSetting(testValues, bases, exponents);
    expected = glm::dvec3(1, -1, 1);
    correct = addTestSetting(correct, expected);

    bases = glm::dvec3(-1.0, -1.0, -1.0);
    exponents = glm::dvec3(4, 5, 6);
    testValues = addTestSetting(testValues, bases, exponents);
    expected = glm::dvec3(1, -1, 1);
    correct = addTestSetting(correct, expected);

    double settings = 7;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;
    compareFromCorrect(correct, outputRays, tolerance);
}

/**
 * test creation of direction vector from 2 angles
 * @param phi               rotation in xz plane
 * @param psi               rotation in yz-plane
 * @return expected_dir     vector that we get when rotating z-axis around these
 * angles
 */
TEST_F(Tracer, testCosini) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // phi, psi given in position.x, position.y
    double phi = 0;
    double psi = 0;
    testValues = addTestSetting(testValues, glm::dvec3(phi, psi, 0));
    glm::dvec3 expected_dir =
        glm::dvec3(sin(phi) * cos(psi), -sin(psi), cos(phi) * cos(psi));
    correct = addTestSetting(correct, expected_dir);

    phi = 1;
    psi = 1;
    testValues = addTestSetting(testValues, glm::dvec3(phi, psi, 0));
    expected_dir =
        glm::dvec3(sin(phi) * cos(psi), -sin(psi), cos(phi) * cos(psi));
    correct = addTestSetting(correct, expected_dir);

    phi = 1;
    psi = 0;
    testValues = addTestSetting(testValues, glm::dvec3(phi, psi, 0));
    expected_dir =
        glm::dvec3(sin(phi) * cos(psi), -sin(psi), cos(phi) * cos(psi));
    correct = addTestSetting(correct, expected_dir);

    phi = 0;
    psi = 1;
    testValues = addTestSetting(testValues, glm::dvec3(phi, psi, 0));
    expected_dir =
        glm::dvec3(sin(phi) * cos(psi), -sin(psi), cos(phi) * cos(psi));
    correct = addTestSetting(correct, expected_dir);

    phi = PI;
    psi = PI;
    testValues = addTestSetting(testValues, glm::dvec3(phi, psi, 0));
    expected_dir = glm::dvec3(0, 0, 1);
    correct = addTestSetting(correct, expected_dir);

    phi = 0;
    psi = PI / 2;
    testValues = addTestSetting(testValues, glm::dvec3(phi, psi, 0));
    expected_dir = glm::dvec3(0, -1, 0);
    correct = addTestSetting(correct, expected_dir);

    double settings = 9;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;
    compareFromCorrect(correct, outputRays, tolerance);
}

// test factorial f(a) = a!
TEST_F(Tracer, factTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    testValues =
        addTestSetting(testValues, glm::dvec3(0, 1, 2), glm::dvec3(-1, 4, 17),
                       glm::dvec4(10, -4, 12.2, -0), 0, -2);

    correct = addTestSetting(correct, glm::dvec3(1, 1, 2),
                             glm::dvec3(-1, 24, 355687428096000),
                             glm::dvec4(3628800, -4, 479001600, 1), 1, -2);

    double settings = 8;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, bessel1Test) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // some test values for besser function
    testValues = addTestSetting(testValues, glm::dvec3(-12.123, 20.1, 100),
                                glm::dvec3(20.0, 0, 23.1),
                                glm::dvec4(0, 0, 0, 0), 0, -0.1);
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0),
                             glm::dvec3(0.066833545658411195, 0, 0),
                             glm::dvec4(0, 0, 0, 0), 0, 0);

    testValues = addTestSetting(testValues, glm::dvec3(12.123, 2, 0.00000001),
                                glm::dvec3(19.99, 10.2, PI),
                                glm::dvec4(0, 0, 0, 0), 0, 4);
    correct = addTestSetting(
        correct, glm::dvec3(-0.21368198451302897, 0.57672480775687363, 5e-09),
        glm::dvec3(0.065192988349741882, -0.0066157432977083167,
                   0.28461534317975273),
        glm::dvec4(0, 0, 0, 0), 0, -0.06604332802354923);

    double settings = 6;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-08;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, diffractionTest) {
    std::vector<RAYX::Ray> testValues;
    // pos = (iopt,  xlenght, ylength) weight = wavelength
    // r = RAYX::Ray(glm::dvec3(1, 50,100), glm::dvec3(0.0,0.0,0.0), 0.1);
    // testValues.push_back(r);
    int n = 10;
    double iopt = 1;
    double xlength = 20;
    double ylength = 2;
    double wavelength = 12.39852;
    for (int i = 0; i < n; i++) {
        testValues = addTestSetting(
            testValues, glm::dvec3(iopt, xlength, ylength),
            glm::dvec3(0.0, 0.0, 0.0), glm::dvec4(0, 0, 0, 0), 0, wavelength);
    }
    double lowerDphi = 1e-10;
    double upperDphi = 1e-06;
    double lowerDpsi = 1e-08;
    double upperDpsi = 1e-05;

    double settings = 5;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    int counter = 0;
    // check that results are within bounds
    for (std::list<double>::iterator i = outputRays.begin();
         i != outputRays.end();) {
        std::cout << *i << ", ";
        if (counter % RAY_DOUBLE_COUNT == 0) {  // position.x
            EXPECT_TRUE(abs(*i) < upperDphi);
            EXPECT_TRUE(abs(*i) > lowerDphi);
        } else if (counter % RAY_DOUBLE_COUNT == 1) {  // position.y
            EXPECT_TRUE(abs(*i) < upperDpsi);
            EXPECT_TRUE(abs(*i) > lowerDpsi);
        }
        counter++;
        i++;
    }
}

TEST_F(Tracer, SinTest) {
    std::list<std::vector<RAYX::Ray>> rayList;
    int n = 10;
    int low = -1;
    int high = 1;
    RAYX::SimulationEnv::get().m_numOfRays = n;
    RAYX::RandomRays random = RAYX::RandomRays(low, high);

    // add some test values
    std::vector<RAYX::Ray> testValues = random.getRays();
    testValues = addTestSetting(testValues, glm::dvec3(0, 1, PI),
                                glm::dvec3(-1, -PI, PI / 2));
    double settings = 1;

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;
    auto sinfun = fn<double, double>([](double x) { return sin(x); });
    compareFromFunction(sinfun, testValues, outputRays, tolerance);
}

TEST_F(Tracer, CosTest) {
    if (!shouldDoVulkanTests()) {
        GTEST_SKIP();
    }

    std::list<std::vector<RAYX::Ray>> rayList;
    int n = 10;
    int low = -1;
    int high = 1;
    RAYX::SimulationEnv::get().m_numOfRays = n;
    RAYX::RandomRays random = RAYX::RandomRays(low, high);

    // add some test values
    std::vector<RAYX::Ray> testValues = random.getRays();
    testValues = addTestSetting(testValues, glm::dvec3(0, 1, PI),
                                glm::dvec3(-1, -PI, PI / 2));
    double settings = 27;
    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;
    auto fun = fn<double, double>([](double x) { return cos(x); });
    compareFromFunction(fun, testValues, outputRays, tolerance);
}

TEST_F(Tracer, AtanTest) {
    std::list<std::vector<RAYX::Ray>> rayList;
    int n = 10;
    int low = -1;
    int high = 1;
    RAYX::SimulationEnv::get().m_numOfRays = n;
    RAYX::RandomRays random = RAYX::RandomRays(low, high);

    // add some test values
    std::vector<RAYX::Ray> testValues = random.getRays();
    testValues = addTestSetting(testValues, glm::dvec3(0, 1, PI),
                                glm::dvec3(-1, -PI, PI / 2));
    double settings = 28;

    std::list<double> outputRays = runUnitTest(settings, testValues);
    double tolerance = 1e-12;
    auto fun = fn<double, double>([](double x) { return atan(x); });
    compareFromFunction(fun, testValues, outputRays, tolerance);
}

// test VLS function that calculates new a from given a, z-position and 6
// vls parameters
TEST_F(Tracer, vlsGratingTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    double z = 5.0020783775947848;
    double a = 0.01239852;
    double settings = 4;

    // encode vls parameters in ray direction and position, a =
    // wl*linedensity*ord*1.e-6 is given as well (in weight of ray)
    glm::dvec3 vls_123 = glm::dvec3(0.0, 0.0, 0.0);
    glm::dvec3 vls_456 = glm::dvec3(0.0, 0.0, 0.0);

    testValues = addTestSetting(testValues, vls_123, vls_456,
                                glm::dvec4(0, 0, 0, 0), z, a);
    // a should remain unchanged if all vls parameters are 0
    double expected_weight = a;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0),
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // use some vls values and compare with A calculated by old ray UI
    vls_123 = glm::dvec3(1, 2, 3);
    vls_456 = glm::dvec3(4, 5, 6);
    testValues = addTestSetting(testValues, vls_123, vls_456,
                                glm::dvec4(0, 0, 0, 0), z, a);
    expected_weight = 9497.479959611925;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0),
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // give setting=4 to start vls test on shader
    std::list<double> outputRays = runUnitTest(settings, testValues);

    // we reduced the precision here from 1e-15, because it didn't work on
    // Rudi's and Oussama's video cards. we presume this inaccuracy is not
    // relevant. interestingly though, it worked on the other video cards.
    // reduce to 1e-11 because control value 9497.479959611925 has no more
    // digits after the comma than that. Apparently different computers
    // handle this differently
    double tolerance = 1e-11;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, planeRefracTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 3;

    // normal (always 0,1,0) encoded in ray position, a encoded in weight,
    // direction in ray direction
    glm::dvec3 normal = glm::dvec3(0.0, 1.0, 0.0);
    glm::dvec3 direction =
        glm::dvec3(0, -0.99558611855684065, 0.09385110834192662);
    double a = 0.01239852;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), 0, a);
    glm::dvec3 expected_direction =
        glm::dvec3(0, 0.99667709206767885, 0.08145258834192623);
    double expected_weight = 0.01239852;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 2nd test
    normal = glm::dvec3(0.0, 1.0, 0.0);
    direction =
        glm::dvec3(0.01239852, -0.99558611855684065, 0.09385110834192662);
    a = 0.01239852;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), 0, a);
    expected_direction =
        glm::dvec3(0.01239852, 0.99667709206767885, 0.08145258834192623),
    expected_weight = 0.01239852;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 3rd test
    normal = glm::dvec3(0.0, 1.0, 0.0);
    direction =
        glm::dvec3(0.01239852, -0.99567947186812988, 0.0928554753392902);
    a = 0.01239852;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), 0, a);
    expected_direction =
        glm::dvec3(0.01239852, 0.99675795875308415, 0.080456955339290204);
    expected_weight = 0.01239852;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 4th test
    normal = glm::dvec3(0.0, 1.0, 0.0);
    direction =
        glm::dvec3(0.01239852, -0.99567947186812988, 0.0928554753392902);
    a = 0.01239852;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), 0, a);
    expected_direction =
        glm::dvec3(0.01239852, 0.99675795875308415, 0.080456955339290204);
    expected_weight = 0.01239852;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 5th test
    normal = glm::dvec3(0, 1, 0);
    direction = glm::dvec3(-0.0004999999166666, -0.99558611855684065,
                           0.093851108341926226);
    a = 0.01239852;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), 0, a);
    expected_direction = glm::dvec3(-0.0004999999166666, 0.99667709206767885,
                                    0.08145258834192623);
    expected_weight = 0.01239852;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    // 6th test
    normal = glm::dvec3(0, 1, 0);
    direction = glm::dvec3(-0.0004999999166666, -0.995586229182718,
                           0.093851118714515264);
    a = 0.01239852;
    testValues = addTestSetting(testValues, normal, direction,
                                glm::dvec4(0, 0, 0, 0), 0, a);
    expected_direction = glm::dvec3(-0.0004999999166666, 0.9966772027014974,
                                    0.081452598714515267);
    expected_weight = 0.01239852;
    correct = addTestSetting(correct, glm::dvec3(0, 0, 0), expected_direction,
                             glm::dvec4(0, 0, 0, 0), 0, expected_weight);

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-12;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, iteratToTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 20;

    // normal (always 0,1,0) encoded in ray position, a encoded in
    // direction.x, direction.y and direction.z are actual ray directions
    glm::dvec3 position =
        glm::dvec3(-0.0175, 1736.4751598838836, -9848.1551798768887);
    glm::dvec3 direction = glm::dvec3(-0.00026923073232438285,
                                      -0.17315574581145807, 0.984894418304465);
    double weight = 1.0;
    double longRadius = 10470.491787499999;
    double shortRadius = 315.72395939400002;
    testValues =
        addTestSetting(testValues, position, direction,
                       glm::dvec4(longRadius, shortRadius, 0, 0), 0, weight);

    glm::dvec3 expected_intersection = glm::dvec3(
        -2.7173752216893443, 0.050407875158271054, 28.473736158432885);
    glm::dvec3 expected_normal = glm::dvec3(
        -0.00026923073232438285, -0.17315574581145807, 0.984894418304465);
    correct = addTestSetting(correct, expected_intersection, expected_normal);

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-09;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, getThetaTest) {
    // put here what you want as input for the shader
    std::vector<RAYX::Ray> testValues;
    // put here what you expect to come back from the shader
    std::vector<RAYX::Ray> correct;
    // defines the test on the shader
    double settings = 21;

    glm::dvec3 normal = glm::dvec3(0, 1, 0);
    glm::dvec3 direction = glm::dvec3(
        -0.00049999997222222275, -0.17381228817387082, 0.98477867487054738);
    testValues = addTestSetting(testValues, normal, direction);
    double expected_theta = 1.3960967569703167;
    correct = addTestSetting(correct, glm::dvec3(expected_theta, 0, 0));

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-14;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, reflectanceTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 22;

    double energy = 100;
    double incidence_angle = 1.3962634006709251;

    testValues =
        addTestSetting(testValues, glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0),
                       glm::dvec4(0, 0, 0, 0), energy, incidence_angle);

    glm::dvec2 exp_spol = glm::dvec2(0.4312494615693625, 0.42810153127435358);
    glm::dvec2 exp_ppol = glm::dvec2(0.4385787674250865, 0.3522102644170379);
    correct = addTestSetting(correct, glm::dvec3(exp_spol, 0),
                             glm::dvec3(exp_ppol, 0));

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-09;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, snellTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 23;

    // 1st test
    glm::dvec2 incidenceCos = glm::dvec2(0.17364817766693041, 0);
    glm::dvec2 refractiveIndex1 = glm::dvec2(1, 0);
    glm::dvec2 refractiveIndex2 =
        glm::dvec2(0.91452118089946777, 0.035187568837614078);
    glm::dvec2 expected = glm::dvec2(0.1090636366167056, 0.4078927218856746);
    testValues = addTestSetting(testValues, glm::dvec3(incidenceCos, 0),
                                glm::dvec3(refractiveIndex1, 0),
                                glm::dvec4(refractiveIndex2, 0, 0));
    correct = addTestSetting(correct, glm::dvec3(expected, 0));

    // 2nd test
    incidenceCos = glm::dvec2(0.17315572500228882, 0);
    refractiveIndex1 = glm::dvec2(1, 0);
    refractiveIndex2 = glm::dvec2(0.91453807092958361, 0.035170965000031584);
    expected = glm::dvec2(0.10897754475504851, 0.40807275584607544);
    // add test setting (input and expected)
    testValues = addTestSetting(testValues, glm::dvec3(incidenceCos, 0),
                                glm::dvec3(refractiveIndex1, 0),
                                glm::dvec4(refractiveIndex2, 0, 0));
    correct = addTestSetting(correct, glm::dvec3(expected, 0));

    // 3rd test
    incidenceCos = glm::dvec2(0.1736481785774231, 0);
    refractiveIndex1 = glm::dvec2(1, 0);
    refractiveIndex2 = glm::dvec2(0.9668422, 6.5589860E-02);
    expected = glm::dvec2(0.24302165191294139, 0.28697207607552583);
    // add test setting (input and expected)
    testValues = addTestSetting(testValues, glm::dvec3(incidenceCos, 0),
                                glm::dvec3(refractiveIndex1, 0),
                                glm::dvec4(refractiveIndex2, 0, 0));
    correct = addTestSetting(correct, glm::dvec3(expected, 0));

    // 4th test
    incidenceCos = glm::dvec2(0.16307067260397731, 0.0027314608130525712);
    refractiveIndex1 = glm::dvec2(0.99816471240025439, 0.00045674598468145697);
    refractiveIndex2 = glm::dvec2(0.99514154037883318, 0.0047593281563246184);
    expected = glm::dvec2(0.14743465849863294, 0.031766878855366755);
    // add test setting (input and expected)
    testValues = addTestSetting(testValues, glm::dvec3(incidenceCos, 0),
                                glm::dvec3(refractiveIndex1, 0),
                                glm::dvec4(refractiveIndex2, 0, 0));
    correct = addTestSetting(correct, glm::dvec3(expected, 0));

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-15;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, fresnelTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 24;

    // 1st test
    glm::dvec2 cn1 = glm::dvec2(0.91453807092958361, 0.035170965000031584);
    glm::dvec2 cn2 = glm::dvec2(1, 0);
    glm::dvec2 cosa1 = glm::dvec2(0.10897754475504851, 0.40807275584607544);
    glm::dvec2 cosa2 = glm::dvec2(0.17315572500228882, 0);

    glm::dvec2 exp_spol = glm::dvec2(0.57163467986230043, 0.62486367906829532);
    glm::dvec2 exp_ppol = glm::dvec2(0.63080662811278621, 0.52640331936127871);
    testValues = addTestSetting(testValues, glm::dvec3(cosa1, 0),
                                glm::dvec3(cosa2, 0), glm::dvec4(cn1, cn2));
    correct = addTestSetting(correct, glm::dvec3(exp_spol, 0),
                             glm::dvec3(exp_ppol, 0));

    // 2nd test
    cn1 = glm::dvec2(0.91452118089946777, 0.035187568837614078);
    cn2 = glm::dvec2(1, 0);
    cosa1 = glm::dvec2(0.10906363669865969, 0.40789272144618016);
    cosa2 = glm::dvec2(0.1736481785774231, 0);
    exp_spol = glm::dvec2(0.56981824812215442, 0.62585833416785819);
    exp_ppol = glm::dvec2(0.62929764490596996, 0.52731592442193231);

    testValues = addTestSetting(testValues, glm::dvec3(cosa1, 0),
                                glm::dvec3(cosa2, 0), glm::dvec4(cn1, cn2));
    correct = addTestSetting(correct, glm::dvec3(exp_spol, 0),
                             glm::dvec3(exp_ppol, 0));

    std::list<double> outputRays = runUnitTest(settings, testValues);
    double tolerance = 1e-15;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, amplitudeTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 25;

    glm::dvec2 complex_1 = glm::dvec2(0.63080662811278621, 0.52640331936127871);
    glm::dvec2 complex_2 = glm::dvec2(0.57163467986230043, 0.62486367906829532);
    glm::dvec2 complex_3 = glm::dvec2(1, 0);
    glm::dvec2 complex_4 = glm::dvec2(0, 1);

    glm::dvec2 exp_1 = glm::dvec2(0.67501745670559532, 0.69542190922049119);
    glm::dvec2 exp_2 = glm::dvec2(0.71722082464004022, 0.82985616444880206);
    glm::dvec2 exp_3 = glm::dvec2(1, 0);
    glm::dvec2 exp_4 = glm::dvec2(1, PI / 2);
    testValues = addTestSetting(testValues, glm::dvec3(complex_1, 0),
                                glm::dvec3(complex_2, 0),
                                glm::dvec4(complex_3, complex_4));
    correct = addTestSetting(correct, glm::dvec3(exp_1, 0),
                             glm::dvec3(exp_2, 0), glm::dvec4(exp_3, exp_4));

    std::list<double> outputRays = runUnitTest(settings, testValues);

    double tolerance = 1e-15;
    compareFromCorrect(correct, outputRays, tolerance);
}

TEST_F(Tracer, palikTest) {
    std::vector<RAYX::Ray> testValues;
    testValues.push_back(RAYX::Ray());

    double settings = 26;

    std::list<double> outputRays = runUnitTest(settings, testValues);

    std::vector<double> v;
    for (auto x : outputRays) {
        v.push_back(x);
    }

    double tolerance = 1e-15;

    // first copper entry
    EXPECT_NEAR(v[0], 1.0, tolerance);
    EXPECT_NEAR(v[1], 0.433, tolerance);
    EXPECT_NEAR(v[2], 8.46, tolerance);

    // copper index
    EXPECT_NEAR(v[3], 0., tolerance);

    // first gold entry
    EXPECT_NEAR(v[4], 0.04959, tolerance);
    EXPECT_NEAR(v[5], 20.3, tolerance);
    EXPECT_NEAR(v[6], 76.992, tolerance);

    // gold index
    EXPECT_NEAR(v[7], 324., tolerance);
}
// test complete optical elements instead of single functions
// uses deterministic source (matrix source with source depth = 0)
// use name of optical element as file name
class opticalElements : public Tracer {};

TEST_F(opticalElements, planeMirrorDefault) {
    const char* filename = "PlaneMirrorDef";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, planeMirrorMis) {
    const char* filename = "PlaneMirrorMis";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, sphereMirror) {
    const char* filename = "SphereMirrorDefault";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, planeGratingDevDefault) {
    const char* filename = "PlaneGratingDeviationDefault";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, planeGratingDevAzimuthal) {
    const char* filename = "PlaneGratingDeviationAz";
    testBeamline(filename);
}

TEST_F(opticalElements, planeGratingDevAzMis) {
    const char* filename = "PlaneGratingDeviationAzMis";
    testBeamline(filename);
}

// constant incidence angle mode, azimuthal angle and misalignment
TEST_F(opticalElements, planeGratingIncAzMis) {
    const char* filename = "PlaneGratingIncAzMis";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, planeGratingDevMisVLS) {
    const char* filename = "PlaneGratingDevAzMisVLS";
    testBeamline(filename);

    ASSERT_TRUE(true);
}

// RZPs

TEST_F(opticalElements, RZPDefaultParams) {
    const char* filename = "ReflectionZonePlateDefault";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, RZPDefaultParams200) {
    const char* filename = "ReflectionZonePlateDefault200";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, RZPDefaultParamsToroid200) {
    const char* filename = "ReflectionZonePlateDefault200Toroid";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, RZPAzimuthal200) {
    const char* filename = "ReflectionZonePlateAzim200";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, RZPMis) {
    const char* filename = "ReflectionZonePlateMis";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

// ellipsoid mirrors

// default ellipsoid with no misalignment but with image plane, data stored
// in image-plane-coordinate system (footprint?)
TEST_F(opticalElements, EllipsoidImagePlane) {
    const char* filename = "ellipsoid_ip_200default";
    testBeamline(filename);

    /*RAYX::GeometricUserParams g_params =
        RAYX::GeometricUserParams(10, 10000, 1000);
    double alpha = 0.031253965260898464;
    double beta = 0.31781188513796743;
    ASSERT_DOUBLE_EQ(g_params.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(g_params.getBeta(), beta);
    int coordinatesystem = 0;  // misalignment in ellipsoid coordinate system
    double tangentAngle =
        g_params.calcTangentAngle(10, 10000, 1000, coordinatesystem);

    RAYX::WorldUserParams w_coord = RAYX::WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), 0, 10000,
        std::array<double, 6>{0, 0, 0, 0, 0, 0}, tangentAngle);
    glm::dvec4 pos = w_coord.calcPosition();
    glm::dmat4x4 or1 = w_coord.calcOrientation();

    std::shared_ptr<RAYX::Ellipsoid> eb = std::make_shared<RAYX::Ellipsoid>(
        "ellipsoid_ip_200default", RAYX::Geometry::GeometricalShape::RECTANGLE,
        50, 200, w_coord.getAzimuthalAngle(), pos, or1, 10, 10000, 1000, 0, 1,
        zeros7, Material::CU);

    RAYX::WorldUserParams w_coord2 = RAYX::WorldUserParams(
        0, 0, 0, 1000, std::array<double, 6>{0, 0, 0, 0, 0, 0});
    glm::dvec4 pos2 = w_coord2.calcPosition(w_coord, pos, or1);
    glm::dmat4x4 or2 = w_coord2.calcOrientation(w_coord, or1);
    std::shared_ptr<RAYX::ImagePlane> i =
        std::make_shared<RAYX::ImagePlane>("ImagePlane", pos2, or2);

    testOpticalElement({eb, i}, 200);*/
}

// default ellipsoid with ELLIPSOID misalignment and with image plane, data
// stored in image-plane-coordinate system
TEST_F(opticalElements, EllipsoidImagePlane_ellipsmisalignment) {
    const char* filename = "ellipsoid_ip_200ellipsmis";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

// default ellipsoid with MIRROR misalignment and with image plane, data
// stored in image-plane-coordinate system
TEST_F(opticalElements, EllipsoidImagePlane_mirrormisalignment) {
    const char* filename = "ellipsoid_ip_200mirrormis";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

// plame mirror with misalignment, default ellipsoid with MIRROR
// misalignment and image plane, data stored in image-plane-coordinate
// system
TEST_F(opticalElements, PlaneMirrorEllipsoidImagePlane_mirrormisalignment) {
    const char* filename = "pm_ell_ip_200mirrormis";
    testBeamline(filename);
}

TEST_F(opticalElements, FourMirrors_20Rays) {
    const char* filename = "globalCoordinates_20rays";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, slit1) {
    std::string beamline_file = resolvePath("Tests/rml_files/test_shader/");
    const char* filename = "slit";
    beamline_file.append(filename);
    beamline_file.append(".rml");
    std::shared_ptr<RAYX::Beamline> beamline = std::make_shared<RAYX::Beamline>(
        RAYX::importBeamline(beamline_file.c_str()));

    std::string outfile = "testFile_";
    outfile.append(filename);

    std::vector<std::shared_ptr<RAYX::OpticalElement>> elements =
        beamline->m_OpticalElements;
    std::vector<RAYX::Ray> testValues = beamline->m_LightSources[0]->getRays();
    std::list<double> outputRays = runTracer(testValues, elements);

    int counter = 0;
    for (std::list<double>::iterator i = outputRays.begin();
         i != outputRays.end();) {
        if (counter % RAY_DOUBLE_COUNT == 0) {  // x loc
            std::list<double>::iterator j = i;
            j++;
            j++;
            if (*(j) == 1) {
                EXPECT_TRUE(
                    abs(*i) <=
                    6);  // if weight == 1 check if x location is correct
            }
        } else if (counter % RAY_DOUBLE_COUNT == 1) {  // y loc
            std::list<double>::iterator j = i;
            j++;
            j++;
            j++;
            if (*j == 1) {  // if weight == 1 check if y location is correct
                EXPECT_TRUE(abs(*i) >= 0.5);
                EXPECT_TRUE(abs(*i) <= 1.3);
            }
        }
        counter++;
        i++;
    }
}

TEST_F(opticalElements, slit2) {
    const char* filename = "slit";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, toroid) {
    const char* filename = "toroid";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

// PETES SETUP

TEST_F(opticalElements, spec1_first_ip) {
    const char* filename = "Spec1-first_rzp4mm";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, spec1_first_plus_ip) {
    const char* filename = "Spec1+first_rzp4mm";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, spec1_first_minus_ip2) {
    const char* filename = "Spec1-first_rzp02mm";
    testBeamline(filename);
    ASSERT_TRUE(true);
}

TEST_F(opticalElements, CylinderDefault) {
    const char* filename = "CylinderDefault";
    testBeamline(filename);
    ASSERT_TRUE(true);
}
#endif
