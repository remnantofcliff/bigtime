#include "fps_timer.h"
#include "time.h"
#include <SDL3/SDL_timer.h>

void bt_fps_timer_increment_fps(struct bt_fps_timer timer[static 1],
                                struct bt_fps_report out[static 1]) {
  timer->fps += 1;
  uint64_t current_time = SDL_GetTicksNS();
  if (current_time - timer->last_report_time < bt_second) {
    out->did_update = false;
    return;
  }

  out->did_update = true;
  out->fps = timer->fps;
  timer->last_report_time = current_time;
  timer->fps = 0;
}
