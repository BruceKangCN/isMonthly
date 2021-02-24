#include "ismonthly.h"
#include "ui_widget.h"

#include <QDebug>
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QNetworkProxy>
#include <QNetworkProxyQuery>
#include <QNetworkProxyFactory>
#include <QRegularExpression>

namespace isMonthly {

IsMonthly::IsMonthly(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , config(new QSettings("app.ini", QSettings::IniFormat))
{
    ui->setupUi(this);
    this->setWindowTitle("is monthly");

    /*
     * check SSL compatibility, only for debug purpose
     */
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << QSslSocket::sslLibraryVersionString();
    qDebug() << QSslSocket::supportsSsl();

    /*
     * get settings from ini file
     */
    if (config->contains("url")) {
        const QString isMonthlyUrl = config->value("url").toString();
        isMonthlyController.setUrl(isMonthlyUrl);
        ui->inpUrl->setText(isMonthlyUrl);
    }
    if (config->contains("quotaUrl")) {
        const QString quotaUrl = config->value("quotaUrl").toString();
        quotaController.setUrl(quotaUrl);
        ui->inpQuotaUrl->setText(quotaUrl);
    }
    if (config->contains("serialCode")) {
        const QString serialCode = config->value("serialCode").toString();
        quotaController.setSerialCode(serialCode);
        ui->inpSN->setText(serialCode);
    }
    if (config->contains("proxy")) {
        QString v = config->value("proxy").toString();
        if (v == "no") {
            ui->radioNoProxy->setChecked(true);
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        } else if (parseProxy(v)) {
            ui->radioCustom->setChecked(true);
            ui->inpProxy->setText(v);
            QNetworkProxy::setApplicationProxy(proxy);
        } else {
            ui->radioSystemProxy->setChecked(true);
            for (QNetworkProxy& p
                : QNetworkProxyFactory::systemProxyForQuery(
                    QNetworkProxyQuery(config->value("url").toString())
                    )
                ) {
                if (p.type() == QNetworkProxy::HttpProxy
                ||  p.type() == QNetworkProxy::Socks5Proxy) {
                    QNetworkProxy::setApplicationProxy(p);
                    break;
                }
            }
        }
    }
    if (config->contains("language")) {
        const int language = config->value("language").toInt();
        ui->boxLanguage->setCurrentIndex(language);
    }

    /*
     * init connections
     */
    connect(&isMonthlyController, &IsMonthlyController::queryFinished, this, &IsMonthly::appendResult);
    connect(&quotaController, &QuotaController::queryFinished, this, &IsMonthly::setQuota);
}

IsMonthly::~IsMonthly()
{
    delete config;

    delete ui;
}

/*
 * send a request to query infomation
 * ids are splited by ',' in lineEdit widget
 */
void IsMonthly::on_pushButton_clicked()
{
    QStringList ids = ui->lineEdit->text().split(QRegularExpression("\\s*,\\s*"));
    for (QString& id : ids) {
        isMonthlyController.query(id);
    }
}

void IsMonthly::appendResult(QNetworkReply* reply)
{
    IsMonthlyResponse response = isMonthlyController.getResponse(reply);

    /*
     * append different types of QTreeWidget depends on query result
     */
    if (response.success) {
        if (response.isMonthly) {
            monthly.append(response.cid);
            ui->txtMonthly->setText(monthly.join(','));
            new QTreeWidgetItem(ui->treeWidget, QStringList({response.cid, tr("月额视频：是"), QString("%1k").arg(response.bitrate)}));
        } else {
            notmonthly.append(response.cid);
            ui->txtNotMonthly->setText(notmonthly.join(','));
            new QTreeWidgetItem(ui->treeWidget, QStringList({response.cid, tr("月额视频：否"), QString("%1k").arg(response.bitrate)}));
        }
    } else {
        failure.append(response.cid);
        ui->txtFailure->setText(failure.join(','));
        new QTreeWidgetItem(ui->treeWidget, QStringList({response.cid, tr("查询失败")}));
    }
}

void IsMonthly::keyPressEvent(QKeyEvent* ev)
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
        QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
        QTreeWidgetItem* p = nullptr;

        if (!items.isEmpty()) {
            for (QTreeWidgetItem* item : items) {
                p = item->parent();
                if (p == nullptr) {
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
                    p->removeChild(item);
                    delete item;
                }
            }
        }
    }
}

QString IsMonthly::getMonthly() const
{
    return monthly.join(',');
}

QString IsMonthly::getNotMonthly() const
{
    return notmonthly.join(',');
}

QString IsMonthly::getFailure() const
{
    return failure.join(',');
}

/*
 * apply settings
 * also overwrite app.ini
 */
void IsMonthly::on_btnApply_clicked()
{
    const QString isMonthlyUrl = ui->inpUrl->text();
    isMonthlyController.setUrl(isMonthlyUrl);

    const QString quotaUrl = ui->inpQuotaUrl->text();
    quotaController.setUrl(quotaUrl);

    const QString serialCode = ui->inpSN->text();
    quotaController.setSerialCode(serialCode);

    const int language = ui->boxLanguage->currentIndex();

    config->setValue("url", isMonthlyUrl);
    config->setValue("quotaUrs", quotaUrl);
    config->setValue("serialCode", serialCode);
    config->setValue("language", language);

    /*
     * proxy settings
     */
    if (ui->radioNoProxy->isChecked()) {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        config->setValue("proxy", "no");
    }
    if (ui->radioSystemProxy->isChecked()) {
        for (QNetworkProxy& p : QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(isMonthlyUrl))) {
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
            for (QNetworkProxy& p : QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(isMonthlyUrl))) {
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

void IsMonthly::on_copyMonthly_clicked() const
{
    QGuiApplication::clipboard()->setText(ui->txtMonthly->toPlainText());
}

void IsMonthly::on_copyNotMonthly_clicked() const
{
    QGuiApplication::clipboard()->setText(ui->txtNotMonthly->toPlainText());
}

void IsMonthly::on_copyFailure_clicked() const
{
    QGuiApplication::clipboard()->setText(ui->txtFailure->toPlainText());
}

bool IsMonthly::parseProxy(QString url) {
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
    if (host == "") {
        return false;
    }
    proxy.setHostName(host);

    // validate port
    int port = proxyUrl.port();
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

void IsMonthly::on_btnQuota_clicked()
{
    quotaController.query();
}

void IsMonthly::setQuota(QNetworkReply* reply)
{
    QString response = QString::fromUtf8(reply->readAll());
    ui->inpQuota->setText(quotaController.getQuota(response));
}

} // namespace isMonthly
