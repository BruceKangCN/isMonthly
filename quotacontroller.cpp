#include "quotacontroller.h"

#include <QRegularExpression>

namespace isMonthly {

QuotaController::QuotaController(QObject *parent)
    : QObject(parent)
    , quotaManager(new QNetworkAccessManager(this))
    , quotaUrl("https://v2.mahuateng.cf/check_quota?serial_code=")
    , serialCode("00000-00000-00000-00000-00000")
{
    connect(quotaManager, &QNetworkAccessManager::finished,
        this, &QuotaController::queryFinished);
}

QuotaController::~QuotaController()
{

}

void QuotaController::setUrl(const QString url)
{
    this->quotaUrl = url;
}

void QuotaController::setSerialCode(const QString serialCode)
{
    this->serialCode = serialCode;
}

void QuotaController::query()
{
    QNetworkRequest request(quotaUrl + serialCode);
    quotaManager->get(request);
}

const QString QuotaController::getQuota(const QString& response) const
{
    return parseQuota(response);
}

const QString QuotaController::parseQuota(const QString& content) const
{
    // this depends on the response format
    QRegularExpression quotaRegex("总计可用\\s*(\\d+\\s*/\\s*\\d+)");
    QRegularExpressionMatch match = quotaRegex.match(content);
    if (match.lastCapturedIndex() > 0) {
        return match.captured(1);
    }
    return "?/?";
}

} // namespace isMonthly
