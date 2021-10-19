#pragma once
#include "Core.h"
#include <vector>
#include <iostream>
#include <stdexcept>

namespace RAYX
{
    /*
    * Brief: Abstract parent class for all beamline objects used in Ray-X.
    *
    */
    class RAYX_API BeamlineObject
    {
    public:
        ~BeamlineObject();

        const char* getName() const;

        const int m_ID;

    protected:
        BeamlineObject(const char* name);
        BeamlineObject();

    private:
        const char* m_name;
        //m_geometry;
        //m_surface; //(for lightsource??)


    };


} // namespace RAYX