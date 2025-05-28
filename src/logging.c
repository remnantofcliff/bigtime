#include "logging.h"

FILE *bt_log_file_err;
FILE *bt_log_file_out;

void bt_init_logger(void) {
  bt_log_file_out = stdout;
  bt_log_file_err = stderr;
}
