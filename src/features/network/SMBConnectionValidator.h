#pragma once

#include "SMBProfile.h"

#include <QString>

struct SMBConnectionResult
{
    bool success = false;
    QString message;
};

class SMBConnectionValidator final
{
public:
    static SMBConnectionResult validate(const SMBProfile& profile);
};
