#pragma once

#include <QString>

struct SMBProfile
{
    QString name;
    QString host;
    QString share;
    QString path;
    QString username;
    QString password;
    int port = 445;
    bool rememberPassword = false;
};
