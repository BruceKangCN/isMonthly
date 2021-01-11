#include "widget.h"
#include "ui_widget.h"

#include <QtDebug>
#include <QDateTime>
#include <QFile>

#include <QList>
#include <QNetworkProxy>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , reply(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("is monthly");

    qDebug() << QSslSocket::sslLibraryBuildVersionNumber();
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionNumber();
    qDebug() << QSslSocket::sslLibraryVersionString();
    qDebug() << QSslSocket::supportsSsl();

    monthly = QStringList();
    notmonthly = QStringList();
    failure = QStringList();

    nam = new QNetworkAccessManager(this);

    nam->setProxy(QNetworkProxy::DefaultProxy);

    connect(nam, &QNetworkAccessManager::finished, this, &Widget::getInfo);
}

Widget::~Widget()
{
    delete nam;

    generateFile("monthly");

    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QStringList urls = ui->lineEdit->text().split(',');
    int i = 0;

    while (i < urls.count()) {
        while (reply != nullptr && reply->isRunning()) {
            QApplication::processEvents();
        }
        url = "https://v2.mahuateng.cf/isMonthly/" + urls[i++].simplified();
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "url: " << url;
        request.setUrl(url);
        reply = nam->get(request);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "request send.";

    }
}

void Widget::getInfo()
{
    QFile log("isMonthly.log");
    log.open(QIODevice::WriteOnly | QIODevice::Append);
    log.write(QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]%1\n").arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()).toUtf8());
    log.write(QString("ssl version: %1\n").arg(QSslSocket::sslLibraryVersionString()).toUtf8());
    log.write(QString("ssl support: %1\n").arg(QSslSocket::supportsSsl()).toUtf8());

    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
            << "get data.";
    log.write(QString("get data.\n").toUtf8());
    QString cid = reply->url().toString().split("/")[4];
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "cid: " << cid;
    log.write(QString("cid: %1\n").arg(cid).toUtf8());
    data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    QJsonObject root = doc.object();
    double bitrate = root["bitrate"].toDouble();
    bool isMonthly = root["monthly"].toBool();
    bool success = root["success"].toBool();
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "is monthly: " << isMonthly;
    log.write(QString("isMonthly: %1\n").arg(isMonthly).toUtf8());

    if (success) {
        if (isMonthly) {
            monthly.append(cid);
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget, QStringList(QString("[%1]月额视频：是，%2k").arg(cid).arg(bitrate)));
            list.append(item);
            item->addChild(new QTreeWidgetItem(item, QStringList(QString("\"bitrate\": %1").arg(bitrate))));
            item->addChild(new QTreeWidgetItem(item, QStringList(QString("\"monthly\": %1").arg(isMonthly?"true":"false"))));
            item->addChild(new QTreeWidgetItem(item, QStringList(QString("\"success\": %1").arg(success?"true":"false"))));
        } else {
            notmonthly.append(cid);
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget, QStringList(QString("[%1]月额视频：否,%2k").arg(cid).arg(bitrate)));
            list.append(item);
            item->addChild(new QTreeWidgetItem(item, QStringList(QString("\"bitrate\": %1").arg(bitrate))));
            item->addChild(new QTreeWidgetItem(item, QStringList(QString("\"monthly\": %1").arg(isMonthly?"true":"false"))));
            item->addChild(new QTreeWidgetItem(item, QStringList(QString("\"success\": %1").arg(success?"true":"false"))));
        }
    } else {
        failure.append(cid);
        new QTreeWidgetItem(ui->treeWidget, QStringList(QString("[%1]查询失败").arg(cid)));
    }

    log.close();
}

void Widget::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_F8)
    {
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "is reply finished: " << reply->isFinished();
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "status code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    }
    if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
    {
        on_pushButton_clicked();
    }
    if (ev->key() == Qt::Key_Delete)
    {
        qDebug() << "delete!";
        QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
        QTreeWidgetItem* p = nullptr;

        if (!items.isEmpty()) {
            for (QTreeWidgetItem* item : items) {
                p = item->parent();
                if (p == nullptr) {
                    list.removeOne(item);
                    delete item;
                } else {
                    p->removeChild(item);
                    delete item;
                }
            }
        }
    }
}

QString Widget::getMonthly()
{
    return monthly.join(',');
}

QString Widget::getNotMonthly()
{
    return notmonthly.join(',');
}

QString Widget::getFailure()
{
    return failure.join(',');
}

void Widget::generateFile(QString type)
{
    QFile f = QFile("result/" + type + ".txt");
    f.open(QIODevice::WriteOnly);
    QString content;

    if (type == "monthly") {
        content = getMonthly();
    }
    if (type == "notmonthly") {
        content = getNotMonthly();
    }
    if (type == "failure") {
        content = getFailure();
    }

    f.write(content.toUtf8());
    f.close();
}

void Widget::on_btnMonthly_clicked()
{
    generateFile("monthly");
}

void Widget::on_btnNotMonthly_clicked()
{
    generateFile("notmonthly");
}

void Widget::on_btnFailure_clicked()
{
    generateFile("failure");
}
