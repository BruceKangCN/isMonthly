#ifndef ISMONTHLY_QUOTACONTROLLER_HPP
#define ISMONTHLY_QUOTACONTROLLER_HPP

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

inline const QString QuotaController::getQuota(const QString &response) const {
    return parseQuota(response);
}

} // namespace isMonthly
#endif // ISMONTHLY_QUOTACONTROLLER_HPP
