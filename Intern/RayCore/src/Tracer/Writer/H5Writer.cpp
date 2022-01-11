#include "H5Writer.hpp"

#include <sstream>

H5Writer::H5Writer() : m_file("output.h5", H5Easy::File::Overwrite) {}

void H5Writer::appendRays(const std::vector<double>& outputRays, size_t index) {
    std::stringstream ss;
    ss << "/data/";
    ss << index;

    H5Easy::dump(m_file, ss.str(), outputRays);
}