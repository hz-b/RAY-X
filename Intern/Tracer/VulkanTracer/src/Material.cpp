#include "Material.h"

#include <assert.h>

#include <iostream>

#include "PalikTable.h"

/**
 * returns the name of the material:
 * EXAMPLE: getMaterialName(Material::CU) == "CU"
 **/
const char* getMaterialName(Material m) {
    switch (m) {
#define X(e)          \
    case Material::e: \
        return #e;
#include "materials.xmacro"
#undef X
    }
    std::cerr << "unknown material in getMaterialName()!\n";
    assert(false);
    return nullptr;
}

// std::vector over all materials
std::vector<Material> allMaterials() {
    std::vector<Material> mats;
#define X(e) mats.push_back(Material::e);
#include "materials.xmacro"
#undef X
    return mats;
}

// these tables are private to this file, only read-access is granted to others
// by the functions below
static std::vector<double> MATERIAL_TABLE;
static std::vector<int> MATERIAL_INDEX_TABLE;

// fills the tables above
void fillMaterialTables() {
    auto mats = allMaterials();
    for (uint i = 0; i < mats.size(); i++) {
        PalikTable t;
        assert(PalikTable::load(getMaterialName(mats[i]), &t));
        MATERIAL_INDEX_TABLE.push_back(MATERIAL_TABLE.size() /
                                       4);  // 4 doubles per PalikEntry
        for (auto x : t.m_Lines) {
            MATERIAL_TABLE.push_back(x.m_energy);
            MATERIAL_TABLE.push_back(x.m_n);
            MATERIAL_TABLE.push_back(x.m_k);
            MATERIAL_TABLE.push_back(0);
        }
    }
}

const std::vector<double>* getMaterialTable() {
    if (MATERIAL_TABLE.empty()) {
        fillMaterialTables();
    }
    return &MATERIAL_TABLE;
}

const std::vector<int>* getMaterialIndexTable() {
    if (MATERIAL_TABLE.empty()) {
        fillMaterialTables();
    }
    return &MATERIAL_INDEX_TABLE;
}