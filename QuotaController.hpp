#ifndef ISMONTHLY_QUOTACONTROLLER_HPP
#define ISMONTHLY_QUOTACONTROLLER_HPP

#include "LogWrapper.hpp"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace isMonthly {

/*
 * a class to manage the network access for qouta infomation
 * and parse the response from net
 */
class QuotaController : public QObject
{
    Q_OBJECT
public:
    explicit QuotaController(QObject *parent = nullptr);
    ~QuotaController();

    void setUrl(const QString& url); // set query url
    void setSerialCode(const QString& serialCode); // set serial code
    void query() const; // start query
    const QString getQuota(const QString& response) const; // return result

signals:
    void queryFinished(QNetworkReply* reply);

protected:
    const QString parseQuota(const QString& content) const; // parse result

private:
    QNetworkAccessManager* quotaManager; // to manage network access
    QString quotaUrl; // url for query
    QString serialCode; // serial code to query

};

inline void QuotaController::setUrl(const QString& url)
{
    logger.info() << "quota url set to" << url;
    this->quotaUrl = url;
}

inline void QuotaController::setSerialCode(const QString& serialCode)
{
    logger.info() << "serial code set to" << serialCode;
    this->serialCode = serialCode;
}

inline void QuotaController::query() const
{
    logger.info() << "query for quota";
    QNetworkRequest request(quotaUrl + serialCode);
    quotaManager->get(request);
}

inline const QString QuotaController::getQuota(const QString &response) const {
    return parseQuota(response);
}

} // namespace isMonthly
#endif // ISMONTHLY_QUOTACONTROLLER_HPP
