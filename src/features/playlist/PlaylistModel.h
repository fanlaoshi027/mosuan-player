#pragma once

#include <QString>
#include <QStringList>

class PlaylistModel final
{
public:
    void add(const QString& path);
    void removeAt(int index);
    void clear();
    void moveUp(int index);
    void moveDown(int index);
    int size() const { return m_items.size(); }
    QString at(int index) const { return m_items.value(index); }
    int indexOf(const QString& path) const { return m_items.indexOf(path); }
    const QStringList& items() const { return m_items; }

private:
    QStringList m_items;
};
