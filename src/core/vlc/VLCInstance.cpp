#include "VLCInstance.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QVector>

VLCInstance::VLCInstance() = default;

VLCInstance::~VLCInstance()
{
    if (m_instance) {
        libvlc_release(m_instance);
    }
}

bool VLCInstance::initialize()
{
    if (m_instance) {
        return true;
    }

    // The Windows package ships VLC plugins beside the executable.
    // Explicitly provide the plugin path so playback does not depend on
    // the user's VLC installation or the current working directory.
    const QString applicationDir = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();
    const QString pluginDir = QDir(applicationDir).filePath(QStringLiteral("plugins"));
    const QByteArray pluginPathArg = QStringLiteral("--plugin-path=%1").arg(pluginDir).toUtf8();

    const char* args[] = {
        "--no-video-title-show",
        pluginPathArg.constData()
    };

    m_instance = libvlc_new(2, args);
    return m_instance != nullptr;
}
