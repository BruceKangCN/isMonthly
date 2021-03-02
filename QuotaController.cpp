#include "QuotaController.hpp"
#include "LogWrapper.hpp"

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

void QuotaController::setUrl(const QString& url)
{
    logger.info() << "quota url set to" << url;
    this->quotaUrl = url;
}

void QuotaController::setSerialCode(const QString& serialCode)
{
    logger.info() << "serial code set to" << serialCode;
    this->serialCode = serialCode;
}

void QuotaController::query() const
{
    logger.info() << "query for quota";
    QNetworkRequest request(quotaUrl + serialCode);
    quotaManager->get(request);
}

const QString QuotaController::getQuota(const QString& response) const
{
    return parseQuota(response);
}

const QString QuotaController::parseQuota(const QString& content) const
{
    logger.debug() << "parse for content: " << content;

    // this depends on the response format
    QRegularExpression quotaRegex("总计可用\\s*(\\d+\\s*/\\s*\\d+)");
    QRegularExpressionMatch match = quotaRegex.match(content);
    if (match.lastCapturedIndex() > 0) {
        return match.captured(1);
    }

    logger.critical() << "quota infomation parse failed";
    return QString("?/?");
}

} // namespace isMonthly
