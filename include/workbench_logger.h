/**
 *  @file workbench_logger.h
 *  @brief A logging utility for C programs with configurable verbosity levels
 * and optional context information.
 *
 *  This header file provides macros for logging messages with different
 * severity levels (debug, info, warning, error). The log messages can include
 * additional context information such as the function name, file name, and line
 * number. The verbosity of the logging can be controlled through the
 * `LOG_LEVEL` macro.
 *
 *  Features:
 *  - Configurable log levels: DEBUG, INFO, WARNING, ERROR
 *  - Optional inclusion of function names and file names in log messages
 *  - ANSI color codes for colored log messages in the terminal
 *
 *  Example usage:
 *  @code
 *  #define LOG_LEVEL 3  // Set log level to INFO
 *  #include "workbench_logger.h"
 *
 *  void example_function() {
 *      log_d("This is a debug message.");
 *      log_i("This is an info message.");
 *      log_w("This is a warning message.");
 *      log_e("This is an error message.");
 *  }
 *  @endcode
 */
#pragma once

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @name Helper
 * @{ */
/**
 * @brief Extracts the filename from the full path of the source file.
 */
#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
/** @} */

/**
 * @name Config
 * @{ */
#ifndef LOG_LEVEL
/**
 * @brief Default log level if not defined elsewhere.
 *
 * LOG_LEVEL determines the verbosity of the logging. The higher the value, the
 * more verbose the logging:
 * - 0: No logging
 * - 1: Error logging
 * - 2: Warning and Error logging
 * - 3: Info, Warning, and Error logging
 * - 4: Debug, Info, Warning, and Error logging
 */
#define LOG_LEVEL 4
#endif

/**
 * @brief Controls whether to include the function name in log messages.
 */
#ifndef PRINT_FUNC
#define PRINT_FUNC (LOG_LEVEL > 3)
#endif

/**
 * @brief Controls whether to include the file name and line number
 * in log messages.
 */
#ifndef PRINT_FILE
#define PRINT_FILE (LOG_LEVEL > 2)
#endif
/** @} */

/**
 * @name Color function-like macros
 * @{ */
/**
 * @brief Adds ANSI color and style codes to a text string.
 *
 * This macro wraps a text string with ANSI color and style codes to display
 * the text in a specific color and style in the terminal.
 * Common style codes include:
 * - 0: Regular
 * - 1: Bold
 * - 2: Dim
 * - 3: Italic
 * - 4: Underline
 *
 * Common color codes include:
 * - 30: Black
 * - 31: Red
 * - 32: Green
 * - 33: Yellow
 * - 34: Blue
 * - 35: Magenta
 * - 36: Cyan
 * - 37: White
 *
 * @param text The text to be colored and styled.
 * @param color The ANSI color code.
 * @param style The ANSI style code.
 *
 * @return The colored and styled text string.
 */
#define COLOR(text, color, style) "\033[" #style ";" #color "m" text "\033[0m"

#define RED(text) COLOR(text, 31, 0) /**< @brief Colors the text red. */
#define GRN(text) COLOR(text, 32, 0) /**< @brief Colors the text green. */
#define YLW(text) COLOR(text, 33, 0) /**< @brief Colors the text yellow. */
#define PRL(text) COLOR(text, 35, 0) /**< @brief Colors the text purple. */
#define CYN(text) COLOR(text, 36, 0) /**< @brief Colors the text cyan. */
/** @} */

/**
 * @name Logging function-like macros
 * @param message The debug message string. Can include format specifiers.
 * @param ... Optional additional arguments corresponding to the format
 * specifiers.
 * @{ */
/**
 * @brief Logs a debug message.
 */
#define log_d(message, ...)                                                    \
  if (get_log_level() > 3)                                                     \
    log_format(__stdoutp, CYN("dbg"), message __VA_OPT__(, ) __VA_ARGS__);
/**
 * @brief Logs an info message.
 */
#define log_i(message, ...)                                                    \
  if (get_log_level() > 2)                                                     \
    log_format(__stdoutp, GRN("inf"), message __VA_OPT__(, ) __VA_ARGS__);
/**
 * @brief Logs a warning message.
 */

#define log_w(message, ...)                                                    \
  if (get_log_level() > 1)                                                     \
    log_format(__stderrp, YLW("wrn"), YLW(message) __VA_OPT__(, ) __VA_ARGS__);
/**
 * @brief Logs an error message.
 */
#define log_e(message, ...)                                                    \
  if (get_log_level() > 0)                                                     \
    log_format(__stderrp, RED("err"), RED(message) __VA_OPT__(, ) __VA_ARGS__);
/** @} */

#if PRINT_FUNC == 1
#define _fun_print_tmpl "%s "
#define _fun_print_arg , __func__
#else
#define _fun_print_tmpl ""
#define _fun_print_arg
#endif
#if PRINT_FILE == 1
#define _file_print_tmpl "(%s:%d) "
#define _file_print_arg , __FILENAME__, __LINE__
#else
#define _file_print_tmpl ""
#define _file_print_arg
#endif

/**
 * @brief Formats and logs a message to a specified file with a tag and optional
 * arguments.
 *
 * This macro writes a formatted log message to the provided file. The log
 * message includes a tag and can incorporate additional formatted arguments
 * similar to `printf`.
 *
 * The macro supports optional arguments which are included in the formatted
 * message if provided.
 *
 * @param file     The file stream to write the log message to
 * (e.g., stderr, stdout, or a file pointer).
 * @param tag      A string tag to categorize or label the log message
 * (e.g., "INFO", "ERROR").
 * @param message  The main message string to be logged. This can include format
 * specifiers like printf.
 * @param ...      Optional additional arguments that correspond to the format
 * specifiers in the message.
 *
 * Example usage:
 * @code
 * log_format(stderr, "ERROR", "Failed to open file: %s", filename);
 * // This would log: [ERROR] Failed to open file: example.txt
 * @endcode
 */
#define log_format(file, tag, message, ...)                                    \
  fprintf(file, "[%s] " _fun_print_tmpl _file_print_tmpl message "\n",         \
          tag _fun_print_arg _file_print_arg __VA_OPT__(, ) __VA_ARGS__);
