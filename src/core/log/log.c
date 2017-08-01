#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*!
 * @brief Writes an error message to the error log.
 *
 * @param[i] msg    Message to write to the error log file.
 *
 */
void core_log_logErrorMessage(const char *msg)
{
    fprintf(stderr, msg);
    return;
}

/*!
 * @brief Writes a warning message to the warning log.
 *
 * @param[in] msg    Message to write to the warning log file.
 *
 */
void core_log_logWarningMessage(const char *msg)
{
    fprintf(stdout, msg); 
    return;
}

/*!
 * @brief Writes an info message to the info log.
 *
 * @param[in] msg    Message to write to the info log file.
 *
 */
void core_log_logInfoMessage(const char *msg)
{
    fprintf(stdout, msg); 
    return;
}
