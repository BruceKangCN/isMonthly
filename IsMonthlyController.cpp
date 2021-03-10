#include "IsMonthlyController.hpp"

#include <QJsonDocument>

namespace isMonthly {

IsMonthlyController::IsMonthlyController(QObject *parent)
    : QObject(parent)
    , replyId(0)
    , isMonthlyUrl("https://v2.mahuateng.cf/isMonthly/")
    , isMonthlyManager(new QNetworkAccessManager(this))
{
    // notify that the query is end
    connect(isMonthlyManager, &QNetworkAccessManager::finished,
        this, &IsMonthlyController::queryFinished);
}

IsMonthlyController::~IsMonthlyController()
{

}

const IsMonthlyResponse IsMonthlyController::getResponse(QNetworkReply* reply)
{
    logger.debug() << "isMonthly response: " << reply->readAll();
    IsMonthlyResponse response = IsMonthlyResponse();
    // set the auto-increment id for every response
    response.replyId = this->replyId++;

    response.cid = reply->url().toString().split("/")[4]; // get cid from url

    /*
     * convert the reply content to JSON object and set response values
     */
    QJsonDocument responseJson = QJsonDocument::fromJson(reply->readAll());
    response.bitrate = responseJson["bitrate"].toDouble();
    response.isMonthly = responseJson["monthly"].toBool();
    response.success = responseJson["success"].toBool();

    return response;
}

} // namespace isMonthly
