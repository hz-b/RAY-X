
#include "PalikTable.h"

#include <Debug.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "PathResolver.h"

bool PalikTable::load(const char* element, PalikTable* out) {
    std::string f = "Data/PALIK/" + std::string(element) + ".NKP";
    auto resolved = resolvePath(f);

    RAYX_WARN << "@David:";
    RAYX_WARN << element;
    RAYX_WARN << f;
    RAYX_WARN << resolved;
    std::ifstream s(resolved);

    if (s.fail()) {
        return false;
    }

    std::string line;

    // ignore first three lines
    for (int i = 0; i < 3; i++) {
        std::getline(s, line);
    }

    // line 4..EOF
    for (uint32_t lineidx = 4; std::getline(s, line); lineidx++) {
        if (line.empty()) {
            continue;
        }

        PalikEntry e;

        if (sscanf(line.c_str(), "%le %le %le", &e.m_energy, &e.m_n, &e.m_k) !=
            3) {
            RAYX_ERR << "Failed to parse PalikTable \"" << element
                     << "\", at line " << lineidx << ": \"" << line << "\"";
            return false;
        }
        out->m_Lines.push_back(e);
    }

    out->m_element = element;

    return true;
}
