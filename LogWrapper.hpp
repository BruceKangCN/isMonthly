#ifndef ISMONTHLY_LOGWRAPPER_HPP
#define ISMONTHLY_LOGWRAPPER_HPP

// force context record in Release build
#define QT_MESSAGELOGCONTEXT
// construct a logger with proper context
#define logger (QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO))

#include <QFile>
#include <QMessageLogger>

namespace isMonthly {

void log_wrapper_init(const char* path, QtMsgType level); // init the log wrapper
void log_wrapper_destroy(); // destroy the log wrapper

} // namespace isMonthly
#endif // ISMONTHLY_LOGWRAPPER_HPP
