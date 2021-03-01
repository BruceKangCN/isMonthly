#ifndef QUOTACONTROLLER_H
#define QUOTACONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace isMonthly {

class QuotaController : public QObject
{
    Q_OBJECT
public:
    explicit QuotaController(QObject *parent = nullptr);
    ~QuotaController();

    void setUrl(const QString url);
    void setSerialCode(const QString serialCode);
    void query();
    const QString getQuota(const QString& response) const;

signals:
    void queryFinished(QNetworkReply* reply);

protected:
    const QString parseQuota(const QString& content) const;

private:
    QNetworkAccessManager* quotaManager;
    QString quotaUrl;
    QString serialCode;

};

} // namespace isMonthly
#endif // QUOTACONTROLLER_H
