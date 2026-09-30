#include "MediaLogger.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

namespace Mosuan {

static void writeLog(const QString& level, const QString& message)
{
    QFile file("MosuanPlayer.log");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            << " [" << level << "] " << message << "\n";
    }
}

void MediaLogger::info(const QString& message)
{
    writeLog("INFO", message);
}

void MediaLogger::error(const QString& message)
{
    writeLog("ERROR", message);
}

}
