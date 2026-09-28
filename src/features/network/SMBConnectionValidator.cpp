#include "SMBConnectionValidator.h"

#include <QHostAddress>

SMBConnectionResult SMBConnectionValidator::validate(const SMBProfile& profile)
{
    SMBConnectionResult result;

    if (profile.host.trimmed().isEmpty()) {
        result.message = QStringLiteral("服务器地址不能为空");
        return result;
    }
    if (profile.share.trimmed().isEmpty()) {
        result.message = QStringLiteral("共享名称不能为空");
        return result;
    }
    if (profile.port <= 0 || profile.port > 65535) {
        result.message = QStringLiteral("SMB端口无效");
        return result;
    }

    result.success = true;
    result.message = QStringLiteral("SMB配置有效");
    return result;
}
