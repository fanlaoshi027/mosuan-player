#include "PlaylistModel.h"

void PlaylistModel::add(const QString& path)
{
    if (path.isEmpty() || m_items.contains(path)) return;
    m_items.append(path);
}

void PlaylistModel::removeAt(int index)
{
    if (index >= 0 && index < m_items.size()) m_items.removeAt(index);
}

void PlaylistModel::clear()
{
    m_items.clear();
}
