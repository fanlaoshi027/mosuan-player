#include "VideoProfileStore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

VideoProfileStore::VideoProfileStore(QString storageFile)
    : m_storageFile(std::move(storageFile))
{
    if (m_storageFile.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_storageFile = QDir(dir).filePath(QStringLiteral("video-profiles.json"));
    }
    load();
}

VideoProfile& VideoProfileStore::profileFor(const QString& mediaPath)
{
    auto it = m_profiles.find(mediaPath);
    if (it == m_profiles.end()) {
        VideoProfile profile;
        profile.mediaPath = mediaPath;
        it = m_profiles.insert(mediaPath, profile);
    }
    return it.value();
}

const VideoProfile* VideoProfileStore::find(const QString& mediaPath) const
{
    const auto it = m_profiles.constFind(mediaPath);
    return it == m_profiles.cend() ? nullptr : &it.value();
}

void VideoProfileStore::clear(const QString& mediaPath)
{
    m_profiles.remove(mediaPath);
    save();
}

bool VideoProfileStore::load()
{
    QFile file(m_storageFile);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    m_profiles.clear();
    const QJsonArray profiles = doc.object().value(QStringLiteral("profiles")).toArray();
    for (const QJsonValue& value : profiles) {
        const QJsonObject obj = value.toObject();
        const QString path = obj.value(QStringLiteral("mediaPath")).toString();
        if (path.isEmpty()) continue;

        VideoProfile profile;
        profile.mediaPath = path;
        const QJsonObject crop = obj.value(QStringLiteral("cropRect")).toObject();
        profile.cropRect = QRectF(
            crop.value(QStringLiteral("x")).toDouble(0.0),
            crop.value(QStringLiteral("y")).toDouble(0.0),
            crop.value(QStringLiteral("w")).toDouble(1.0),
            crop.value(QStringLiteral("h")).toDouble(1.0));
        profile.cropEnabled = obj.value(QStringLiteral("cropEnabled")).toBool(false);
        profile.smartInvert = obj.value(QStringLiteral("smartInvert")).toBool(false);
        const QJsonObject protectedRect = obj.value(QStringLiteral("protectedRect")).toObject();
        profile.protectedRect = QRectF(
            protectedRect.value(QStringLiteral("x")).toDouble(0.0),
            protectedRect.value(QStringLiteral("y")).toDouble(0.0),
            protectedRect.value(QStringLiteral("w")).toDouble(0.0),
            protectedRect.value(QStringLiteral("h")).toDouble(0.0));
        profile.protectedBrightness = obj.value(QStringLiteral("protectedBrightness")).toInt(100);
        m_profiles.insert(path, profile);
    }
    return true;
}

bool VideoProfileStore::save() const
{
    QJsonArray profiles;
    for (auto it = m_profiles.cbegin(); it != m_profiles.cend(); ++it) {
        const VideoProfile& profile = it.value();
        const QRectF r = profile.cropRect;
        const QRectF p = profile.protectedRect;
        QJsonObject crop{
            {QStringLiteral("x"), r.x()}, {QStringLiteral("y"), r.y()},
            {QStringLiteral("w"), r.width()}, {QStringLiteral("h"), r.height()}
        };
        QJsonObject protectedRect{
            {QStringLiteral("x"), p.x()}, {QStringLiteral("y"), p.y()},
            {QStringLiteral("w"), p.width()}, {QStringLiteral("h"), p.height()}
        };
        profiles.append(QJsonObject{
            {QStringLiteral("mediaPath"), profile.mediaPath},
            {QStringLiteral("cropRect"), crop},
            {QStringLiteral("cropEnabled"), profile.cropEnabled},
            {QStringLiteral("smartInvert"), profile.smartInvert},
            {QStringLiteral("protectedRect"), protectedRect},
            {QStringLiteral("protectedBrightness"), profile.protectedBrightness}
        });
    }

    QFile file(m_storageFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    file.write(QJsonDocument(QJsonObject{{QStringLiteral("profiles"), profiles}}).toJson(QJsonDocument::Indented));
    return file.error() == QFile::NoError;
}
