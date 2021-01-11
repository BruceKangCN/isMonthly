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
    void getInfo();
    QString getMonthly();
    QString getNotMonthly();
    QString getFailure();

private slots:
    void on_pushButton_clicked();

    void on_btnMonthly_clicked();

    void on_btnNotMonthly_clicked();

    void on_btnFailure_clicked();

private:
    Ui::Widget *ui;

    QString id;
    QString url;
    QNetworkAccessManager *nam;
    QNetworkRequest request;
    QNetworkReply *reply;
    QByteArray data;
    QList<QTreeWidgetItem*> list;
    QStringList monthly;
    QStringList notmonthly;
    QStringList failure;
};
#endif // WIDGET_H
