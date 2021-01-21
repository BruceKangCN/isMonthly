#include "widget.h"
#include "ui_widget.h"

#include <QtDebug>
#include <QDateTime>
#include <QFile>

#include <QClipboard>
#include <QDir>
#include <QFileDialog>

#include <QList>
#include <QNetworkProxy>
#include <QUrl>
#include <QNetworkProxyQuery>
#include <QNetworkProxyFactory>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , reply(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("is monthly");

    /*
     * check SSL infomation, only for debug use
     */
    qDebug() << QSslSocket::sslLibraryBuildVersionNumber();
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionNumber();
    qDebug() << QSslSocket::sslLibraryVersionString();
    qDebug() << QSslSocket::supportsSsl();

    /*
     * set default value
     */
    baseUrl = "https://v2.mahuateng.cf/isMonthly/";
    logPath = "isMonthly.log";
    resultPath = "result/";
    /*
     * get settings from ini file
     */
    config = new QSettings("app.ini", QSettings::IniFormat);
    if (config->contains("url")) {
        baseUrl = config->value("url").toString();
        ui->inpUrl->setText(baseUrl);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find url: " << baseUrl;
    }
    if (config->contains("log")) {
        logPath = config->value("log").toString();
        ui->inpLog->setText(logPath);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find log: " << logPath;
    }
    if (config->contains("result")) {
        resultPath = config->value("result").toString();
        ui->inpResult->setText(resultPath);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find result: " << resultPath;
    }
    if (config->contains("proxy")) {
        QString v = config->value("proxy").toString();
        if (v == "no") {
            ui->radioNoProxy->setChecked(true);
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        } else if (parseProxy(v)) {
            ui->radioCustom->setChecked(true);
            QNetworkProxy::setApplicationProxy(proxy);
        } else {
            ui->radioSystemProxy->setChecked(true);
            for (QNetworkProxy p : QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(baseUrl))) {
                if (p.type() == QNetworkProxy::HttpProxy || p.type() == QNetworkProxy::Socks5Proxy) {
                    QNetworkProxy::setApplicationProxy(p);
                    break;
                }
            }
        }
    }

    /*
     * init QStringLists
     */
    monthly = QStringList();
    notmonthly = QStringList();
    failure = QStringList();

    /*
     * init QNetworkAccessManager
     */
    nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished, this, &Widget::getInfo);
}

Widget::~Widget()
{
    delete nam;
    delete config;

    generateFile("monthly");

    delete ui;
}

/*
 * send a request to query infomation
 * ids are splited by ',' in lineEdit widget
 */
