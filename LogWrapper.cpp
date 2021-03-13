#include "LogWrapper.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>

QtMsgType log_level; // log level setting
QFile logFile; // a pointer to the log file
const char* logFormat = "[%1][%2][%3:%4 %5]%6\n";
const char* timeFormat = "yyyy-MM-dd hh:mm:ss.zzz t";

/*
 * log message handler
 * type must be QtMessageHandler,
 * aka (void)(QtMsgType, const QMessageLogContext&, const QString&)
 * so do not wrap it with namespace
 */
void msg_hnd(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    // check for log level setting, DO NOT add break!
    switch (log_level) {
    case QtFatalMsg   : if (type == QtCriticalMsg) return;
    case QtCriticalMsg: if (type == QtWarningMsg)  return;
    case QtWarningMsg : if (type == QtInfoMsg)     return;
    case QtInfoMsg    : if (type == QtDebugMsg)    return;
    case QtDebugMsg   : break;
    default: return;
    }

    // get a formatted current DateTime string
    QString time = QDateTime::currentDateTime().toString(timeFormat);

    // get a proper level string
    QString level;
    switch (type) {
    case QtDebugMsg   : level = "DEBUG"   ; break;
    case QtInfoMsg    : level = "INFO"    ; break;
    case QtWarningMsg : level = "WARN"    ; break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg   : level = "FATAL"   ; break;
    default           : level = ""        ; break;
    }

    // write the formmated log string to file
    logFile.write(QString(logFormat).arg(time, level, QString(ctx.file)
                 , QString::number(ctx.line), QString(ctx.function), msg)
                 .toUtf8());
}

namespace isMonthly {

void log_wrapper_init(const char* path, QtMsgType level)
{
    log_level = level; // set the log level
    logFile.setFileName(path);
    qInstallMessageHandler(msg_hnd); // install the log message handler
    logFile.open(QIODevice::WriteOnly | QIODevice::Append); // open the log file

    // print version infomation
    logger.info(QString("// %1 %2 //").arg(qAppName(), qApp->applicationVersion())
                    .toUtf8().constData());
    logger.info(QString("// based on Qt version %1 //").arg(QT_VERSION_STR)
                    .toUtf8().constData());
    logger.debug(QString("// running with Qt version %1 //").arg(qVersion())
                    .toUtf8().constData());
}

void log_wrapper_destroy()
{
    // close the log file if it is open
    if (logFile.isOpen()) {
        logFile.close();
    }
}

} // namespace isMonthly
