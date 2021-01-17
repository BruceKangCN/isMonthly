#ifndef WIDGET_H
#define WIDGET_H

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

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void keyPressEvent(QKeyEvent* ev);
    void generateFile(QString type);

protected:
    void getInfo(); // get query state, is monthly state, bitrate of a video

    /*
     * concat QStrings with ','
     */
    QString getMonthly();
    QString getNotMonthly();
    QString getFailure();

private slots:
    void on_pushButton_clicked();

    void on_btnMonthly_clicked();

    void on_btnNotMonthly_clicked();

    void on_btnFailure_clicked();

    void on_btnApply_clicked();

    void on_copyMonthly_clicked();

    void on_copyNotMonthly_clicked();

    void on_copyFailure_clicked();

private:
    Ui::Widget *ui;

    QSettings* config;
    /*
     * configuration
     */
    QString baseUrl;
    QString logPath;
    QString resultPath;

    /*
     * for resolving network requests
     */
    QNetworkAccessManager *nam;
    QNetworkRequest request;
    QNetworkReply *reply;
    QByteArray data;

    /*
     * to show infomation
     */
    QList<QTreeWidgetItem*> list;

    /*
     * to store ids
     */
    QStringList monthly;
    QStringList notmonthly;
    QStringList failure;
};
#endif // WIDGET_H
