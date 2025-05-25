#ifndef BT_EVENT_QUEUE_H
#define BT_EVENT_QUEUE_H

#include "math.h"
#include <SDL3/SDL_mutex.h>

enum bt_event_type {
  bt_event_type_key,
  bt_event_type_mouse_motion,
};

enum bt_key {
  bt_key_w,
  bt_key_a,
  bt_key_s,
  bt_key_d,
  bt_key_up,
  bt_key_down,
  bt_key_left,
  bt_key_right,
};

struct bt_key_event {
  enum bt_key code;
  bool down;
};

struct bt_mouse_motion_event {
  struct bt_vec2 diff;
};

struct bt_event {
  enum bt_event_type type;
  union {
    struct bt_key_event key;
    struct bt_mouse_motion_event mouse_motion;
  };
};

struct bt_event_queue {
  SDL_Mutex *mutex;
  uint16_t cursor;
  struct bt_event events[1024];
};

bool bt_event_queue_init(struct bt_event_queue event_queue[static 1]);
void bt_event_queue_deinit(struct bt_event_queue event_queue[static 1]);

/*
 * Put an event into the event queue.
 */
void bt_event_queue_add(struct bt_event_queue event_queue[static 1],
                        struct bt_event const event[static 1]);
/*
 * Puts the events from the queue into the buffer. The function is thread-safe.
 * Returns the count of events in the buffer.
 */
uint16_t bt_event_queue_get(
    struct bt_event_queue event_queue[static 1],
    struct bt_event events[static SDL_arraysize(event_queue->events)]);

#endif
