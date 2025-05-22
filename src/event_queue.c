#include "event_queue.h"
#include "helpers.h"

bool bt_event_queue_init(struct bt_event_queue event_queue[static 1]) {
  SDL_zerop(event_queue);
  event_queue->mutex = SDL_CreateMutex();
  if (event_queue->mutex == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create event queue mutex");
    return false;
  }

  return true;
}
void bt_event_queue_deinit(struct bt_event_queue event_queue[static 1]) {
  SDL_DestroyMutex(event_queue->mutex);
}

void bt_event_queue_add(struct bt_event_queue event_queue[static 1],
                        struct bt_event const event[static 1]) {
  SDL_LockMutex(event_queue->mutex);
  if (event_queue->cursor < SDL_arraysize(event_queue->events)) {
    event_queue->events[event_queue->cursor] = *event;
    event_queue->cursor += 1;
  }
  SDL_UnlockMutex(event_queue->mutex);
}
uint16_t bt_event_queue_get(
    struct bt_event_queue event_queue[restrict static 1],
    struct bt_event
        events[restrict static SDL_arraysize(event_queue->events)]) {
  SDL_LockMutex(event_queue->mutex);
  uint16_t count = event_queue->cursor;
  SDL_memcpy(events, event_queue->events, count * sizeof(*event_queue->events));
  event_queue->cursor = 0;
  SDL_UnlockMutex(event_queue->mutex);

  return count;
}