void Widget::on_pushButton_clicked()
{
    QStringList urls = ui->lineEdit->text().split(',');
    int i = 0;

    while (i < urls.count()) {
        while (reply != nullptr && reply->isRunning()) { // wait for respoense
            QApplication::processEvents();
        } // must be first
        QString url = baseUrl + urls[i++].simplified(); // trim id string
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
    /*
     * write log
     */
    QFile log(logPath);
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
    data = reply->readAll(); // get response data
    QJsonDocument doc = QJsonDocument::fromJson(data); // convert response text to JSON object

    /*
     * set member variables
     */
    QJsonObject root = doc.object();
    double bitrate = root["bitrate"].toDouble();
    bool isMonthly = root["monthly"].toBool();
    bool success = root["success"].toBool();
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "is monthly: " << isMonthly;
    log.write(QString("isMonthly: %1\n").arg(isMonthly).toUtf8());

    /*
     * append different types of QTreeWidget depends on query result
     */
    if (success) {
        if (isMonthly) {
            monthly.append(cid);
            ui->txtMonthly->setText(monthly.join(','));
            new QTreeWidgetItem(ui->treeWidget, QStringList({QString("%1").arg(cid), "月额视频：是", QString("%1k").arg(bitrate)}));
        } else {
            notmonthly.append(cid);
            ui->txtNotMonthly->setText(notmonthly.join(','));
            new QTreeWidgetItem(ui->treeWidget, QStringList({QString("%1").arg(cid), "月额视频：否", QString("%1k").arg(bitrate)}));
        }
    } else {
        failure.append(cid);
        ui->txtFailure->setText(failure.join(','));
        new QTreeWidgetItem(ui->treeWidget, QStringList({QString("%1").arg(cid), "查询失败"}));
    }

    log.close();
}

void Widget::keyPressEvent(QKeyEvent* ev)
{
    /*
     * only for debug use
     * print debug info when F8 is pressed
     */
    if (ev->key() == Qt::Key_F8)
    {
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "is reply finished: " << reply->isFinished();
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "status code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    }
    /*
     * execute query when enter or return is pressed
     */
    if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
    {
        on_pushButton_clicked();
    }
    /*
     * delete selected widgets when delete is pressed
     * also change associated QStringList
     */
    if (ev->key() == Qt::Key_Delete)
    {
        qDebug() << "delete!";
        QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
        QTreeWidgetItem* p = nullptr;

        if (!items.isEmpty()) {
            for (QTreeWidgetItem* item : items) {
                p = item->parent();
                if (p == nullptr) {
                    qDebug() << "remove no-parent item";

                    QString content = item->data(0, Qt::DisplayRole).toString();
                    QString type = item->data(1, Qt::DisplayRole).toString();

                    if (type == "月额视频：是") {
                        monthly.removeAt(monthly.indexOf(content));
                        ui->txtMonthly->setText(monthly.join(','));
                    }
                    if (type == "月额视频：否") {
                        notmonthly.removeAt(notmonthly.indexOf(content));
                        ui->txtNotMonthly->setText(notmonthly.join(','));
                    }
                    if (type == "查询失败") {
                        failure.removeAt(failure.indexOf(content));
                        ui->txtFailure->setText(failure.join(','));
                    }

                    delete item;
                } else {
                    qDebug() << "remove child item";
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

/*
 * generate file to store query result
 */
void Widget::generateFile(QString type)
{
    QFile f = QFile(QDir(resultPath).filePath(type + ".txt")); // safely join the directory and filename
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

/*
 * apply settings
 * also overwrite app.ini
 */
void Widget::on_btnApply_clicked()
{
    baseUrl = ui->inpUrl->text();
    logPath = ui->inpLog->text();
    resultPath = ui->inpResult->text();

    config->setValue("url", baseUrl);
    config->setValue("log", logPath);
    config->setValue("result", resultPath);

    /*
     * proxy settings
     */
    if (ui->radioNoProxy->isChecked()) {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        config->setValue("proxy", "no");
    }
    if (ui->radioSystemProxy->isChecked()) {
        for (QNetworkProxy p : QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(baseUrl))) {
            if (p.type() == QNetworkProxy::HttpProxy || p.type() == QNetworkProxy::Socks5Proxy) {
                QNetworkProxy::setApplicationProxy(p);
                break;
            }
        }
        config->setValue("proxy", "system");
    }
    if (ui->radioCustom->isChecked()) {
        QString proxyUrl = ui->inpProxy->text();
        if (parseProxy(proxyUrl)) {
            QNetworkProxy::setApplicationProxy(proxy);
            config->setValue("proxy", proxyUrl);
        } else {
            for (QNetworkProxy p : QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(baseUrl))) {
                if (p.type() == QNetworkProxy::HttpProxy || p.type() == QNetworkProxy::Socks5Proxy) {
                    QNetworkProxy::setApplicationProxy(p);
                    break;
                }
            }
            ui->radioCustom->setChecked(false);
            ui->radioSystemProxy->setChecked(true);
            ui->inpProxy->setText(proxyUrl + "[解析出错]");
            config->setValue("proxy", "system");
        }
    }

    config->sync();
}

void Widget::on_copyMonthly_clicked()
{
    QGuiApplication::clipboard()->setText(ui->txtMonthly->toPlainText());
}

void Widget::on_copyNotMonthly_clicked()
{
    QGuiApplication::clipboard()->setText(ui->txtNotMonthly->toPlainText());
}

void Widget::on_copyFailure_clicked()
{
    QGuiApplication::clipboard()->setText(ui->txtFailure->toPlainText());
}

void Widget::on_btnLogPath_clicked()
{
    QString path = QFileDialog::getSaveFileName(nullptr, QString(), ".");
    if (path != "") {
        logPath = path;
        ui->inpLog->setText(logPath);
    }
}

void Widget::on_btnResultDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(nullptr, QString(), ".");
    if (path != "") {
        resultPath = path;
        ui->inpResult->setText(resultPath);
    }
}

bool Widget::parseProxy(QString url) {
    QUrl proxyUrl(url);
    if (!proxyUrl.isValid()) {
        return false;
    }

    // validate scheme
    if (proxyUrl.scheme() == "socks5") {
        proxy.setType(QNetworkProxy::Socks5Proxy);
    } else if (proxyUrl.scheme() == "http") {
        proxy.setType(QNetworkProxy::HttpProxy);
    } else {
        return false;
    }

    // validate host
    QString host = proxyUrl.host();
    qDebug() << "host: " << host;
    if (host == "") {
        return false;
    }
    proxy.setHostName(host);

    // validate port
    int port = proxyUrl.port();
    qDebug() << "port: " << port;
    if (port == -1) {
        return false;
    }
    proxy.setPort(port);

    QString userName = proxyUrl.userName();
    if (userName != "") {
        proxy.setUser(userName);
    }
    QString password = proxyUrl.password();
    if (password != "") {
        proxyUrl.setPassword(password);
    }

    return true;
}
