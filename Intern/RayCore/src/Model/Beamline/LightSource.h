#pragma once
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <random>

#include "glm.hpp"

#include "BeamlineObject.h"
#include "Core.h"
#include "Tracer/Ray.h"
#include "Presenter/SimulationEnv.h"

namespace RAYX
{
    class RAYX_API LightSource : public BeamlineObject
    {
    public:
        enum SPREAD_TYPE { ES_WHITE_BAND, ES_THREE_ENERGIES }; // default ES_WHITE_BAND
        enum ENERGY_DISTRIBUTION_TYPE { ET_FILE, ET_VALUES, ET_TOTAL, ET_PARAM }; // default ET_VALUES
        enum SOURCE_DISTRIBUTION_TYPE { ST_SIMULTANEOUS, ST_HARD_EDGE, ST_GAUSS }; // default simultaneously

        LightSource(const char* name, const int spreadType, const double photonEnergy, const double energySpread, const double linPol0, const double linPol45, const double circPol, const std::vector<double> misalignment, const double sourceDepth, const double sourceHeight, const double sourceWidth, const double horDivergence, const double verDivergence);
        LightSource(const char* name, const int spreadType, const double photonEnergy, const double energySpread, const double linPol0, const double linPol45, const double circPol, const std::vector<double> misalignment);

        // Getter
        int getNumberOfRays();
        int getId();
        std::vector<double> getMisalignmentParams();
        double getPhotonEnergy();
        double getEnergySpread();
        double getLinear0();
        double getLinear45();
        double getCircular();
        int getSpreadType();

        double selectEnergy();
        void setNumberOfRays(int numberOfRays);
        glm::dvec3 getDirectionFromAngles(double phi, double psi);
        // get the rays according to specific light source, has to be implemented in each class that inherits from LightSource
        virtual std::vector<Ray> getRays() = 0;

        LightSource();
        virtual ~LightSource();




    protected:
        // Geometric Parameters 
        double m_sourceDepth;
        double m_sourceHeight;
        double m_sourceWidth;
        double m_horDivergence;
        double m_verDivergence;

        std::uniform_real_distribution<double> m_uniformDist;
        std::normal_distribution<double> m_normDist;
        std::default_random_engine m_randEngine;

    private:
        // User/Design Parameter
        std::vector<double> m_misalignmentParams; // x, y, psi, phi

        // Physics Parameters
        // point source & matrix source
        SPREAD_TYPE m_energySpreadType;
        double m_photonEnergy; ///< mid point
        double m_energySpread; ///< distance to other two points
        ENERGY_DISTRIBUTION_TYPE m_energyDistributionType;
        double m_linearPol_0;
        double m_linearPol_45;
        double m_circularPol;

        // TODO(Jannis): move to children
        SOURCE_DISTRIBUTION_TYPE m_sourceDistributionType;
    };

} // namespace RAYX