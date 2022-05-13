#include "Material.h"

#include <Debug.h>
#include <strings.h>

#include <iostream>

#include "NffTable.h"

/**
 * returns the name of the material:
 * EXAMPLE: getMaterialName(Material::Cu) == "CU"
 **/
const char* getMaterialName(Material m) {
    switch (m) {
        case Material::VACUUM:
            return "VACUUM";
        case Material::REFLECTIVE:
            return "REFLECTIVE";

#define X(e, z, a, rho) \
    case Material::e:   \
        return #e;
#include "materials.xmacro"
#undef X
    }
    RAYX_ERR << "unknown material in getMaterialName()!";
    return nullptr;
}

// std::vector over all materials
std::vector<Material> allNormalMaterials() {
    std::vector<Material> mats;
#define X(e, z, a, rho) mats.push_back(Material::e);
#include "materials.xmacro"
#undef X
    return mats;
}

bool materialFromString(const char* matname, Material* out) {
    for (auto m : allNormalMaterials()) {
        const char* name = getMaterialName(m);
        if (strcasecmp(name, matname) == 0) {
            *out = m;
            return true;
        }
    }
    RAYX_ERR << "materialFromString(): material \"" << matname
             << "\" not found";
    return false;
}

MaterialTables loadMaterialTables() {
    MaterialTables out;

    auto mats = allNormalMaterials();
    for (size_t i = 0; i < mats.size(); i++) {
        NffTable t;

        if (!NffTable::load(getMaterialName(mats[i]), &t)) {
            RAYX_ERR << "could not load NffTable!";
        }

        out.indexTable.push_back(out.materialTable.size() /
                                 4);  // 4 doubles per Nff Entry
        for (auto x : t.m_Lines) {
            out.materialTable.push_back(x.m_energy);
            out.materialTable.push_back(x.m_f1);
            out.materialTable.push_back(x.m_f2);
            out.materialTable.push_back(0);
        }
    }

    return out;
}