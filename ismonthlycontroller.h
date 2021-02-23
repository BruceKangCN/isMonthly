#ifndef ISMONTHLYCONTROLLER_H
#define ISMONTHLYCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace isMonthly {

struct IsMonthlyResponse {
    quint64 replyId;
    QString cid;
    bool success;
    bool isMonthly;
    double bitrate;
};

class IsMonthlyController : public QObject
{
    Q_OBJECT
public:
    explicit IsMonthlyController(QObject *parent = nullptr);
    ~IsMonthlyController();

    void setUrl(const QString url);
    void query(const QString cid);
    IsMonthlyResponse getResponse(QNetworkReply* reply);

signals:
    void queryFinished(QNetworkReply* reply);

private:
    quint64 replyId;
    QString isMonthlyUrl;
    QNetworkAccessManager* isMonthlyManager;

};

} // namespace isMonthly
#endif // ISMONTHLYCONTROLLER_H
