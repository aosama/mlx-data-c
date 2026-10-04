#ifndef MLXD_ERROR_H
#define MLXD_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup mlxd_error Error management
 */
/**@{*/

typedef void (*mlx_error_handler_func)(const char* msg, void* data);

/**
 * Install the handler invoked by mlxd_error(). Passing NULL for `handler`
 * restores the default handler, which prints the message and terminates the
 * process. `data` is handed back to the handler; when `dtor` is non-NULL it
 * is run on `data` when the handler is replaced or the library unloads.
 */
void mlxd_set_error_handler(
    mlx_error_handler_func handler,
    void* data,
    void (*dtor)(void*));

/**
 * Report an error: format the message, append the reporting file and line,
 * and hand it to the installed handler. Never returns if the default
 * handler is installed.
 */
void _mlxd_error(const char* file, const int line, const char* fmt, ...);

/**
 * Report an error. Macro passing file name and line number to _mlxd_error().
 */
#define mlxd_error(...) _mlxd_error(__FILE__, __LINE__, __VA_ARGS__)

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
