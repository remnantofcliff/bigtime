#ifndef BT_EVENT_QUEUE_H
#define BT_EVENT_QUEUE_H

#include <SDL3/SDL_mutex.h>
#include <stdint.h>

enum bt_event_type : uint8_t {
  bt_event_type_key,
};

enum bt_key : uint8_t {
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

struct bt_event {
  enum bt_event_type type;
  union {
    struct bt_key_event key;
  };
};

struct bt_event_queue {
  SDL_Mutex *mutex;
  uint16_t cursor;
  struct bt_event events[4096];
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
uint16_t
bt_event_queue_get(struct bt_event_queue event_queue[static 1],
                   struct bt_event events[static SDL_arraysize(
                       (struct bt_event_queue){}.events)]);

#endif
