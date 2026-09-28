#include "PlaylistModel.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

PlaylistModel::PlaylistModel(QString storageFile)
    : m_storageFile(std::move(storageFile))
{
    if (m_storageFile.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_storageFile = QDir(dir).filePath(QStringLiteral("playlist.json"));
    }
    load();
}

void PlaylistModel::add(const QString& path)
{
    if (path.isEmpty() || m_items.contains(path)) return;
    m_items.append(path);
    save();
}

void PlaylistModel::removeAt(int index)
{
    if (index < 0 || index >= m_items.size()) return;
    m_items.removeAt(index);
    save();
}

void PlaylistModel::clear()
{
    m_items.clear();
    save();
}

void PlaylistModel::moveUp(int index)
{
    if (index <= 0 || index >= m_items.size()) return;
    m_items.swapItemsAt(index, index - 1);
    save();
}

void PlaylistModel::moveDown(int index)
{
    if (index < 0 || index >= m_items.size() - 1) return;
    m_items.swapItemsAt(index, index + 1);
    save();
}

bool PlaylistModel::load()
{
    QFile file(m_storageFile);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) return false;
    m_items.clear();
    for (const auto& value : doc.array()) {
        const QString path = value.toString();
        if (!path.isEmpty() && !m_items.contains(path)) m_items.append(path);
    }
    return true;
}

bool PlaylistModel::save() const
{
    QFile file(m_storageFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    QJsonArray array;
    for (const QString& path : m_items) array.append(path);
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    return file.error() == QFile::NoError;
}
