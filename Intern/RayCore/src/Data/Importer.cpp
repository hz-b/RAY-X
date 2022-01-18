#include "Importer.h"

#include <Data/xml.h>
#include <string.h>

#include <fstream>
#include <sstream>

#include "Debug.h"
#include "Debug/Instrumentor.h"
#include "Model/Beamline/Objects/Objects.h"
#include "rapidxml.hpp"

namespace RAYX {
Importer::Importer() {}

Importer::~Importer() {}

void addBeamlineObjectFromXML(rapidxml::xml_node<>* node, Beamline* beamline,
                              const std::vector<xml::Group>& group_context) {
    const char* type = node->first_attribute("type")->value();

    // the following three blocks of code are lambda expressions (see
    // https://en.cppreference.com/w/cpp/language/lambda) They define functions
    // to be used in the if-else-chain below to keep it structured and readable.

    // addLightSource(s, node) is a function adding a light source to the
    // beamline (if it's not nullptr)
    const auto addLightSource = [&](std::shared_ptr<LightSource> s,
                                    rapidxml::xml_node<>* node) {
        if (s) {
            beamline->m_LightSources.push_back(s);
        } else {
            RAYX_ERR << "could not construct LightSource with Name: "
                     << node->first_attribute("name")->value()
                     << "; Type: " << node->first_attribute("type")->value();
        }
    };

    // addOpticalElement(e, node) is a function adding an optical element to the
    // beamline (if it's not nullptr)
    const auto addOpticalElement = [&](std::shared_ptr<OpticalElement> e,
                                       rapidxml::xml_node<>* node) {
        if (e) {
            beamline->m_OpticalElements.push_back(e);
        } else {
            RAYX_ERR << "could not construct OpticalElement with Name: "
                     << node->first_attribute("name")->value()
                     << "; Type: " << node->first_attribute("type")->value();
        }
    };

    // calcSourceEnergy does some validity checks and then yields the
    // source-energy required by the Slit::createFromXML function
    const auto calcSourceEnergy = [&] {
        assert(!beamline->m_LightSources.empty());
        assert(beamline->m_LightSources[0]);
        return beamline->m_LightSources[0]->m_EnergyDistribution.getAverage();
    };

    // every beamline object has a function createFromXML which constructs the
    // object from a given xml-node if possible (otherwise it will return a
    // nullptr) The createFromXML functions use the param* functions declared in
    // <Data/xml.h>
    if (strcmp(type, "Point Source") == 0) {
        addLightSource(PointSource::createFromXML(node), node);
    } else if (strcmp(type, "Matrix Source") == 0) {
        addLightSource(MatrixSource::createFromXML(node), node);
    } else if (strcmp(type, "ImagePlane") == 0) {
        addOpticalElement(ImagePlane::createFromXML(node, group_context), node);
    } else if (strcmp(type, "Plane Mirror") == 0) {
        addOpticalElement(PlaneMirror::createFromXML(node, group_context),
                          node);
    } else if (strcmp(type, "Toroid") == 0) {
        addOpticalElement(ToroidMirror::createFromXML(node, group_context),
                          node);
    } else if (strcmp(type, "Slit") == 0) {
        addOpticalElement(
            Slit::createFromXML(node, calcSourceEnergy(), group_context), node);
    } else if (strcmp(type, "Spherical Grating") == 0) {
        addOpticalElement(SphereGrating::createFromXML(node, group_context),
                          node);
    } else if (strcmp(type, "Plane Grating") == 0) {
        addOpticalElement(PlaneGrating::createFromXML(node, group_context),
                          node);
    } else if (strcmp(type, "Sphere") == 0) {
        addOpticalElement(SphereMirror::createFromXML(node, group_context),
                          node);
    } else if (strcmp(type, "Reflection Zoneplate") == 0) {
        addOpticalElement(
            ReflectionZonePlate::createFromXML(node, group_context), node);
    } else if (strcmp(type, "Ellipsoid") == 0) {
        addOpticalElement(Ellipsoid::createFromXML(node, group_context), node);
    } else if (strcmp(type, "Cylinder") == 0) {
        addOpticalElement(Cylinder::createFromXML(node, group_context), node);
    } else {
        RAYX_ERR << "could not classify beamline object with Name: "
                 << node->first_attribute("name")->value()
                 << "; Type: " << node->first_attribute("type")->value();
    }
}

/** `collection` is an xml object, over whose children-objects we want to
 * iterate in order to add them to the beamline.
 * `collection` may either be a <beamline> or a <group>. */
void handleObjectCollection(rapidxml::xml_node<>* collection,
                            Beamline* beamline,
                            std::vector<xml::Group>* group_context) {
    for (rapidxml::xml_node<>* object = collection->first_node(); object;
         object = object->next_sibling()) {  // Iterating through objects
        if (strcmp(object->name(), "object") == 0) {
            addBeamlineObjectFromXML(object, beamline, *group_context);
        } else if (strcmp(object->name(), "group") == 0) {
            xml::Group g;
            bool success = xml::parseGroup(object, &g);
            if (success) {
                group_context->push_back(g);
            } else {
                RAYX_ERR << "parseGroup failed!";
            }
            handleObjectCollection(object, beamline, group_context);
            if (success) {
                group_context->pop_back();
            }
        } else if (strcmp(object->name(), "param") != 0) {
            RAYX_ERR << "received weird object->name(): " << object->name();
        }
    }
}

Beamline Importer::importBeamline(const char* filename) {
    RAYX_PROFILE_FUNCTION();
    // first implementation: stringstreams are slow; this might need
    // optimization
    RAYX_LOG << "importBeamline is called with file \"" << filename << "\"";

    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string test = buffer.str();
    if (test.empty()) {
        RAYX_ERR
            << "importBeamline could not open file! (or it was just empty)";
        exit(1);
    }
    std::vector<char> cstr(test.c_str(), test.c_str() + test.size() + 1);
    rapidxml::xml_document<> doc;
    doc.parse<0>(cstr.data());

    RAYX_LOG << "\t Version: "
             << doc.first_node("lab")->first_node("version")->value();
    rapidxml::xml_node<>* xml_beamline =
        doc.first_node("lab")->first_node("beamline");

    Beamline beamline;
    std::vector<xml::Group> group_context;
    handleObjectCollection(xml_beamline, &beamline, &group_context);
    return beamline;
}

// ------RAY-UI related. (Used for backwards compatibility to legacy Sequential
// tracing)

BeamlineHierarchy::BeamlineHierarchy() {
    RAYX_LOG << "A new Hierarchy created.";
}

BeamlineHierarchy::~BeamlineHierarchy() {}

// Add Object to Beamline and calculate its position and orientation
void BeamlineHierarchy::addAbstractBeamlineOject(
    double incidence, double entranceArmLength, double exitArmLength,
    int mCoordSys, double mAzimAngle, std::array<double, 6> mis,
    double distancePreceding, BeamlineElementType objectType) {
    GeometricUserParams g_params =
        GeometricUserParams(incidence, entranceArmLength, exitArmLength);

    double tangentAngle = g_params.calcTangentAngle(
        incidence, entranceArmLength, exitArmLength, mCoordSys);
    WorldUserParams w_coord = WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), degToRad(mAzimAngle),
        distancePreceding, mis, tangentAngle);

    glm::vec4 position = w_coord.calcPosition();
    glm::mat4x4 orientation = w_coord.calcOrientation();
    // Push to Vector
    m_AbstractElements.push_back(std::make_shared<AbstractBeamlineElement>(
        distancePreceding, position, orientation, objectType));
}

void BeamlineHierarchy::printBeamlineHierarchy() {
    std::string output_string;
    for (const auto& obj : m_AbstractElements) {
        output_string = output_string + (*obj).getName() + "→";
    }
    output_string.pop_back();
    RAYX_LOG << output_string;
}

std::vector<std::shared_ptr<AbstractBeamlineElement>>
BeamlineHierarchy::getBeamlineElements() const {
    return m_AbstractElements;
}

AbstractBeamlineElement::AbstractBeamlineElement(double distancePreceding,
                                                 glm::vec4 position,
                                                 glm::dmat4x4 orientation,
                                                 BeamlineElementType objectType)
    : m_distancePreceding(distancePreceding),
      m_postion(position),
      m_orientation(orientation),
      m_type(objectType) {}

double AbstractBeamlineElement::getDistancePreceding() const {
    return m_distancePreceding;
}

glm::dvec4 AbstractBeamlineElement::getPosition() const { return m_postion; }
glm::dmat4x4 AbstractBeamlineElement::getOrientation() const {
    return m_orientation;
}

BeamlineElementType AbstractBeamlineElement::getElementType() const {
    return m_type;
}
}  // namespace RAYX
