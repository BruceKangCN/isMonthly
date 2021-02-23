#ifndef ISMONTHLY_H
#define ISMONTHLY_H

#include "ismonthlycontroller.h"

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEvent>
#include <QKeyEvent>
#include <QTreeWidget>
#include <QSettings>
#include <QNetworkProxy>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

namespace isMonthly {

class IsMonthly : public QWidget
{
    Q_OBJECT

public:
    IsMonthly(QWidget *parent = nullptr);
    ~IsMonthly();

    void keyPressEvent(QKeyEvent* ev);
    void generateFile(const QString& type) const;

protected:
    void appendResult(QNetworkReply* reply); // get query state, is monthly state, bitrate of a video
    void setQuota(); // set quota info

    /*
     * concat QStrings with ','
     */
    QString getMonthly() const;
    QString getNotMonthly() const;
    QString getFailure() const;
    bool parseProxy(QString url);

private slots:
    void on_pushButton_clicked(); // start query
    void on_btnApply_clicked(); // apply settings

    /*
     * copy ids to clipboard
     */
    void on_copyMonthly_clicked() const;
    void on_copyNotMonthly_clicked() const;
    void on_copyFailure_clicked() const;

    void getQuota(); // get quota info
    void on_btnQuota_clicked();

private:
    Ui::Widget* ui;

    QSettings* config;
    /*
     * configuration
     */
    QString isMonthlyUrl;
    QString quotaUrl;
    QString serialCode;
    QNetworkProxy proxy;

    /*
     * for resolving network requests
     */
    QNetworkAccessManager* quotaManager;
    QNetworkReply *quotaReply;

    // access data
    IsMonthlyController isMonthlyController;

    /*
     * to store ids
     */
    QStringList monthly;
    QStringList notmonthly;
    QStringList failure;
};

} // namespace isMonthly
#endif // ISMONTHLY_H
