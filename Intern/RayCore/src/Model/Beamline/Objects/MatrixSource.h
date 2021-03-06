#pragma once
#include <Data/xml.h>

#include <filesystem>

#include "Model/Beamline/LightSource.h"

namespace RAYX {

class RAYX_API MatrixSource : public LightSource {
  public:
    MatrixSource(const std::string name, int numberOfRays,
                 EnergyDistribution dist, const double sourceWidth,
                 const double sourceHeight, const double sourceDepth,
                 const double horDivergence, const double verDivergence,
                 const double linPol0, const double linPol45,
                 const double circPol,
                 const std::array<double, 6> misalignment);

    MatrixSource();
    ~MatrixSource();

    static std::shared_ptr<MatrixSource> createFromXML(xml::Parser);

    virtual std::vector<Ray> getRays() const override;

  private:
    int m_numberOfRays;
};

}  // namespace RAYX
