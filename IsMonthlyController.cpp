#include "IsMonthlyController.hpp"
#include "LogWrapper.hpp"

#include <QJsonDocument>
#include <QJsonParseError>

namespace isMonthly {

IsMonthlyController::IsMonthlyController(QObject *parent) noexcept
    : QObject(parent)
    , replyId(0)
    , isMonthlyUrl("https://v2.mahuateng.cf/isMonthly/")
    , isMonthlyManager(new QNetworkAccessManager(this))
{
    // notify that the query is end
    connect(isMonthlyManager, &QNetworkAccessManager::finished,
        this, &IsMonthlyController::queryFinished);
}

IsMonthlyController::~IsMonthlyController() noexcept
{

}

void IsMonthlyController::setUrl(const QString& url)
{
    logger.info() << "query url set to" << url;
    this->isMonthlyUrl = url;
}

void IsMonthlyController::query(const QString& cid) const
{
    logger.info() << "query for" << cid;
    QNetworkRequest request(isMonthlyUrl + cid);
    isMonthlyManager->get(request);
}

const IsMonthlyResponse IsMonthlyController::getResponse(QNetworkReply* reply)
{
    const QByteArray& responseBytes = reply->readAll();
    logger.debug() << "isMonthly response: " << responseBytes;
    IsMonthlyResponse response = IsMonthlyResponse();
    // set the auto-increment id for every response
    response.replyId = this->replyId++;

    response.cid = reply->url().toString().split("/")[4]; // get cid from url

    /*
     * convert the reply content to JSON object and set response values
     */
    QJsonParseError err;
    const QJsonDocument& responseJson = QJsonDocument::fromJson(responseBytes, &err);
    if (err.error != QJsonParseError::NoError) {
      logger.critical() << "JSON parse error: " << err.errorString();
      return response;
    }
    response.bitrate = responseJson["bitrate"].toDouble();
    response.isMonthly = responseJson["monthly"].toBool();
    response.success = responseJson["success"].toBool();
    logger.debug() << "IsMonthlyResponse: " << response.replyId
                   << response.cid << response.success << response.bitrate
                   << response.isMonthly;

    return response;
}

} // namespace isMonthly
