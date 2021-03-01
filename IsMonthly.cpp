#include "IsMonthly.hpp"
#include "ui_widget.h"

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
    , config(new QSettings("app.ini", QSettings::IniFormat, this))
    , isMonthlyModel(new QStandardItemModel(0, 4, this))
{
    ui->setupUi(this);
    this->setWindowTitle("is monthly");

    isMonthlyModel->setHorizontalHeaderLabels({"#", "id", tr("结果"), tr("码率")});
    ui->tableView->setModel(isMonthlyModel);
    ui->tableView->verticalHeader()->hide();

    /*
     * get settings from ini file
     * if the key-value pair exists
     * first read value from config
     * then set the related property
     * finally set the ui
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
        } else if (parseProxy(v)) { // if custom proxy parse succeed
            ui->radioCustom->setChecked(true);
            ui->inpProxy->setText(v);
            QNetworkProxy::setApplicationProxy(proxy);
        } else {
            ui->radioSystemProxy->setChecked(true);
            QNetworkProxyFactory::setUseSystemConfiguration(true);
        }
    }
    if (config->contains("language")) {
        const int language = config->value("language").toInt();
        ui->boxLanguage->setCurrentIndex(language);
    }

    /*
     * init connections
     */
    connect(&isMonthlyController, &IsMonthlyController::queryFinished,
        this, &IsMonthly::appendResult);
    connect(&quotaController, &QuotaController::queryFinished,
        this, &IsMonthly::setQuota);

    log_wrapper_init("isMonthly.log");

    /*
     * check SSL compatibility, only for debug purpose
     */
    logger.debug() << "build with: " << QSslSocket::sslLibraryBuildVersionString();
    logger.debug() << "SSL lib version: " << QSslSocket::sslLibraryVersionString();
    logger.debug() << "support SSL: " << QSslSocket::supportsSsl();
}

IsMonthly::~IsMonthly()
{
    log_wrapper_destroy();
    delete ui;
}

/*
 * send a request to query infomation
 * ids are splited by ',' in lineEdit widget
 */
void IsMonthly::on_pushButton_clicked()
{
    QRegularExpression seprator("\\s*,\\s*");
    QStringList ids = ui->lineEdit->text().split(seprator);
    for (QString& id : ids) {
        isMonthlyController.query(id);
    }
}

/*
 * append a row to the IsMonthlyModel
 * the row is based on the result parsed from the reply
 * the parser is IsMonthlyController
 */
void IsMonthly::appendResult(QNetworkReply* reply)
{
    // get the result struct from return value
    const IsMonthlyResponse& response = isMonthlyController.getResponse(reply);

    // create simple items and set data based on result
    QStandardItem* idItem = new QStandardItem();
    idItem->setData(response.replyId + 1, Qt::DisplayRole); // # start from 1
    QStandardItem* cidItem = new QStandardItem();
    cidItem->setData(response.cid, Qt::DisplayRole);
    QStandardItem* bitrateItem = new QStandardItem();
    bitrateItem->setData(response.bitrate, Qt::DisplayRole);

    // create IsMonthlyItem and set value based on success, isMonthly properties
    QStandardItem* isMonthlyItem = new QStandardItem();
    if (response.success) {
        if (response.isMonthly) {
            isMonthlyItem->setData(tr("月额：是"), Qt::DisplayRole);
        } else {
            isMonthlyItem->setData(tr("月额：否"), Qt::DisplayRole);
        }
    } else {
        isMonthlyItem->setData(tr("查询失败"), Qt::DisplayRole);
    }

    // append a new row with the items to the IsMonthlyModel
    isMonthlyModel->appendRow({idItem, cidItem, isMonthlyItem, bitrateItem});
}

void IsMonthly::keyPressEvent(QKeyEvent* ev)
{
    // execute query when enter or return is pressed
    if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
    {
        on_pushButton_clicked();
    }

    // delete selected items when delete is pressed
    if (ev->key() == Qt::Key_Delete)
    {
        auto selectedRows = ui->tableView->selectionModel()->selectedRows();
        quint64 count = 0;
        for (auto& index : selectedRows) {
            /*
             * row number will be changed after takeRow
             * so minus the number of taken rows
             * and then add the count
             */
            auto items = isMonthlyModel->takeRow(index.row() - count++);
            for (auto* item : items) {
                delete item;
            }
        }
    }

    // copy cids when copy sequence is pressed
    if (ev->matches(QKeySequence::Copy)) {
        auto selectedRows = ui->tableView->selectionModel()->selectedRows(1);
        QStringList selected = QStringList();
        for (auto& index : selectedRows) {
            selected.append(isMonthlyModel->data(index).toString());
        }
        QGuiApplication::clipboard()->setText(selected.join(','));
    }
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

    // proxy settings
    if (ui->radioNoProxy->isChecked()) {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        config->setValue("proxy", "no");
    }
    if (ui->radioSystemProxy->isChecked()) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        config->setValue("proxy", "system");
    }
    if (ui->radioCustom->isChecked()) {
        QString proxyUrl = ui->inpProxy->text();
        if (parseProxy(proxyUrl)) {
            QNetworkProxy::setApplicationProxy(proxy);
            config->setValue("proxy", proxyUrl);
        } else { // if proxy parse failed
            QNetworkProxyFactory::setUseSystemConfiguration(true);

            /*
             *  set the radio box status,
             *  add parse failed hint,
             *  change the "proxy" value in config
             */
            ui->radioCustom->setChecked(false);
            ui->radioSystemProxy->setChecked(true);
            ui->inpProxy->setText(proxyUrl + "[" + tr("解析出错") + "]");
            config->setValue("proxy", "system");
        }
    }

    config->sync();
}

/*
 * parse user defined proxy read from config file
 * return true if parse successfully
 * the result is stored in private member proxy
 */
bool IsMonthly::parseProxy(QString url) {
    // convert QString to QUrl and validate
    QUrl proxyUrl(url);
    if (!proxyUrl.isValid()) {
        return false;
    }

    // validate scheme and set proxyType based on it
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

    /*
     * check if username and password is required
     */
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
    /*
     * let the QuotaController start query
     * when query is over, QuotaController::queryFinished is emitted
     * then the result can be taken with QuotaController::getQuota
     */
    quotaController.query();
}

inline void IsMonthly::setQuota(QNetworkReply* reply)
{
    // get the response content, then the quota can be parsed with getQuota
    QString response = QString::fromUtf8(reply->readAll());
    /*
     * let the QuotaController parse the response content
     * and return a quota result QString
     */
    ui->inpQuota->setText(quotaController.getQuota(response));
}

} // namespace isMonthly
