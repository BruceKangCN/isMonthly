#ifndef ISMONTHLY_ISMONTHLYCONTROLLER_HPP
#define ISMONTHLY_ISMONTHLYCONTROLLER_HPP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace isMonthly {

// result struct returned from getResponse
struct IsMonthlyResponse {
    quint64 replyId;
    QString cid;
    bool success;
    bool isMonthly;
    double bitrate;
};

/*
 * a class to manage the network access for isMonthly infomation
 * and parse the response from net
 */
class IsMonthlyController : public QObject
{
    Q_OBJECT
public:
    explicit IsMonthlyController(QObject *parent = nullptr);
    ~IsMonthlyController();

    void setUrl(const QString& url); // set query url
    void query(const QString& cid) const; // start the query
    const IsMonthlyResponse getResponse(QNetworkReply* reply); // return result

signals:
    void queryFinished(QNetworkReply* reply);

private:
    quint64 replyId; // to specific id for every reply
    QString isMonthlyUrl; // url for query
    QNetworkAccessManager* isMonthlyManager; // to manage network access

};

} // namespace isMonthly
#endif // ISMONTHLY_ISMONTHLYCONTROLLER_HPP
