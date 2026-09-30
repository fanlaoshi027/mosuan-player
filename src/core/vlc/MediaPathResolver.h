#pragma once

#include <QString>

class MediaPathResolver
{
public:
    static QString toVlcLocation(const QString& filePath);
};
