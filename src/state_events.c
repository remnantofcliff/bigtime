#include "state.h"

void bt_state_handle_keyevent(struct bt_state state[static 1],
                              SDL_KeyboardEvent const event[static 1]) {
  enum bt_key key = {};
  bool key_used = false;
  switch (event->scancode) {
  case SDL_SCANCODE_A:
    key = bt_key_a;
    key_used = true;
    break;
  case SDL_SCANCODE_D:
    key = bt_key_d;
    key_used = true;
    break;
  case SDL_SCANCODE_S:
    key = bt_key_s;
    key_used = true;
    break;
  case SDL_SCANCODE_W:
    key = bt_key_w;
    key_used = true;
    break;
  case SDL_SCANCODE_RIGHT:
    key = bt_key_right;
    key_used = true;
    break;
  case SDL_SCANCODE_LEFT:
    key = bt_key_left;
    key_used = true;
    break;
  case SDL_SCANCODE_DOWN:
    key = bt_key_down;
    key_used = true;
    break;
  case SDL_SCANCODE_UP:
    key = bt_key_up;
    key_used = true;
    break;
  default:
    break;
  }

  if (!key_used) {
    return;
  }

  bt_event_queue_add(&state->game.event_queue, &(struct bt_event){
                                                   .type = bt_event_type_key,
                                                   .key =
                                                       {
                                                           .code = key,
                                                           .down = event->down,
                                                       },
                                               });
}

void bt_state_handle_mouse_motion_event(
    struct bt_state state[static 1],
    SDL_MouseMotionEvent const event[static 1]) {
  bt_event_queue_add(&state->game.event_queue,
                     &(struct bt_event){
                         .type = bt_event_type_mouse_motion,
                         .mouse_motion =
                             {
                                 .diff =
                                     {
                                         .x = event->xrel,
                                         .y = event->yrel,
                                     },
                             },
                     });
}
