#ifndef ISMONTHLY_ISMONTHLY_HPP
#define ISMONTHLY_ISMONTHLY_HPP

#include "IsMonthlyController.hpp"
#include "QuotaController.hpp"

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

/*
 * a class to interactive with the ui
 * and handle the settings
 */
class IsMonthly : public QWidget
{
    Q_OBJECT

public:
    IsMonthly(QWidget *parent = nullptr);
    ~IsMonthly();

    void keyPressEvent(QKeyEvent* ev);
    void generateFile(const QString& type) const;

protected:
    void appendResult(QNetworkReply* reply); // append a parsed result to the model
    void setQuota(QNetworkReply* reply); // set quota info
    bool parseProxy(QString url); // parse string to proxy

private slots:
    void on_pushButton_clicked();
    void on_btnApply_clicked();
    void on_btnQuota_clicked();

private:
    Ui::Widget* ui;

    QSettings* config; // app configuration
    QNetworkProxy proxy; // to store result from parseProxy

    // to access data
    IsMonthlyController isMonthlyController;
    QuotaController quotaController;

    // model for data from IsMonthlyController
    QStandardItemModel* isMonthlyModel;
};

} // namespace isMonthly
#endif // ISMONTHLY_ISMONTHLY_HPP
