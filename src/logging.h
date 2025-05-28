#ifndef BT_LOGGING_H
#define BT_LOGGING_H

#include <SDL3/SDL_error.h> // IWYU pragma: keep
#include <stdio.h>          // IWYU pragma: export

extern FILE *bt_log_file_err;
extern FILE *bt_log_file_out;

#define BT_LOG_INFO(fmt, ...)                                                  \
  fprintf(bt_log_file_out, "BT INFO : " fmt "\n" __VA_OPT__(, ) __VA_ARGS__)
#define BT_LOG_ERR(fmt, ...)                                                   \
  fprintf(bt_log_file_err, "BT ERROR: " fmt "\n" __VA_OPT__(, ) __VA_ARGS__)
#define BT_LOG_SDL_FAIL(fmt, ...)                                              \
  BT_LOG_ERR(fmt "; got error: %s" __VA_OPT__(, ) __VA_ARGS__, SDL_GetError())

void bt_init_logger(void);

#endif
