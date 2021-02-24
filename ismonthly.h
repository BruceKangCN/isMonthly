#ifndef ISMONTHLY_H
#define ISMONTHLY_H

#include "ismonthlycontroller.h"
#include "quotacontroller.h"

#include <QWidget>
#include <QUrl>
#include <QKeyEvent>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
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
    void setQuota(QNetworkReply* reply); // set quota info

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

    void on_btnQuota_clicked();

private:
    Ui::Widget* ui;

    QSettings* config;
    /*
     * settings
     */
    QNetworkProxy proxy;

    // access data
    IsMonthlyController isMonthlyController;
    QuotaController quotaController;

    // model for data from IsMonthlyController
    QStandardItemModel* isMonthlyModel;

    /*
     * to store ids
     */
    QStringList monthly;
    QStringList notmonthly;
    QStringList failure;
};

} // namespace isMonthly
#endif // ISMONTHLY_H
