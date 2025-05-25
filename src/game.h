#ifndef BT_GAME_H
#define BT_GAME_H

#include "camera.h"
#include "event_queue.h"
#include "time.h"

struct bt_render_data {
  struct bt_vec3 camera_dir;
  struct bt_vec3 camera_pos;
};

struct bt_render_info {
  struct bt_render_data current_state;
  struct bt_render_data previous_state;
  float blend_factor;
};

struct bt_game {
  struct bt_event_queue event_queue;
  SDL_Thread *thread;
  SDL_Mutex *render_info_mutex;
  struct bt_render_info render_info;
  struct bt_time time;
  struct bt_camera camera;
  SDL_AtomicU32 running;
  uint8_t current_render_info;
  struct bt_input input;
};

bool bt_game_run(struct bt_game game[static 1]);
void bt_game_stop(struct bt_game game[static 1]);
/*
 * Puts data from the last two updates into the out parameter. The first one is
 * the newer frame and the later one is the earlier one.
 */
void bt_game_get_render_info(struct bt_game const game[static 1],
                             struct bt_render_info out[static 1]);

#endif
