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
#include <QRegularExpression>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , baseUrl("https://v2.mahuateng.cf/isMonthly/")
    , quotaUrl("https://v2.mahuateng.cf/check_quota?serial_code=")
    , serialCode("00000-00000-00000-00000-00000")
    , logPath("isMonthly.log")
    , monthly(QStringList())
    , notmonthly(QStringList())
    , failure(QStringList())
    , config(new QSettings("app.ini", QSettings::IniFormat))
    , nam(new QNetworkAccessManager(this))
    , quotaManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    this->setWindowTitle("is monthly");

    /*
     * check SSL compatibility, only for debug purpose
     */
    qDebug() << QSslSocket::sslLibraryBuildVersionNumber();
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionNumber();
    qDebug() << QSslSocket::sslLibraryVersionString();
    qDebug() << QSslSocket::supportsSsl();

    /*
     * get settings from ini file
     */
    if (config->contains("url")) {
        baseUrl = config->value("url").toString();
        ui->inpUrl->setText(baseUrl);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find url: " << baseUrl;
    }
    if (config->contains("quotaUrl")) {
        quotaUrl = config->value("quotaUrl").toString();
        ui->inpQuotaUrl->setText(quotaUrl);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find quotaUrl: " << quotaUrl;
    }
    if (config->contains("serialCode")) {
        serialCode = config->value("serialCode").toString();
        ui->inpSN->setText(serialCode);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find serialCode: " << serialCode;
    }
    if (config->contains("log")) {
        logPath = config->value("log").toString();
        ui->inpLog->setText(logPath);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find log: " << logPath;
    }
    if (config->contains("proxy")) {
        QString v = config->value("proxy").toString();
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "find proxy: " << v;
        if (v == "no") {
            ui->radioNoProxy->setChecked(true);
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        } else if (parseProxy(v)) {
            ui->radioCustom->setChecked(true);
            ui->inpProxy->setText(v);
            QNetworkProxy::setApplicationProxy(proxy);
        } else {
            ui->radioSystemProxy->setChecked(true);
            for (QNetworkProxy& p : QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(baseUrl))) {
                if (p.type() == QNetworkProxy::HttpProxy || p.type() == QNetworkProxy::Socks5Proxy) {
                    QNetworkProxy::setApplicationProxy(p);
                    break;
                }
            }
        }
    }

    /*
     * init connections
     */
    connect(nam, &QNetworkAccessManager::finished, this, &Widget::getInfo);
    connect(quotaManager, &QNetworkAccessManager::finished, this, &Widget::setQuota);

    getQuota(); // init quota info
}

Widget::~Widget()
{
    delete nam;
    delete quotaManager;
    delete config;

    delete ui;
}

/*
 * send a request to query infomation
 * ids are splited by ',' in lineEdit widget
 */
void Widget::on_pushButton_clicked()
{
    QStringList ids = ui->lineEdit->text().split(QRegularExpression("\\s*,\\s*"));

    for (QString& id : ids) {
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "id: " << id;
        request.setUrl(baseUrl + id);
        nam->get(request);
        qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
                 << "request send.";
    }
}

void Widget::getInfo(QNetworkReply* reply)
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
    QString cid = reply->url().toString().split("/")[4];
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "cid: " << cid;
    log.write(QString("cid: %1\n").arg(cid).toUtf8());
    QByteArray data = reply->readAll(); // get response data
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "data: " << QString::fromUtf8(data);
    QJsonDocument doc = QJsonDocument::fromJson(data); // convert response text to JSON object

    /*
     * set member variables
     */
    QJsonObject root = doc.object();
    const double bitrate = root["bitrate"].toDouble();
    const bool isMonthly = root["monthly"].toBool();
    const bool success = root["success"].toBool();
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
            new QTreeWidgetItem(ui->treeWidget, QStringList({cid, tr("月额视频：是"), QString("%1k").arg(bitrate)}));
        } else {
            notmonthly.append(cid);
            ui->txtNotMonthly->setText(notmonthly.join(','));
            new QTreeWidgetItem(ui->treeWidget, QStringList({cid, tr("月额视频：否"), QString("%1k").arg(bitrate)}));
        }
    } else {
        failure.append(cid);
        ui->txtFailure->setText(failure.join(','));
        new QTreeWidgetItem(ui->treeWidget, QStringList({cid, tr("查询失败")}));
    }

    log.close();
}

void Widget::keyPressEvent(QKeyEvent* ev)
{
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

                    if (type == tr("月额视频：是")) {
                        monthly.removeAt(monthly.indexOf(content));
                        ui->txtMonthly->setText(monthly.join(','));
                    }
                    if (type == tr("月额视频：否")) {
                        notmonthly.removeAt(notmonthly.indexOf(content));
                        ui->txtNotMonthly->setText(notmonthly.join(','));
                    }
                    if (type == tr("查询失败")) {
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

QString Widget::getMonthly() const
{
    return monthly.join(',');
}

QString Widget::getNotMonthly() const
{
    return notmonthly.join(',');
}

QString Widget::getFailure() const
{
    return failure.join(',');
}

/*
 * apply settings
 * also overwrite app.ini
 */
void Widget::on_btnApply_clicked()
{
    baseUrl = ui->inpUrl->text();
    quotaUrl = ui->inpQuotaUrl->text();
    serialCode = ui->inpSN->text();
    logPath = ui->inpLog->text();

    config->setValue("url", baseUrl);
    config->setValue("quotaUrs", quotaUrl);
    config->setValue("serialCode", serialCode);
    config->setValue("log", logPath);

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
            ui->inpProxy->setText(proxyUrl + "[" + tr("解析出错") + "]");
            config->setValue("proxy", "system");
        }
    }

    config->sync();
}

void Widget::on_copyMonthly_clicked() const
{
    QGuiApplication::clipboard()->setText(ui->txtMonthly->toPlainText());
}

void Widget::on_copyNotMonthly_clicked() const
{
    QGuiApplication::clipboard()->setText(ui->txtNotMonthly->toPlainText());
}

void Widget::on_copyFailure_clicked() const
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

void Widget::getQuota()
{
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "query for quota...";
    quotaReply = quotaManager->get(QNetworkRequest(quotaUrl + serialCode));
    qDebug() << QDateTime::currentDateTime().toString("[hh:mm:ss:zzz]") << "[debug]" << __FILE__ << __LINE__ << Q_FUNC_INFO
             << "waiting for quota reply...";
}

void Widget::on_btnQuota_clicked()
{
    getQuota();
}

void Widget::setQuota()
{
    QStringList lines = QString::fromUtf8(quotaReply->readAll()).split('\n');
    if (lines.count() > 3) { // check wheather return successfully
        ui->inpQuota->setText(lines[lines.count() - 3]);
    }
}
