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
#include <QNetworkProxy>

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
    void generateFile(const QString& type) const;

protected:
    void getInfo(); // get query state, is monthly state, bitrate of a video

    /*
     * concat QStrings with ','
     */
    QString getMonthly() const;
    QString getNotMonthly() const;
    QString getFailure() const;
    bool parseProxy(QString url);

private slots:
    void on_pushButton_clicked(); // start query

    /*
     * generate specific file
     */
    void on_btnMonthly_clicked() const;
    void on_btnNotMonthly_clicked() const;
    void on_btnFailure_clicked() const;

    void on_btnApply_clicked(); // apply settings

    /*
     * copy ids to clipboard
     */
    void on_copyMonthly_clicked() const;
    void on_copyNotMonthly_clicked() const;
    void on_copyFailure_clicked() const;

    /*
     * select path/dir
     */
    void on_btnLogPath_clicked();
    void on_btnResultDir_clicked();

private:
    Ui::Widget *ui;

    QSettings* config;
    /*
     * configuration
     */
    QString baseUrl;
    QString logPath;
    QString resultPath;
    QNetworkProxy proxy;

    /*
     * for resolving network requests
     */
    QNetworkAccessManager *nam;
    QNetworkRequest request;
    QNetworkReply *reply;
    QByteArray data;

    /*
     * to store ids
     */
    QStringList monthly;
    QStringList notmonthly;
    QStringList failure;
};
#endif // WIDGET_H
