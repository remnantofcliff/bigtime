#ifndef BT_FPS_TIMER_H
#define BT_FPS_TIMER_H

#include <stdint.h>

struct bt_fps_timer {
  uint64_t last_report_time;
  uint64_t fps;
};

struct bt_fps_report {
  uint64_t fps;
  bool did_update;
};

void bt_fps_timer_increment_fps(struct bt_fps_timer timer[static 1],
                                struct bt_fps_report out[static 1]);

#endif
