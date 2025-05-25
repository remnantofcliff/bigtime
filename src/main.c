#define SDL_MAIN_USE_CALLBACKS 1
#include "helpers.h"
#include "state.h"
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void **appstate, [[maybe_unused]] int argc,
                          [[maybe_unused]] char *argv[]) {
  SDL_SetAppMetadata("bigtime", "0.1.0", "org.remnantofcliff.bigtime");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    BT_LOG_SDL_FAIL("Failed to initialize SDL");
    return SDL_APP_FAILURE;
  }

  struct bt_state *state = SDL_calloc(1, sizeof(*state));
  *appstate = state;
  if (!state) {
    return SDL_APP_FAILURE;
  }

  if (!bt_state_init(state)) {
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  struct bt_state *state = appstate;

  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    [[fallthrough]];
  case SDL_EVENT_KEY_UP:
    bt_state_handle_keyevent(state, &event->key);
    break;
  case SDL_EVENT_MOUSE_MOTION:
    bt_state_handle_mouse_motion_event(state, &event->motion);
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    [[fallthrough]];
  case SDL_EVENT_MOUSE_BUTTON_UP:
    break;
  default:
    break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  bt_state_render((struct bt_state *)appstate);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, [[maybe_unused]] SDL_AppResult result) {
  struct bt_state *state = appstate;
  if (state) {
    bt_state_deinit(state);
    SDL_free(state);
  }
}
