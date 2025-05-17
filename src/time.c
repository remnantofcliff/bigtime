#include "time.h"
#include <SDL3/SDL_timer.h>

void bt_time_init(struct bt_time time[static 1]) {
  time->last_time = SDL_GetTicksNS();
  time->accumulator = 0;
}
void bt_time_start_loop(struct bt_time time[static 1]) {
  time->current_time = SDL_GetTicksNS();
  time->accumulator += time->current_time - time->last_time;
}
bool bt_time_should_update(struct bt_time time[static 1]) {
  return time->accumulator >= bt_time_between_updates;
}
void bt_time_update(struct bt_time time[static 1]) {
  time->accumulator -= bt_time_between_updates;
}
void bt_time_end_loop(struct bt_time time[static 1]) {
  time->last_time = time->current_time;
}
