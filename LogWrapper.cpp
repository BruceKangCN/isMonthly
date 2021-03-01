#include "LogWrapper.hpp"

#include <QDateTime>

QFile* logFile;
const char* logFormat = "[%1][%2][%3:%4 %5]%6\n";
const char* timeFormat = "yyyy-MM-dd hh:mm:ss.zzz";

void msg_hnd(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    QString time = QDateTime::currentDateTime().toString(timeFormat);

    QString level;
    switch (type) {
    case QtDebugMsg   : level = "DEBUG"   ; break;
    case QtInfoMsg    : level = "INFO"    ; break;
    case QtWarningMsg : level = "WARN"    ; break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg   : level = "FATAL"   ; break;
    default           : level = ""        ; break;
    }

    logFile->write(QString(logFormat).arg(time).arg(level).arg(ctx.file)
                       .arg(ctx.line).arg(ctx.function).arg(msg).toUtf8());
}

namespace isMonthly {

void log_wrapper_init(const char* path)
{
    logFile = new QFile(path);
    qInstallMessageHandler(msg_hnd);
    logFile->open(QIODevice::WriteOnly | QIODevice::Append);
}

void log_wrapper_destroy()
{
    if (logFile->isOpen()) {
        logFile->close();
    }
    delete logFile;
}

} // namespace isMonthly
