#include "game.h"
#include "helpers.h"
#include <SDL3/SDL_timer.h>

static void init(struct bt_game game[static 1]) {
  game->camera = bt_default_camera;
  bt_time_init(&game->time);
}

static void update(struct bt_game game[static 1]) {
  struct bt_input input = game->input;
  bt_camera_update(&game->camera, &input);
  bt_time_update(&game->time);
}

static void set_render_info(struct bt_game game[static 1]) {
  struct bt_render_data render_data = {};
  render_data.camera_dir = game->camera.dir;
  render_data.camera_pos = game->camera.eye;
  float blend_factor =
      (float)game->time.accumulator / (float)bt_time_between_updates;
  SDL_LockMutex(game->render_info_mutex);
  game->render_info.previous_state = game->render_info.current_state;
  game->render_info.current_state = render_data;
  game->render_info.blend_factor = blend_factor;
  SDL_UnlockMutex(game->render_info_mutex);
}

static void deinit(struct bt_game game[static 1]) {
  bt_event_queue_deinit(&game->event_queue);
  SDL_DestroyMutex(game->render_info_mutex);
}

static int update_thread_fn(void *data) {
  struct bt_game *game = data;

  init(game);

  while (SDL_GetAtomicU32(&game->running)) {
    bt_time_start_loop(&game->time);

    struct bt_event event_buffer[SDL_arraysize(game->event_queue.events)] = {};
    uint16_t ev_count = bt_event_queue_get(&game->event_queue, event_buffer);
    for (uint16_t i = 0; i < ev_count; i += 1) {
      struct bt_event *event = &event_buffer[i];
      switch (event_buffer[i].type) {
      case bt_event_type_key:
        switch (event_buffer[i].key.code) {
        case bt_key_w:
          game->input.kbd.moving_forwards = event->key.down;
          break;
        case bt_key_a:
          game->input.kbd.moving_left = event->key.down;
          break;
        case bt_key_s:
          game->input.kbd.moving_backwards = event->key.down;
          break;
        case bt_key_d:
          game->input.kbd.moving_right = event->key.down;
          break;
        case bt_key_up:
          game->input.kbd.view_moving_up = event->key.down;
          break;
        case bt_key_down:
          game->input.kbd.view_moving_down = event->key.down;
          break;
        case bt_key_left:
          game->input.kbd.view_moving_left = event->key.down;
          break;
        case bt_key_right:
          game->input.kbd.view_moving_right = event->key.down;
          break;
        default:
        }
        break;
      case bt_event_type_mouse_motion:
        game->input.mouse.diff =
            bt_vec2_add(game->input.mouse.diff, event->mouse_motion.diff);
        break;
      default:
      }
    }

    while (bt_time_should_update(&game->time)) {
      update(game);
      set_render_info(game);
      SDL_zero(game->input.mouse.diff);
    }

    bt_time_end_loop(&game->time);
  }

  deinit(game);

  return 0;
}

bool bt_game_run(struct bt_game game[static 1]) {
  SDL_SetAtomicU32(&game->running, 1);

  if (!bt_event_queue_init(&game->event_queue)) {
    return false;
  }
  game->render_info_mutex = SDL_CreateMutex();
  if (!game->render_info_mutex) {
    BT_LOG_SDL_FAIL("Failed to create render info mutex");
    return false;
  }
  game->thread = SDL_CreateThread(update_thread_fn, "Update thread", game);
  if (!game->thread) {
    BT_LOG_SDL_FAIL("Failed to create update thread");
    return false;
  }

  return true;
}

void bt_game_stop(struct bt_game game[static 1]) {
  SDL_SetAtomicU32(&game->running, 0);
  SDL_WaitThread(game->thread, nullptr);
}

void bt_game_get_render_info(struct bt_game const game[static 1],
                             struct bt_render_info out[static 1]) {
  SDL_LockMutex(game->render_info_mutex);
  *out = game->render_info;
  SDL_UnlockMutex(game->render_info_mutex);
}
