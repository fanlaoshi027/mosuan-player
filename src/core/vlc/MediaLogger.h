#pragma once

#include <QString>

namespace Mosuan {

class MediaLogger
{
public:
    static void info(const QString& message);
    static void error(const QString& message);
};

}
