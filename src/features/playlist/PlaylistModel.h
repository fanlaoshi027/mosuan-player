#pragma once

#include <QString>
#include <QStringList>

class PlaylistModel final
{
public:
    explicit PlaylistModel(QString storageFile = {});

    void add(const QString& path);
    void removeAt(int index);
    void clear();
    void moveUp(int index);
    void moveDown(int index);
    int size() const { return m_items.size(); }
    QString at(int index) const { return m_items.value(index); }
    int indexOf(const QString& path) const { return m_items.indexOf(path); }
    const QStringList& items() const { return m_items; }

    bool load();
    bool save() const;

private:
    QString m_storageFile;
    QStringList m_items;
};
