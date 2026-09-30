#pragma once

#include "ProtectedRegion.h"
#include <vector>

namespace Mosuan {

class ProtectedRegionManager
{
public:
    void addRegion(const ProtectedRegion& region);
    void clear();

    const std::vector<ProtectedRegion>& regions() const;

    bool contains(int x, int y) const;

private:
    std::vector<ProtectedRegion> m_regions;
};

}
