
#include <plog/Log.h>
#include <pthread.h>
#include <ExternalMutexConsoleAppender.h>
#include <SeverityFormatter.h>


static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> mtx_console_appender(&print_lock);

extern "C" void init_plog();
extern "C" void core_log_plog_V(const char *msg);
extern "C" void core_log_plog_D(const char *msg);
extern "C" void core_log_plog_I(const char *msg);
extern "C" void core_log_plog_W(const char *msg);
extern "C" void core_log_plog_E(const char *msg);
extern "C" void core_log_plog_F(const char *msg);
extern "C" void core_log_plog_N(const char *msg);

// #define LOGV                            LOG_VERBOSE
// #define LOGD                            LOG_DEBUG
// #define LOGI                            LOG_INFO
// #define LOGW                            LOG_WARNING
// #define LOGE                            LOG_ERROR
// #define LOGF                            LOG_FATAL
// #define LOGN                            LOG_NONE

void init_plog() {

    // static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
    // DMLib::setPrintLock(&print_lock);
    // static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&print_lock);

    plog::init(plog::verbose, &mtx_console_appender);
}

void core_log_plog_V(const char *msg) { LOGV << msg; }
void core_log_plog_D(const char *msg) { LOGD << msg; }
void core_log_plog_I(const char *msg) { LOGI << msg; }
void core_log_plog_W(const char *msg) { LOGW << msg; }
void core_log_plog_E(const char *msg) { LOGE << msg; }
void core_log_plog_F(const char *msg) { LOGF << msg; }
void core_log_plog_N(const char *msg) { LOGN << msg; }