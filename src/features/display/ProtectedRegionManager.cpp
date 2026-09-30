#include "ProtectedRegionManager.h"

namespace Mosuan {

void ProtectedRegionManager::addRegion(const ProtectedRegion& region)
{
    m_regions.push_back(region);
}

void ProtectedRegionManager::clear()
{
    m_regions.clear();
}

const std::vector<ProtectedRegion>& ProtectedRegionManager::regions() const
{
    return m_regions;
}

bool ProtectedRegionManager::contains(int x, int y) const
{
    for (const auto& region : m_regions) {
        if (region.enabled() && region.contains(x, y)) {
            return true;
        }
    }

    return false;
}

}
