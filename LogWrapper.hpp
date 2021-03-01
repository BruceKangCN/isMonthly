#ifndef ISMONTHLY_LOGWRAPPER_HPP
#define ISMONTHLY_LOGWRAPPER_HPP

#define QT_MESSAGELOGCONTEXT
#define logger (QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO))

#include <QObject>
#include <QFile>
#include <QMessageLogger>

namespace isMonthly {

void log_wrapper_init(const char* path);
void log_wrapper_destroy();

} // namespace isMonthly
#endif // ISMONTHLY_LOGWRAPPER_HPP
