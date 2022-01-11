#pragma once

#include <highfive/H5Easy.hpp>

#include "Writer.hpp"

class H5Writer : public Writer {
  public:
    H5Writer();
    virtual ~H5Writer() = default;
    virtual void appendRays(const std::vector<double>& rays, size_t index);

  private:
    H5Easy::File m_file;
};