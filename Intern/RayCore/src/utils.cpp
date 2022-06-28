#include "utils.h"

#include <sstream>

#include "Debug.h"
#include "Debug/Instrumentor.h"

/**
 * Calculates photon wavelength (nm) from its energy (eV) or
 * vice verse. For example is used for gratings.
 * @see taken from RAYLIB.FOR
 *
 */
double hvlam(double x) {
    if (x == 0) {
        return 0.0;
    }
    return inm2eV / x;
}

/**
 *  Calculates radians from degree.
 *
 *  @param degree Input in degree, to calculate radians from
 */
double degToRad(double degree) { return degree * PI / 180; }

/**
 *  Calculates degree from radians.
 *
 *  @param rad Input in radians, to calculate degree from
 */
double radToDeg(double rad) { return rad * 180 / PI; }

glm::dmat4x4 getRotationMatrix(double dpsi, double dphi, double dchi) {
    RAYX_PROFILE_FUNCTION();
    glm::dmat4x4 misalignmentMatrix =
        glm::dmat4x4(cos(dphi) * cos(dchi),
                     -cos(dpsi) * sin(dchi) - sin(dpsi) * sin(dphi) * cos(dchi),
                     -sin(dpsi) * sin(dchi) + cos(dpsi) * sin(dphi) * cos(dchi),
                     0, sin(dchi) * cos(dphi),
                     cos(dpsi) * cos(dchi) - sin(dpsi) * sin(dphi) * sin(dchi),
                     sin(dpsi) * cos(dchi) + cos(dpsi) * sin(dphi) * sin(dchi),
                     0, -sin(dphi), -sin(dpsi) * cos(dphi),
                     cos(dpsi) * cos(dphi), 0, 0, 0, 0, 1);
    return glm::transpose(misalignmentMatrix);
}

void printDMatrix(std::array<double, 4 * 4> matrix) {
    RAYX_PROFILE_FUNCTION();
    std::stringstream s;
    s << "\t";
    for (int i = 0; i < 4 * 4; i++) {
        s << matrix[i] << ", ";
        if (i % 4 == 3) {
            RAYX_LOG << s.str();
            s.str("");
            s << "\t";
        }
    }
    RAYX_LOG << s.str();
}

void printDVec4(glm::dvec4 vec) {
    RAYX_PROFILE_FUNCTION();
    std::stringstream s;
    s.precision(17);
    for (int i = 0; i < 4; i++) {
        s << vec[i] << ", ";
    }
    RAYX_LOG << s.str();
}

void RAYX_API printDMat4(glm::dmat4 matrix) {
    RAYX_PROFILE_FUNCTION();
    std::stringstream s;
    s.precision(17);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            s << matrix[i][j] << ", ";
        }
        RAYX_LOG << s.str();
        s.str("");
    }
    RAYX_LOG << s.str();
}

std::array<double, 4 * 4> glmToArray16(glm::dmat4x4 m) {
    std::array<double, 4 * 4> matrix = {
        m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]};
    return matrix;
}

glm::dmat4x4 RAYX_API arrayToGlm16(std::array<double, 4 * 4> m) {
    glm::dmat4x4 matrix =
        glm::dmat4x4(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9],
                     m[10], m[11], m[12], m[13], m[14], m[15]);
    return matrix;
}

std::array<double, 4> glmToArray4(glm::dvec4 v) {
    std::array<double, 4> a = {v[0], v[1], v[2], v[3]};
    return a;
}

glm::dvec4 RAYX_API arrayToGlm4(std::array<double, 4> v) {
    return glm::dvec4(v[0], v[1], v[2], v[3]);
}

/**
 * @brief Moves Source Vector at the end of destination Vector.
 *
 * Source Vector is empty after calling this function!
 * @author
 * //stackoverflow.com/questions/17010005/how-to-use-c11-move-semantics-to-append-vector-contents-to-another-vector
 * @param srcVector Source vector to move.
 * @param destVector Destiation vector to move into.
 * @return vector<T>::iterator Iterator to the destination Vector
 */
std::vector<double>::iterator movingAppend(std::vector<double>&& srcVector,
                                           std::vector<double>& destVector) {
    typename std::vector<double>::iterator result;

    if (destVector.empty()) {
        destVector = std::move(srcVector);
        result = std::begin(destVector);
    } else {
        result =
            destVector.insert(std::end(destVector),
                              std::make_move_iterator(std::begin(srcVector)),
                              std::make_move_iterator(std::end(srcVector)));
    }

    srcVector.clear();
    srcVector.shrink_to_fit();

    return result;
}

/**
 * @brief Checks if Matrix is the Identiy matrix;
 *
 * @param matrix Matrix to check
 * @return true
 * @return false
 */
bool isIdentMatrix(glm::dmat4x4 matrix) { return (matrix == glm::dmat4x4(1.0)); }