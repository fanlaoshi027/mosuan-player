#include "SMBProfileStore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {
QString defaultPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/smb-profiles.json");
}

QJsonObject toJson(const SMBProfile& p)
{
    QJsonObject o;
    o[QStringLiteral("name")] = p.name;
    o[QStringLiteral("host")] = p.host;
    o[QStringLiteral("share")] = p.share;
    o[QStringLiteral("path")] = p.path;
    o[QStringLiteral("username")] = p.username;
    // Password persistence is opt-in. Encryption/OS credential storage will be added
    // before enabling password-at-rest for the production Windows build.
    o[QStringLiteral("rememberPassword")] = p.rememberPassword;
    if (p.rememberPassword) o[QStringLiteral("password")] = p.password;
    o[QStringLiteral("port")] = p.port;
    return o;
}

SMBProfile fromJson(const QJsonObject& o)
{
    SMBProfile p;
    p.name = o.value(QStringLiteral("name")).toString();
    p.host = o.value(QStringLiteral("host")).toString();
    p.share = o.value(QStringLiteral("share")).toString();
    p.path = o.value(QStringLiteral("path")).toString();
    p.username = o.value(QStringLiteral("username")).toString();
    p.rememberPassword = o.value(QStringLiteral("rememberPassword")).toBool(false);
    p.password = p.rememberPassword ? o.value(QStringLiteral("password")).toString() : QString();
    p.port = o.value(QStringLiteral("port")).toInt(445);
    return p;
}
}

SMBProfileStore::SMBProfileStore(QString storagePath)
    : m_storagePath(storagePath.isEmpty() ? defaultPath() : std::move(storagePath))
{
}

QList<SMBProfile> SMBProfileStore::load() const
{
    QFile file(m_storagePath);
    if (!file.open(QIODevice::ReadOnly)) return {};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) return {};

    QList<SMBProfile> result;
    for (const auto& value : doc.array()) {
        if (value.isObject()) result.append(fromJson(value.toObject()));
    }
    return result;
}

bool SMBProfileStore::save(const QList<SMBProfile>& profiles) const
{
    QFile file(m_storagePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    QJsonArray array;
    for (const auto& profile : profiles) array.append(toJson(profile));
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    return true;
}

bool SMBProfileStore::upsert(const SMBProfile& profile) const
{
    auto profiles = load();
    for (auto& current : profiles) {
        if (current.name == profile.name) {
            current = profile;
            return save(profiles);
        }
    }
    profiles.append(profile);
    return save(profiles);
}

bool SMBProfileStore::remove(const QString& name) const
{
    auto profiles = load();
    for (qsizetype i = profiles.size() - 1; i >= 0; --i) {
        if (profiles.at(i).name == name) profiles.removeAt(i);
    }
    return save(profiles);
}
