#include "game.h"
#include "helpers.h"

static void init(struct bt_game game[static 1]) {
  game->camera = bt_default_camera;
  bt_time_init(&game->time);
}

static void update(struct bt_game game[static 1]) {
  bt_camera_update(&game->camera, &game->input);
  bt_time_update(&game->time);
}

static void set_render_info(struct bt_game game[static 1]) {
  struct bt_render_info render_info = {};
  render_info.time = game->time.current_time;
  render_info.camera_dir = game->camera.dir;
  render_info.camera_pos = game->camera.eye;
  SDL_LockMutex(game->render_info_mutex);
  {
    game->current_render_info ^= 1;
    game->render_infos[game->current_render_info] = render_info;
  }
  SDL_UnlockMutex(game->render_info_mutex);
}

static void deinit(struct bt_game game[static 1]) {
  SDL_DestroyMutex(game->input_mutex);
  SDL_DestroyMutex(game->render_info_mutex);
}

static int update_thread_fn(void *data) {
  struct bt_game *game = data;

  init(game);

  while (SDL_GetAtomicU32(&game->running)) {
    bt_time_start_loop(&game->time);

    while (bt_time_should_update(&game->time)) {
      update(game);
      set_render_info(game);
    }

    bt_time_end_loop(&game->time);
  }

  deinit(game);

  return 0;
}

bool bt_game_run(struct bt_game game[static 1]) {
  SDL_SetAtomicU32(&game->running, 1);

  game->render_info_mutex = SDL_CreateMutex();
  if (game->render_info_mutex == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create render info mutex");
    return false;
  }
  game->input_mutex = SDL_CreateMutex();
  if (game->render_info_mutex == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create input mutex");
    return false;
  }
  game->thread =
      SDL_CreateThread(update_thread_fn, "Update thread", game);
  if (game->thread == nullptr) {
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
                             struct bt_render_info out[static 2]) {
  SDL_LockMutex(game->render_info_mutex);
  {
    out[0] = game->render_infos[game->current_render_info];
    out[1] = game->render_infos[game->current_render_info ^ 1];
  }
  SDL_UnlockMutex(game->render_info_mutex);
}

